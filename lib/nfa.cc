#include "lib/nfa.h"

#include <cstdint>
#include <string_view>

namespace re3 {

bool NFA::Run(std::string_view const input) const { return RunInternal(initial_state_, input); }

bool NFA::RunInternal(int32_t const state, std::string_view const input) const {
  if (input.empty() && state == final_state_) {
    return true;
  }
  auto const& edges = states_[state];
  for (auto const transition : edges[0]) {
    if (RunInternal(transition, input)) {
      return true;
    }
  }
  if (input.empty()) {
    return false;
  }
  auto const substr = input.substr(1);
  for (auto const transition : edges[input[0]]) {
    if (RunInternal(transition, substr)) {
      return true;
    }
  }
  return false;
}

}  // namespace re3
