#include "lib/dfa.h"

#include <string_view>

namespace re3 {

bool DFA::Run(std::string_view input) const {
  int32_t state = initial_state_;
  while (!input.empty()) {
    int const ch = input[0];
    auto const &edges = states_[state];
    if (edges[ch] >= 0) {
      state = edges[ch];
    } else if (edges[0] >= 0) {
      state = edges[0];
    } else {
      continue;
    }
    input.remove_prefix(1);
  }
  return state == final_state_;
}

}  // namespace re3
