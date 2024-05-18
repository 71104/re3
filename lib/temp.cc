#include "lib/temp.h"

#include <cstdint>
#include <memory>
#include <utility>

#include "absl/container/btree_map.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "lib/automaton.h"
#include "lib/dfa.h"
#include "lib/nfa.h"

namespace re3 {

State MakeState(absl::flat_hash_map<uint8_t, absl::InlinedVector<int32_t, 1>> &&edges) {
  State state;
  for (auto &[ch, edge] : edges) {
    state[ch] = std::move(edge);
  }
  return state;
}

bool TempNFA::force_nfa_for_testing = false;

bool TempNFA::IsDeterministic() const {
  if (force_nfa_for_testing) {
    return false;
  }
  for (auto const &[state, edges] : states_) {
    if (edges[0].size() > 1) {
      return false;
    }
    bool const has_epsilon = !edges[0].empty();
    for (int ch = 1; ch < 256; ++ch) {
      auto const &edge = edges[ch];
      if (edge.size() > 1 || (!edge.empty() && has_epsilon)) {
        return false;
      }
    }
  }
  return true;
}

void TempNFA::RenameState(int32_t const old_name, int32_t const new_name) {
  auto const it = states_.find(old_name);
  if (it != states_.end()) {
    auto node = states_.extract(it);
    MergeState(new_name, std::move(node.mapped()));
  }
  for (auto &[state, edges] : states_) {
    for (auto &edge : edges) {
      for (auto &transition : edge) {
        if (transition == old_name) {
          transition = new_name;
        }
      }
    }
  }
  if (initial_state_ == old_name) {
    initial_state_ = new_name;
  }
  if (final_state_ == old_name) {
    final_state_ = new_name;
  }
}

void TempNFA::RenameAllStates(int32_t *const next_state) {
  absl::flat_hash_map<int32_t, int32_t> state_map;
  for (auto &[state, edges] : states_) {
    state_map.try_emplace(state, (*next_state)++);
  }
  state_map.try_emplace(initial_state_, (*next_state)++);
  state_map.try_emplace(final_state_, (*next_state)++);
  absl::btree_map<int32_t, State> new_states;
  for (auto &[state, edges] : states_) {
    for (auto &edge : edges) {
      for (auto &transition : edge) {
        transition = state_map[transition];
      }
    }
    new_states.try_emplace(state_map[state], std::move(edges));
  }
  states_ = std::move(new_states);
  initial_state_ = state_map[initial_state_];
  final_state_ = state_map[final_state_];
}

void TempNFA::AddEdge(uint8_t const label, int32_t const from, int32_t const to) {
  states_[from][label].emplace_back(to);
}

void TempNFA::Chain(TempNFA &&other) {
  RenameState(final_state_, other.initial_state_);
  for (auto &[state, edges] : other.states_) {
    MergeState(state, std::move(edges));
  }
  final_state_ = other.final_state_;
}

void TempNFA::Merge(TempNFA &&other, int32_t const initial_state, int32_t const final_state) {
  for (auto &[state, edges] : other.states_) {
    MergeState(state, std::move(edges));
  }
  states_.try_emplace(initial_state, MakeState({{0, {initial_state_, other.initial_state_}}}));
  states_.try_emplace(final_state, MakeState({}));
  AddEdge(0, final_state_, final_state);
  AddEdge(0, other.final_state_, final_state);
  initial_state_ = initial_state;
  final_state_ = final_state;
}

std::unique_ptr<AutomatonInterface> TempNFA::Finalize() && {
  CollapseEpsilonMoves();
  if (IsDeterministic()) {
    return std::make_unique<DFA>(std::move(*this).ToDFA());
  } else {
    return std::make_unique<NFA>(std::move(*this).ToNFA());
  }
}

void TempNFA::MergeState(int32_t const state, State &&edges) {
  auto const [it, inserted] = states_.try_emplace(state, std::move(edges));
  if (!inserted) {
    for (int ch = 0; ch < 256; ++ch) {
      auto &edge1 = it->second[ch];
      auto const &edge2 = edges[ch];
      edge1.insert(edge1.end(), edge2.begin(), edge2.end());
    }
  }
}

bool TempNFA::HasOnlyOneEpsilonMove(int32_t const state) const {
  auto const &edges = states_.find(state)->second;
  if (edges[0].size() != 1) {
    return false;
  }
  for (int ch = 1; ch < 256; ++ch) {
    if (!edges[ch].empty()) {
      return false;
    }
  }
  return true;
}

bool TempNFA::CollapseNextEpsilonMove() {
  for (auto &[state, edges] : states_) {
    if (HasOnlyOneEpsilonMove(state)) {
      int32_t const destination = edges[0][0];
      if (state == destination || state != final_state_) {
        edges[0].clear();
        RenameState(destination, state);
        return true;
      }
    }
  }
  return false;
}

void TempNFA::CollapseEpsilonMoves() {
  while (CollapseNextEpsilonMove()) {
    // nothing to do.
  }
}

DFA TempNFA::ToDFA() && {
  absl::flat_hash_map<int32_t, int32_t> state_map;
  DFA::States dfa_states;
  dfa_states.reserve(states_.size());
  int32_t i = 0;
  for (auto &[state, edges] : states_) {
    state_map.try_emplace(state, i++);
    DFA::State dfa_state;
    for (int ch = 0; ch < 256; ++ch) {
      if (edges[ch].empty()) {
        dfa_state[ch] = -1;
      } else {
        dfa_state[ch] = edges[ch][0];
      }
    }
    dfa_states.emplace_back(std::move(dfa_state));
  }
  state_map.try_emplace(initial_state_, i++);
  state_map.try_emplace(final_state_, i++);
  for (auto &state : dfa_states) {
    for (auto &transition : state) {
      if (transition >= 0) {
        transition = state_map[transition];
      }
    }
  }
  return DFA(std::move(dfa_states), state_map[initial_state_], state_map[final_state_]);
}

NFA TempNFA::ToNFA() && {
  absl::flat_hash_map<int32_t, int32_t> state_map;
  NFA::States nfa_states;
  nfa_states.reserve(states_.size());
  int32_t i = 0;
  for (auto &[state, edges] : states_) {
    state_map.try_emplace(state, i++);
    nfa_states.emplace_back(std::move(edges));
  }
  state_map.try_emplace(initial_state_, i++);
  state_map.try_emplace(final_state_, i++);
  for (auto &state : nfa_states) {
    for (auto &edge : state) {
      for (auto &transition : edge) {
        transition = state_map[transition];
      }
    }
  }
  return NFA(std::move(nfa_states), state_map[initial_state_], state_map[final_state_]);
}

}  // namespace re3
