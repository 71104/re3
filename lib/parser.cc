#include "lib/parser.h"

#include <cstdint>
#include <memory>
#include <string_view>

#include "absl/status/statusor.h"
#include "lib/automaton.h"
#include "lib/temp.h"

namespace re3 {

namespace {

// Parses a regular expression and compiles it into a runnable automaton.
class Parser {
 public:
  // Constructs a parser to parse the provided regular expression `pattern`.
  explicit Parser(std::string_view const pattern) : pattern_(pattern) {}

  // Parses the pattern provided at construction and returns a runnable automaton. The automaton is
  // initially an `NFA` but it's automatically converted to a `DFA` if it's found to be
  // deterministic. We do that because DFAs run faster.
  absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse();

 private:
  std::string_view pattern_;
  int32_t next_state_ = 0;
};

}  // namespace

absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse(std::string_view const pattern) {
  return Parser(pattern).Parse();
}

}  // namespace re3
