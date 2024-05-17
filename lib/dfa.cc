#include "lib/dfa.h"

#include <string_view>

namespace re3 {

bool DFA::Run(std::string_view input) const {
  int32_t state = initial_state_;
  while (!input.empty()) {
    auto const transition = states_[state][input[0]];
    if (transition < 0) {
      return false;
    }
    state = transition;
    input.remove_prefix(1);
  }
  return state == final_state_;
}

}  // namespace re3
