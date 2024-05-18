#include "lib/dfa.h"

#include <string_view>

namespace re3 {

bool DFA::Run(std::string_view input) const {
  int32_t state = initial_state_;
  while (!input.empty()) {
    auto const epsilon_transition = states_[state][0];
    if (epsilon_transition < 0) {
      auto const transition = states_[state][input[0]];
      if (transition < 0) {
        return false;
      }
      state = transition;
      input.remove_prefix(1);
    } else {
      state = epsilon_transition;
    }
  }
  while (state != final_state_) {
    state = states_[state][0];
    if (state < 0) {
      return false;
    }
  }
  return true;
}

}  // namespace re3
