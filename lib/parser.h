#ifndef __RE3_LIB_PARSER_H__
#define __RE3_LIB_PARSER_H__

#include <memory>
#include <string_view>

#include "absl/status/statusor.h"
#include "lib/automaton.h"

namespace re3 {

// Parses a regular expression and compiles it into a runnable automaton. The automaton is initially
// an `NFA` but it's automatically converted to a `DFA` if it's found to be deterministic. That is
// because DFAs run faster.
absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse(std::string_view pattern);

}  // namespace re3

#endif  // __RE3_LIB_PARSER_H__
