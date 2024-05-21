#include "lib/dfa.h"

#include <memory>
#include <string_view>

namespace re3 {

std::unique_ptr<AutomatonInterface> DFA::Clone() const { return std::make_unique<DFA>(*this); }

bool DFA::Run(std::string_view input) const {
  int32_t state = initial_state_;
  while (!input.empty()) {
    auto const epsilon_transition = states_[state][0];
    if (epsilon_transition < 0) {
      uint8_t const ch = input[0];
      auto const transition = states_[state][ch];
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
