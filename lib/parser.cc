#include "lib/parser.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

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
  // Parses a hex digit. Used to parse hex escape codes.
  static absl::StatusOr<int> ParseHexDigit(int ch);

  TempNFA MakeSingleCharacterNFA(int ch);
  TempNFA MakeCharacterClassNFA(std::string_view chars);
  TempNFA MakeNegatedCharacterClassNFA(std::string_view chars);

  // Called by `Parse0` to parse character classes (i.e. square brackets).
  absl::StatusOr<TempNFA> ParseCharacterClass();

  // Called by `Parse0` to parse escape codes (e.g. `\d`, `\w`, etc.).
  absl::StatusOr<TempNFA> ParseEscape();

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

absl::StatusOr<int> Parser::ParseHexDigit(int const ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else {
    return absl::InvalidArgumentError("invalid hex digit");
  }
}

TempNFA Parser::MakeSingleCharacterNFA(int const ch) {
  int32_t const start = next_state_++;
  int32_t const stop = next_state_++;
  return TempNFA(
      {
          {start, MakeState({{ch, {stop}}})},
          {stop, MakeState({})},
      },
      start, stop);
}

TempNFA Parser::MakeCharacterClassNFA(std::string_view const chars) {
  int32_t const start = next_state_++;
  int32_t const stop = next_state_++;
  State state;
  for (auto const ch : chars) {
    state[ch].emplace_back(stop);
  }
  return TempNFA(
      {
          {start, std::move(state)},
          {stop, MakeState({})},
      },
      start, stop);
}

TempNFA Parser::MakeNegatedCharacterClassNFA(std::string_view const chars) {
  int32_t const start = next_state_++;
  int32_t const stop = next_state_++;
  State state;
  for (int ch = 1; ch < 256; ++ch) {
    state[ch].emplace_back(stop);
  }
  for (auto const ch : chars) {
    state[ch].clear();
  }
  return TempNFA(
      {
          {start, std::move(state)},
          {stop, MakeState({})},
      },
      start, stop);
}

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

absl::StatusOr<TempNFA> Parser::ParseEscape() {
  if (!absl::ConsumePrefix(&pattern_, "\\")) {
    return absl::InvalidArgumentError("expected \\");
  }
  if (pattern_.empty()) {
    return absl::InvalidArgumentError("invalid escape code");
  }
  int const ch = pattern_[0];
  pattern_.remove_prefix(1);
  switch (ch) {
    case '\\':
      return MakeSingleCharacterNFA('\\');
    case 'd':
      return MakeCharacterClassNFA("0123456789");
    case 'D':
      return MakeNegatedCharacterClassNFA("0123456789");
    case 'w':
      return MakeCharacterClassNFA(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
    case 'W':
      return MakeNegatedCharacterClassNFA(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
    case 's':
      // TODO: add Unicode spaces.
      return MakeCharacterClassNFA("\f\n\r\t\v");
    case 'S':
      // TODO: add Unicode spaces.
      return MakeNegatedCharacterClassNFA("\f\n\r\t\v");
    case 't':
      return MakeSingleCharacterNFA('\t');
    case 'r':
      return MakeSingleCharacterNFA('\r');
    case 'n':
      return MakeSingleCharacterNFA('\n');
    case 'v':
      return MakeSingleCharacterNFA('\v');
    case 'f':
      return MakeSingleCharacterNFA('\f');
      // TODO: handle word boundary (`\b`).
    case 'x': {
      if (pattern_.size() < 2) {
        return absl::InvalidArgumentError("invalid escape code");
      }
      auto const status_or_digit1 = ParseHexDigit(pattern_[0]);
      if (!status_or_digit1.ok()) {
        return std::move(status_or_digit1).status();
      }
      auto const status_or_digit2 = ParseHexDigit(pattern_[1]);
      if (!status_or_digit2.ok()) {
        return std::move(status_or_digit2).status();
      }
      int const ch = status_or_digit1.value() * 16 + status_or_digit2.value();
      return MakeSingleCharacterNFA(ch);
    }
      // TODO: handle Unicode escape codes.
    default:
      return absl::InvalidArgumentError("invalid escape code");
  }
}

absl::StatusOr<TempNFA> Parser::Parse0() {
  int32_t const start = next_state_++;
  if (pattern_.empty()) {
    return TempNFA({{start, MakeState({})}}, start, start);
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
      return TempNFA({{start, MakeState({})}}, start, start);
    case '[':
      return ParseCharacterClass();
    case '\\':
      return ParseEscape();
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

absl::StatusOr<TempNFA> Parser::Parse1() {
  auto status_or_nfa = Parse0();
  if (!status_or_nfa.ok()) {
    return status_or_nfa;
  }
  auto nfa = std::move(status_or_nfa).value();
  while (!pattern_.empty() && pattern_[0] != '|' && pattern_[0] != ')') {
    if (absl::ConsumePrefix(&pattern_, "*")) {
      nfa.RenameState(nfa.initial_state(), nfa.final_state());
      continue;
    }
    if (absl::ConsumePrefix(&pattern_, "+")) {
      nfa.AddEdge(0, nfa.final_state(), nfa.initial_state());
      continue;
    }
    if (absl::ConsumePrefix(&pattern_, "?")) {
      nfa.AddEdge(0, nfa.initial_state(), nfa.final_state());
      continue;
    }
    return nfa;
  }
  return nfa;
}

absl::StatusOr<TempNFA> Parser::Parse2() {
  auto status_or_nfa = Parse1();
  if (!status_or_nfa.ok()) {
    return status_or_nfa;
  }
  auto nfa = std::move(status_or_nfa).value();
  while (!pattern_.empty() && pattern_[0] != '|' && pattern_[0] != ')') {
    status_or_nfa = Parse1();
    if (!status_or_nfa.ok()) {
      return status_or_nfa;
    }
    nfa.Chain(std::move(status_or_nfa).value());
  }
  return nfa;
}

absl::StatusOr<TempNFA> Parser::Parse3() {
  auto status_or_nfa = Parse2();
  if (!status_or_nfa.ok()) {
    return status_or_nfa;
  }
  auto nfa = std::move(status_or_nfa).value();
  while (!pattern_.empty() && pattern_[0] != ')') {
    if (!absl::ConsumePrefix(&pattern_, "|")) {
      return absl::InvalidArgumentError("expected pipe operator");
    }
    status_or_nfa = Parse2();
    if (!status_or_nfa.ok()) {
      return status_or_nfa;
    }
    int32_t const initial_state = next_state_++;
    int32_t const final_state = next_state_++;
    nfa.Merge(std::move(status_or_nfa).value(), initial_state, final_state);
  }
  return nfa;
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
