#include "lib/temp.h"

#include <memory>
#include <utility>

#include "lib/automaton.h"
#include "lib/dfa.h"
#include "lib/nfa.h"

namespace re3 {

bool TempNFA::IsDeterministic() const {
  for (auto const& [state, edges] : states_) {
    if (edges[0].size() > 1) {
      return false;
    }
    bool const has_epsilon = !edges[0].empty();
    for (int ch = 1; ch < 256; ++ch) {
      auto const& edge = edges[ch];
      if (edge.size() > 1 || (!edge.empty() && has_epsilon)) {
        return false;
      }
    }
  }
  return true;
}

std::unique_ptr<AutomatonInterface> TempNFA::Finalize() && {
  if (IsDeterministic()) {
    return std::make_unique<DFA>(std::move(*this).ToDFA());
  } else {
    return std::make_unique<NFA>(std::move(*this).ToNFA());
  }
}

}  // namespace re3
