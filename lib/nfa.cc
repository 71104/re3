#include "lib/nfa.h"

#include <cstdint>
#include <string_view>

namespace re3 {

bool NFA::Run(std::string_view const input) const {
  return Runner(*this).Run(initial_state_, input);
}

bool NFA::Runner::Run(int32_t const state, std::string_view const input) {
  visited_.emplace(state);
  if (input.empty() && state == nfa_.final_state_) {
    return true;
  }
  auto const& edges = nfa_.states_[state];
  for (auto const transition : edges[0]) {
    if (!visited_.contains(transition) && Run(transition, input)) {
      return true;
    }
  }
  if (input.empty()) {
    return false;
  }
  visited_.clear();
  auto const substr = input.substr(1);
  for (auto const transition : edges[input[0]]) {
    if (Run(transition, substr)) {
      return true;
    }
  }
  return false;
}

}  // namespace re3
