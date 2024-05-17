#ifndef __RE3_LIB_TEMP_H__
#define __RE3_LIB_TEMP_H__

#include <cstdint>
#include <memory>
#include <utility>

#include "absl/container/btree_map.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "lib/automaton.h"
#include "lib/dfa.h"
#include "lib/nfa.h"

namespace re3 {

using State = NFA::State;

// Convenience function to build an NFA `State` without listing all 256 input characters.
//
// Example:
//
//   // Build a State accepting characters 'a' and 'f'.
//   MakeState({
//     {'a', {12, 3}},
//     {'f', {4, 56, 7}},
//   })
//
State MakeState(absl::flat_hash_map<uint8_t, absl::InlinedVector<int32_t, 1>> &&edges) {
  State state;
  for (auto &[ch, edge] : edges) {
    state[ch] = std::move(edge);
  }
  return state;
}

// Represents an NFA under construction.
//
// `TempNFA` is used by the `Parser` to perform various manipulations during construction.
class TempNFA {
 public:
  using States = absl::btree_map<int32_t, State>;

  explicit TempNFA() = default;

  explicit TempNFA(States states, int32_t const initial_state, int32_t const final_state)
      : states_(std::move(states)), initial_state_(initial_state), final_state_(final_state) {}

  TempNFA(TempNFA const &) = default;
  TempNFA &operator=(TempNFA const &) = default;
  TempNFA(TempNFA &&) noexcept = default;
  TempNFA &operator=(TempNFA &&) noexcept = default;

  int32_t initial_state() const { return initial_state_; }
  int32_t final_state() const { return final_state_; }

  // Checks if the automaton is deterministic (that is, for each state each label is at most on one
  // edge and either there's no epsilon-move or the epsilon-move is the only one).
  bool IsDeterministic() const;

  // Finalizes this automaton by converting it into a `DFA` object if it's deterministic or an `NFA`
  // if it's not. `next_state` is the number of the new (caller-generated) final state.
  std::unique_ptr<AutomatonInterface> Finalize() &&;

 private:
  // Finalizes this NFA by converting it to an `DFA` object, assuming the automaton is deterministic
  // (`IsDeterministic()` must return true).
  DFA ToDFA() &&;

  // Finalizes this NFA by converting it to an `NFA` object.
  NFA ToNFA() &&;

  States states_;
  int32_t initial_state_ = 0;
  int32_t final_state_ = 0;
};

}  // namespace re3

#endif  // __RE3_LIB_TEMP_H__
