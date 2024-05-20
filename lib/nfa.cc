#include "lib/nfa.h"

#include <cstdint>
#include <string_view>
#include <utility>

#include "absl/container/flat_hash_set.h"

namespace re3 {

bool NFA::Run(std::string_view input) const {
  absl::flat_hash_set<int32_t> states{initial_state_};
  states.reserve(states_.size());
  EpsilonClosure(&states);
  absl::flat_hash_set<int32_t> next_states;
  while (!input.empty()) {
    uint8_t const ch = input[0];
    input.remove_prefix(1);
    next_states.reserve(states_.size());
    for (auto const state_num : states) {
      auto const& edges = states_[state_num];
      for (auto const transition : edges[ch]) {
        next_states.emplace(transition);
      }
      EpsilonClosure(&next_states);
    }
    states = std::move(next_states);
    next_states = absl::flat_hash_set<int32_t>();
  }
  return states.contains(final_state_);
}

void NFA::EpsilonClosure(absl::flat_hash_set<int32_t>* const states) const {
  bool new_state_found;
  do {
    new_state_found = false;
    for (auto const state_num : *states) {
      auto const& edges = states_[state_num];
      for (auto const transition : edges[0]) {
        auto const [it, inserted] = states->emplace(transition);
        new_state_found |= inserted;
      }
    }
  } while (new_state_found);
}

}  // namespace re3
