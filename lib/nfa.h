#ifndef __RE3_LIB_NFA_H__
#define __RE3_LIB_NFA_H__

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "lib/automaton.h"

namespace re3 {

class NFA final : public AutomatonInterface {
 public:
  using State = std::array<absl::InlinedVector<int32_t, 1>, 256>;
  using States = std::vector<State>;

  explicit NFA() = default;

  explicit NFA(States states, int32_t const initial_state, int32_t const final_state)
      : states_(std::move(states)), initial_state_(initial_state), final_state_(final_state) {}

  NFA(NFA const &) = default;
  NFA &operator=(NFA const &) = default;
  NFA(NFA &&) noexcept = default;
  NFA &operator=(NFA &&) noexcept = default;

  bool Run(std::string_view input) const override;

 private:
  bool RunInternal(int32_t state, std::string_view input) const;

  States states_;
  int32_t initial_state_ = 0;
  int32_t final_state_ = 0;
};

}  // namespace re3

#endif  // __RE3_LIB_NFA_H__
