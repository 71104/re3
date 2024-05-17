#include "lib/temp.h"

#include <cstdint>
#include <memory>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "lib/automaton.h"
#include "lib/dfa.h"
#include "lib/nfa.h"

namespace re3 {

bool TempNFA::IsDeterministic() const {
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

std::unique_ptr<AutomatonInterface> TempNFA::Finalize() && {
  if (IsDeterministic()) {
    CollapseEpsilonMoves();
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

}  // namespace re3
