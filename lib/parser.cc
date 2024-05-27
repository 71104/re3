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
  static inline int constexpr kMaxNumericQuantifier = 1000;

  // Constructs a parser to parse the provided regular expression `pattern`.
  explicit Parser(std::string_view const pattern) : pattern_(pattern) {}

  // Parses the pattern provided at construction and returns a runnable automaton. The automaton is
  // initially an `NFA` but it's automatically converted to a `DFA` if it's found to be
  // deterministic. We do that because DFAs run faster.
  absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse();

 private:
  // Parses a hex digit. Used by `ParseHexCode` to parse hex escape codes.
  static absl::StatusOr<int> ParseHexDigit(int ch);

  // Parses the next two characters as a hex byte. Used to parse hex escape codes.
  absl::StatusOr<int> ParseHexCode();

  TempNFA MakeSingleCharacterNFA(int ch);
  TempNFA MakeCharacterClassNFA(std::string_view chars);
  TempNFA MakeNegatedCharacterClassNFA(std::string_view chars);

  static absl::Status UpdateCharacterClassEdge(bool negated, State* start_state, uint8_t ch,
                                               int stop_state_num);

  // Called by `ParseCharacterClass` to parse escape codes. `negated` indicates whether the
  // character class is negated (i.e. it begins with ^). `state` is the NFA state to update with the
  // characters resulting from the escape code. Returns an error status if the escape code is
  // invalid.
  //
  // REQUIRES: the backslash before the escape code must have already been consumed.
  absl::Status ParseCharacterClassEscapeCode(bool negated, State* start_state, int stop_state_num);

  // Called by `Parse0` to parse character classes (i.e. square brackets).
  absl::StatusOr<TempNFA> ParseCharacterClass();

  // Called by `Parse0` to parse escape codes (e.g. `\d`, `\w`, etc.).
  absl::StatusOr<TempNFA> ParseEscape();

  // Parses single character, escape code, dot, round brackets, square brackets, or end of input.
  absl::StatusOr<TempNFA> Parse0();

  // Parses the content of the curly braces in quantifiers.
  absl::StatusOr<std::pair<int, int>> ParseQuantifier();

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

absl::StatusOr<int> Parser::ParseHexCode() {
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
  pattern_.remove_prefix(2);
  return status_or_digit1.value() * 16 + status_or_digit2.value();
}

TempNFA Parser::MakeSingleCharacterNFA(int const ch) {
  int const start = next_state_++;
  int const stop = next_state_++;
  return TempNFA(
      {
          {start, MakeState({{ch, {stop}}})},
          {stop, MakeState({})},
      },
      start, stop);
}

TempNFA Parser::MakeCharacterClassNFA(std::string_view const chars) {
  int const start = next_state_++;
  int const stop = next_state_++;
  State state;
  for (uint8_t const ch : chars) {
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
  int const start = next_state_++;
  int const stop = next_state_++;
  State state;
  for (int ch = 1; ch < 256; ++ch) {
    state[ch].emplace_back(stop);
  }
  for (uint8_t const ch : chars) {
    state[ch].clear();
  }
  return TempNFA(
      {
          {start, std::move(state)},
          {stop, MakeState({})},
      },
      start, stop);
}

absl::Status Parser::UpdateCharacterClassEdge(bool const negated, State* const start_state,
                                              uint8_t const ch, int const stop_state_num) {
  if (negated) {
    (*start_state)[ch].clear();
  } else {
    (*start_state)[ch].emplace_back(stop_state_num);
  }
  return absl::OkStatus();
}

absl::Status Parser::ParseCharacterClassEscapeCode(bool const negated, State* const start_state,
                                                   int const stop_state_num) {
  if (pattern_.empty()) {
    return absl::InvalidArgumentError("invalid escape code");
  }
  uint8_t const ch = pattern_[0];
  pattern_.remove_prefix(1);
  switch (ch) {
    case '\\':
    case '^':
    case '$':
    case '.':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '|':
      return UpdateCharacterClassEdge(negated, start_state, ch, stop_state_num);
    case 't':
      return UpdateCharacterClassEdge(negated, start_state, '\t', stop_state_num);
    case 'r':
      return UpdateCharacterClassEdge(negated, start_state, '\r', stop_state_num);
    case 'n':
      return UpdateCharacterClassEdge(negated, start_state, '\n', stop_state_num);
    case 'v':
      return UpdateCharacterClassEdge(negated, start_state, '\v', stop_state_num);
    case 'f':
      return UpdateCharacterClassEdge(negated, start_state, '\f', stop_state_num);
    case 'b':
      return UpdateCharacterClassEdge(negated, start_state, '\b', stop_state_num);
    case 'x': {
      auto status_or_code = ParseHexCode();
      if (!status_or_code.ok()) {
        return std::move(status_or_code).status();
      }
      return UpdateCharacterClassEdge(negated, start_state, status_or_code.value(), stop_state_num);
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return absl::InvalidArgumentError("backreferences are not supported");
    default:
      return absl::InvalidArgumentError("invalid escape code");
  }
}

absl::StatusOr<TempNFA> Parser::ParseCharacterClass() {
  if (!absl::ConsumePrefix(&pattern_, "[")) {
    return absl::InvalidArgumentError("expected [");
  }
  int const start = next_state_++;
  int const stop = next_state_++;
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
    if (absl::ConsumePrefix(&pattern_, "\\")) {
      auto const status = ParseCharacterClassEscapeCode(negated, &state, stop);
      if (!status.ok()) {
        return status;
      }
    } else {
      uint8_t const ch1 = pattern_[0];
      pattern_.remove_prefix(1);
      if (pattern_.size() > 2 && pattern_[0] == '-' && pattern_[1] != ']') {
        pattern_.remove_prefix(2);
        // TODO: ranges
        return absl::UnimplementedError("ranges in character classes");
      } else {
        if (negated) {
          state[ch1].clear();
        } else {
          state[ch1].emplace_back(stop);
        }
      }
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
    case '^':
    case '$':
    case '.':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '|':
      return MakeSingleCharacterNFA(ch);
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
      auto status_or_code = ParseHexCode();
      if (!status_or_code.ok()) {
        return std::move(status_or_code).status();
      }
      return MakeSingleCharacterNFA(status_or_code.value());
    }
    // TODO: handle Unicode escape codes.
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return absl::InvalidArgumentError("backreferences are not supported");
    default:
      return absl::InvalidArgumentError("invalid escape code");
  }
}

absl::StatusOr<TempNFA> Parser::Parse0() {
  int const start = next_state_++;
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
  int const stop = next_state_++;
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
    case ']':
      return absl::InvalidArgumentError("unmatched square bracket");
    case '{':
    case '}':
      return absl::InvalidArgumentError("curly brackets in invalid position");
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

absl::StatusOr<std::pair<int, int>> Parser::ParseQuantifier() {
  if (absl::ConsumePrefix(&pattern_, "}")) {
    return std::make_pair(-1, -1);
  }
  if (pattern_.empty() || !absl::ascii_isdigit(pattern_[0])) {
    return absl::InvalidArgumentError("invalid quantifier");
  }
  int ch = pattern_[0];
  int min = ch - '0';
  pattern_.remove_prefix(1);
  if (ch != 0) {
    while (!pattern_.empty() && absl::ascii_isdigit(pattern_[0])) {
      min = min * 10 + (pattern_[0] - '0');
      if (min > kMaxNumericQuantifier) {
        return absl::InvalidArgumentError(
            "numeric quantifiers greater than 1000 are not supported");
      }
      pattern_.remove_prefix(1);
    }
  }
  if (absl::ConsumePrefix(&pattern_, "}")) {
    return std::make_pair(min, min);
  }
  if (!absl::ConsumePrefix(&pattern_, ",")) {
    return absl::InvalidArgumentError("invalid quantifier");
  }
  if (absl::ConsumePrefix(&pattern_, "}")) {
    return std::make_pair(min, -1);
  }
  if (pattern_.empty() || !absl::ascii_isdigit(pattern_[0])) {
    return absl::InvalidArgumentError("invalid quantifier");
  }
  ch = pattern_[0];
  int max = ch - '0';
  pattern_.remove_prefix(1);
  if (ch != 0) {
    while (!pattern_.empty() && absl::ascii_isdigit(pattern_[0])) {
      max = max * 10 + (pattern_[0] - '0');
      if (max > kMaxNumericQuantifier) {
        return absl::InvalidArgumentError(
            "numeric quantifiers greater than 1000 are not supported");
      }
      pattern_.remove_prefix(1);
    }
  }
  if (absl::ConsumePrefix(&pattern_, "}")) {
    return std::make_pair(min, max);
  } else {
    return absl::InvalidArgumentError("invalid quantifier");
  }
}

absl::StatusOr<TempNFA> Parser::Parse1() {
  auto status_or_nfa = Parse0();
  if (!status_or_nfa.ok()) {
    return status_or_nfa;
  }
  auto nfa = std::move(status_or_nfa).value();
  if (pattern_.empty()) {
    return nfa;
  }
  if (absl::ConsumePrefix(&pattern_, "*")) {
    nfa.RenameState(nfa.initial_state(), nfa.final_state());
  } else if (absl::ConsumePrefix(&pattern_, "+")) {
    nfa.AddEdge(0, nfa.final_state(), nfa.initial_state());
  } else if (absl::ConsumePrefix(&pattern_, "?")) {
    nfa.AddEdge(0, nfa.initial_state(), nfa.final_state());
  } else if (absl::ConsumePrefix(&pattern_, "{")) {
    auto const status_or_quantifier = ParseQuantifier();
    if (!status_or_quantifier.ok()) {
      return std::move(status_or_quantifier).status();
    }
    auto const [min, max] = status_or_quantifier.value();
    if (min < 0) {
      if (max >= 0) {
        return absl::InvalidArgumentError("invalid quantifier");
      }
      nfa.RenameState(nfa.initial_state(), nfa.final_state());
    } else {
      auto piece = std::move(nfa);
      int const start = next_state_++;
      nfa = TempNFA({{start, MakeState({})}}, start, start);
      for (int i = 0; i < min; ++i) {
        piece.RenameAllStates(&next_state_);
        nfa.Chain(piece);
      }
      if (max < 0) {
        piece.RenameState(piece.initial_state(), piece.final_state());
        piece.RenameAllStates(&next_state_);
        nfa.Chain(std::move(piece));
      } else {
        if (max < min) {
          return absl::InvalidArgumentError("invalid quantifier");
        }
        piece.AddEdge(0, piece.initial_state(), piece.final_state());
        for (int i = min; i < max; ++i) {
          piece.RenameAllStates(&next_state_);
          nfa.Chain(piece);
        }
      }
    }
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
    int const initial_state = next_state_++;
    int const final_state = next_state_++;
    nfa.Merge(std::move(status_or_nfa).value(), initial_state, final_state);
  }
  return nfa;
}

absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parser::Parse() {
  auto status_or_nfa = Parse3();
  if (!status_or_nfa.ok()) {
    return std::move(status_or_nfa).status();
  }
  if (!pattern_.empty()) {
    return absl::InvalidArgumentError("expected end of string");
  }
  return std::move(status_or_nfa).value().Finalize();
}

}  // namespace

absl::StatusOr<std::unique_ptr<AutomatonInterface>> Parse(std::string_view const pattern) {
  return Parser(pattern).Parse();
}

}  // namespace re3
