#include "lib/parser.h"

#include <cstdint>
#include <memory>
#include <string_view>

#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/strip.h"
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
  // Called by `Parse0` to parse character classes (i.e. square brackets).
  absl::StatusOr<TempNFA> ParseCharacterClass();

  // Parses single character, escape code, dot, round brackets, square brackets, or end of input.
  absl::StatusOr<TempNFA> Parse0();

  // Parses Kleene star, plus, question mark, or quantifier.
  absl::StatusOr<TempNFA> Parse1();

  // Parses sequences.
  absl::StatusOr<TempNFA> Parse2();

  // Parses the pipe operator.
  absl::StatusOr<TempNFA> Parse3();

  std::string_view pattern_;
  int32_t next_state_ = 0;
};

absl::StatusOr<TempNFA> Parser::ParseCharacterClass() {
  if (!absl::ConsumePrefix(&pattern_, "[")) {
    return absl::InvalidArgumentError("expected [");
  }
  int32_t const start = next_state_++;
  int32_t const stop = next_state_++;
  State state;
  bool const negated = absl::ConsumePrefix(&pattern_, "^");
  if (negated) {
    for (int ch = 1; ch < 256; ++ch) {
      state[ch].emplace_back(stop);
    }
  }
  while (!absl::ConsumePrefix(&pattern_, "]")) {
    if (pattern_.empty()) {
      return absl::InvalidArgumentError("unmatched square bracket");
    }
    // TODO: character ranges
    if (negated) {
      state[pattern_[0]].clear();
    } else {
      state[pattern_[0]].emplace_back(stop);
    }
  }
  return TempNFA(
      {
          {start, std::move(state)},
          {stop, {}},
      },
      start, stop);
}

absl::StatusOr<TempNFA> Parser::Parse0() {
  int32_t const start = next_state_++;
  if (pattern_.empty()) {
    return TempNFA({{start, {}}}, start, start);
  }
  if (absl::ConsumePrefix(&pattern_, "(")) {
    auto result = Parse3();
    if (!result.ok()) {
      return std::move(result).status();
    }
    if (!absl::ConsumePrefix(&pattern_, ")")) {
      return absl::InvalidArgumentError("unmatched parens");
    }
    return result;
  }
  int32_t const stop = next_state_++;
  if (absl::ConsumePrefix(&pattern_, ".")) {
    absl::flat_hash_map<uint8_t, absl::InlinedVector<int32_t, 1>> edges;
    for (int ch = 1; ch < 256; ++ch) {
      edges.try_emplace(ch, absl::InlinedVector<int32_t, 1>{stop});
    }
    return TempNFA(
        {
            {start, MakeState(std::move(edges))},
            {stop, {}},
        },
        start, stop);
  }
  int const ch = pattern_[0];
  switch (ch) {
    case ')':
    case '|':
      return TempNFA({{start, {}}}, start, start);
    case '[':
      return ParseCharacterClass();
    case '*':
    case '+':
      return absl::InvalidArgumentError("Kleene operator in invalid position");
    case '?':
      return absl::InvalidArgumentError("question mark operator in invalid position");
    case '^':
    case '$':
      return absl::InvalidArgumentError("anchors are disallowed in this position");
    default:
      pattern_.remove_prefix(1);
      return TempNFA(
          {
              {start, MakeState({{ch, {stop}}})},
              {stop, {}},
          },
          start, stop);
  }
}

absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parser::Parse() {
  auto status_or_nfa = Parse3();
  if (status_or_nfa.ok()) {
    return std::move(status_or_nfa).value().Finalize();
  } else {
    return std::move(status_or_nfa).status();
  }
}

}  // namespace

absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse(std::string_view const pattern) {
  return Parser(pattern).Parse();
}

}  // namespace re3
