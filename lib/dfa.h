#ifndef __RE3_LIB_DFA_H__
#define __RE3_LIB_DFA_H__

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "lib/automaton.h"

namespace re3 {

class DFA final : public AutomatonInterface {
 public:
  // `State` is represented by an array of 256 edges, one for every possible input character. Each
  // edge is an integer representing the next state, or a negative value if that edge doesn't exist.
  using State = std::array<int32_t, 256>;
  using States = std::vector<State>;

  explicit DFA() = default;

  explicit DFA(States states, int32_t const initial_state, int32_t const final_state)
      : states_(std::move(states)), initial_state_(initial_state), final_state_(final_state) {}

  DFA(DFA const &) = default;
  DFA &operator=(DFA const &) = default;
  DFA(DFA &&) noexcept = default;
  DFA &operator=(DFA &&) noexcept = default;

  bool Run(std::string_view input) const override;

 private:
  States states_;
  int32_t initial_state_ = 0;
  int32_t final_state_ = 0;
};

}  // namespace re3

#endif  // __RE3_LIB_DFA_H__
