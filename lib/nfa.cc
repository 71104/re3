#include "lib/nfa.h"

#include <algorithm>
#include <cstdint>
#include <string_view>

#include "absl/container/flat_hash_set.h"

namespace re3 {

bool NFA::Run(std::string_view const input) const {
  return Runner(*this).Run(initial_state_, input);
}

namespace {

class VisitedSetSaver final {
 public:
  explicit VisitedSetSaver(absl::flat_hash_set<int32_t>& visited) : visited_(visited) {
    std::swap(visited_, saved_);
  }

  ~VisitedSetSaver() { std::swap(visited_, saved_); }

 private:
  absl::flat_hash_set<int32_t>& visited_;
  absl::flat_hash_set<int32_t> saved_;
};

}  // namespace

bool NFA::Runner::Run(int32_t const state, std::string_view const input) {
  visited_.emplace(state);
  if (input.empty() && state == nfa_.final_state_) {
    return true;
  }
  auto const& edges = nfa_.states_[state];
  for (auto const transition : edges[0]) {
    if (!visited_.contains(transition) && Run(transition, input)) {
      return true;
    }
  }
  if (input.empty()) {
    return false;
  }
  VisitedSetSaver vss{visited_};
  auto const substr = input.substr(1);
  for (auto const transition : edges[input[0]]) {
    if (Run(transition, substr)) {
      return true;
    }
  }
  return false;
}

}  // namespace re3
