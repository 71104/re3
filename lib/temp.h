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
State MakeState(absl::flat_hash_map<uint8_t, absl::InlinedVector<int32_t, 1>> &&edges);

// Represents an NFA under construction.
//
// `TempNFA` is used by the `Parser` to perform various manipulations during construction.
class TempNFA {
 public:
  using States = absl::btree_map<int32_t, State>;

  // TESTS ONLY: force `Finalize()` to always generate an NFA even if it's deterministic. Defaults
  // to false.
  static bool force_nfa_for_testing;

  explicit TempNFA() = default;

  explicit TempNFA(States states, int const initial_state, int const final_state)
      : states_(std::move(states)), initial_state_(initial_state), final_state_(final_state) {}

  TempNFA(TempNFA const &) = default;
  TempNFA &operator=(TempNFA const &) = default;
  TempNFA(TempNFA &&) noexcept = default;
  TempNFA &operator=(TempNFA &&) noexcept = default;

  int initial_state() const { return initial_state_; }
  int final_state() const { return final_state_; }

  // Checks if the automaton is deterministic (that is, for each state each label is at most on one
  // edge and either there's no epsilon-move or the epsilon-move is the only one).
  bool IsDeterministic() const;

  // Renames state `old_name` to `new_name`.
  void RenameState(int old_name, int new_name);

  // Renames all states of this NFA so that they are greater than or equal to `next_state`. The
  // `next_state` variable is incremented accordingly.
  void RenameAllStates(int32_t *next_state);

  // Adds a new edge labeled with character `label` from state `from` to state `to`.
  void AddEdge(uint8_t label, int from, int to);

  // Chains this NFA with `other` by merging the final state of the former with the initial state of
  // the latter. The resulting automaton recognizes concatenations of the strings originally
  // recognized by `this` and those originally recognized by `other`.
  //
  // WARNING: this method will NOT rename states as necessary to avoid collisions; the caller is
  // responsible for calling `RenameAllStates` beforehand.
  void Chain(TempNFA other);

  // Merges `other` with this automaton, resulting in a new automaton that accepts both the strings
  // of the original `this` and those of `other`. `initial_state` and `final_state` must be newly
  // generated by the caller.
  void Merge(TempNFA &&other, int initial_state, int final_state);

  // Finalizes this automaton by converting it into a `DFA` object if it's deterministic or an `NFA`
  // if it's not.
  std::unique_ptr<AutomatonInterface> Finalize() &&;

 private:
  // Adds a state and its edges to the NFA, or merges it with an existing one.
  void MergeState(int state, State &&edges);

  // Checks whether the given state has exactly one outbound edge towards a single destination
  // state, and that edge is epsilon-labeled. In that case `CollapseNextEpsilonMove` will collpase
  // it into the destination state.
  bool HasOnlyOneEpsilonMove(int state) const;

  // Auxiliary method for the implementation of `CollapseEpsilonMoves`.
  bool CollapseNextEpsilonMove();

  // Collapses epsilon-moves by merging states that are separated by such a move.
  //
  // REQUIRES: the automaton must be deterministic, in which case two states separated by an
  // epsilon-move can't have any other edge in between.
  void CollapseEpsilonMoves();

  // Finalizes this NFA by converting it to an `DFA` object, assuming the automaton is deterministic
  // (`IsDeterministic()` must return true) and has no epsilon-moves (`CollapseEpsilonMoves()` must
  // have been called).
  DFA ToDFA() &&;

  // Finalizes this NFA by converting it to an `NFA` object.
  NFA ToNFA() &&;

  States states_;
  int32_t initial_state_ = 0;
  int32_t final_state_ = 0;
};

}  // namespace re3

#endif  // __RE3_LIB_TEMP_H__
