#include "lib/nfa.h"

#include <cstdint>
#include <string_view>

#include "absl/container/flat_hash_set.h"

namespace re3 {

bool NFA::Run(std::string_view const input) const {
  absl::flat_hash_set<int32_t> visited;
  return RunInternal(initial_state_, visited, input);
}

namespace {

// Scoped object to push the current state to and pop it from the visited set.
class VisitFlag final {
 public:
  explicit VisitFlag(absl::flat_hash_set<int32_t>& visited, int32_t const state)
      : visited_(visited), state_(state) {
    visited_.emplace(state_);
  }

  ~VisitFlag() { visited_.erase(state_); }

 private:
  absl::flat_hash_set<int32_t>& visited_;
  int32_t state_;
};

}  // namespace

bool NFA::RunInternal(int32_t const state, absl::flat_hash_set<int32_t>& visited,
                      std::string_view const input) const {
  if (input.empty() && state == final_state_) {
    return true;
  }
  auto const& edges = states_[state];
  {
    VisitFlag vf{visited, state};
    for (auto const transition : edges[0]) {
      if (!visited.contains(transition) && RunInternal(transition, visited, input)) {
        return true;
      }
    }
  }
  if (input.empty()) {
    return false;
  }
  absl::flat_hash_set<int32_t> new_visited;
  auto const substr = input.substr(1);
  for (auto const transition : edges[input[0]]) {
    if (RunInternal(transition, new_visited, substr)) {
      return true;
    }
  }
  return false;
}

}  // namespace re3
