#include "absl/status/status.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/parser.h"
#include "lib/temp.h"
#include "lib/testing.h"

namespace {

using ::re3::Parse;
using ::re3::TempNFA;
using ::testing::Values;
using ::testing::status::StatusIs;

class ParserTest : public ::testing::TestWithParam<bool> {
 protected:
  explicit ParserTest() { TempNFA::force_nfa_for_testing = GetParam(); }
  ~ParserTest() { TempNFA::force_nfa_for_testing = false; }
};

TEST_P(ParserTest, Empty) {
  auto const status_or_pattern = Parse("");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("hello"));
  EXPECT_TRUE(pattern->Run(""));
}

TEST_P(ParserTest, SimpleCharacter) {
  auto const status_or_pattern = Parse("a");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST_P(ParserTest, AnotherSimpleCharacter) {
  auto const status_or_pattern = Parse("b");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST_P(ParserTest, InvalidEscapeCode) {
  EXPECT_THAT(Parse("\\a"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\T"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\R"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\N"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\V"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\F"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\X"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST_P(ParserTest, Digit) {
  auto const status_or_pattern = Parse("\\d");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("0"));
  EXPECT_TRUE(pattern->Run("1"));
  EXPECT_TRUE(pattern->Run("2"));
  EXPECT_TRUE(pattern->Run("3"));
  EXPECT_TRUE(pattern->Run("4"));
  EXPECT_TRUE(pattern->Run("5"));
  EXPECT_TRUE(pattern->Run("6"));
  EXPECT_TRUE(pattern->Run("7"));
  EXPECT_TRUE(pattern->Run("8"));
  EXPECT_TRUE(pattern->Run("9"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("d"));
  EXPECT_FALSE(pattern->Run("\\d"));
  EXPECT_FALSE(pattern->Run("\\0"));
}

TEST_P(ParserTest, NotDigit) {
  auto const status_or_pattern = Parse("\\D");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("0"));
  EXPECT_FALSE(pattern->Run("1"));
  EXPECT_FALSE(pattern->Run("2"));
  EXPECT_FALSE(pattern->Run("3"));
  EXPECT_FALSE(pattern->Run("4"));
  EXPECT_FALSE(pattern->Run("5"));
  EXPECT_FALSE(pattern->Run("6"));
  EXPECT_FALSE(pattern->Run("7"));
  EXPECT_FALSE(pattern->Run("8"));
  EXPECT_FALSE(pattern->Run("9"));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("D"));
  EXPECT_FALSE(pattern->Run("\\D"));
  EXPECT_FALSE(pattern->Run("\\0"));
}

TEST_P(ParserTest, WordCharacter) {
  auto const status_or_pattern = Parse("\\w");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("A"));
  EXPECT_TRUE(pattern->Run("B"));
  EXPECT_TRUE(pattern->Run("C"));
  EXPECT_TRUE(pattern->Run("D"));
  EXPECT_TRUE(pattern->Run("E"));
  EXPECT_TRUE(pattern->Run("F"));
  EXPECT_TRUE(pattern->Run("G"));
  EXPECT_TRUE(pattern->Run("H"));
  EXPECT_TRUE(pattern->Run("I"));
  EXPECT_TRUE(pattern->Run("J"));
  EXPECT_TRUE(pattern->Run("K"));
  EXPECT_TRUE(pattern->Run("L"));
  EXPECT_TRUE(pattern->Run("M"));
  EXPECT_TRUE(pattern->Run("N"));
  EXPECT_TRUE(pattern->Run("O"));
  EXPECT_TRUE(pattern->Run("P"));
  EXPECT_TRUE(pattern->Run("Q"));
  EXPECT_TRUE(pattern->Run("R"));
  EXPECT_TRUE(pattern->Run("S"));
  EXPECT_TRUE(pattern->Run("T"));
  EXPECT_TRUE(pattern->Run("U"));
  EXPECT_TRUE(pattern->Run("V"));
  EXPECT_TRUE(pattern->Run("W"));
  EXPECT_TRUE(pattern->Run("X"));
  EXPECT_TRUE(pattern->Run("Y"));
  EXPECT_TRUE(pattern->Run("Z"));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("c"));
  EXPECT_TRUE(pattern->Run("d"));
  EXPECT_TRUE(pattern->Run("e"));
  EXPECT_TRUE(pattern->Run("f"));
  EXPECT_TRUE(pattern->Run("g"));
  EXPECT_TRUE(pattern->Run("h"));
  EXPECT_TRUE(pattern->Run("i"));
  EXPECT_TRUE(pattern->Run("j"));
  EXPECT_TRUE(pattern->Run("k"));
  EXPECT_TRUE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("m"));
  EXPECT_TRUE(pattern->Run("n"));
  EXPECT_TRUE(pattern->Run("o"));
  EXPECT_TRUE(pattern->Run("p"));
  EXPECT_TRUE(pattern->Run("q"));
  EXPECT_TRUE(pattern->Run("r"));
  EXPECT_TRUE(pattern->Run("s"));
  EXPECT_TRUE(pattern->Run("t"));
  EXPECT_TRUE(pattern->Run("u"));
  EXPECT_TRUE(pattern->Run("v"));
  EXPECT_TRUE(pattern->Run("w"));
  EXPECT_TRUE(pattern->Run("x"));
  EXPECT_TRUE(pattern->Run("y"));
  EXPECT_TRUE(pattern->Run("z"));
  EXPECT_TRUE(pattern->Run("0"));
  EXPECT_TRUE(pattern->Run("1"));
  EXPECT_TRUE(pattern->Run("2"));
  EXPECT_TRUE(pattern->Run("3"));
  EXPECT_TRUE(pattern->Run("4"));
  EXPECT_TRUE(pattern->Run("5"));
  EXPECT_TRUE(pattern->Run("6"));
  EXPECT_TRUE(pattern->Run("7"));
  EXPECT_TRUE(pattern->Run("8"));
  EXPECT_TRUE(pattern->Run("9"));
  EXPECT_TRUE(pattern->Run("_"));
  EXPECT_FALSE(pattern->Run("."));
  EXPECT_FALSE(pattern->Run("-"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\w"));
}

TEST_P(ParserTest, NotWordCharacter) {
  auto const status_or_pattern = Parse("\\W");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("A"));
  EXPECT_FALSE(pattern->Run("B"));
  EXPECT_FALSE(pattern->Run("C"));
  EXPECT_FALSE(pattern->Run("D"));
  EXPECT_FALSE(pattern->Run("E"));
  EXPECT_FALSE(pattern->Run("F"));
  EXPECT_FALSE(pattern->Run("G"));
  EXPECT_FALSE(pattern->Run("H"));
  EXPECT_FALSE(pattern->Run("I"));
  EXPECT_FALSE(pattern->Run("J"));
  EXPECT_FALSE(pattern->Run("K"));
  EXPECT_FALSE(pattern->Run("L"));
  EXPECT_FALSE(pattern->Run("M"));
  EXPECT_FALSE(pattern->Run("N"));
  EXPECT_FALSE(pattern->Run("O"));
  EXPECT_FALSE(pattern->Run("P"));
  EXPECT_FALSE(pattern->Run("Q"));
  EXPECT_FALSE(pattern->Run("R"));
  EXPECT_FALSE(pattern->Run("S"));
  EXPECT_FALSE(pattern->Run("T"));
  EXPECT_FALSE(pattern->Run("U"));
  EXPECT_FALSE(pattern->Run("V"));
  EXPECT_FALSE(pattern->Run("W"));
  EXPECT_FALSE(pattern->Run("X"));
  EXPECT_FALSE(pattern->Run("Y"));
  EXPECT_FALSE(pattern->Run("Z"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("c"));
  EXPECT_FALSE(pattern->Run("d"));
  EXPECT_FALSE(pattern->Run("e"));
  EXPECT_FALSE(pattern->Run("f"));
  EXPECT_FALSE(pattern->Run("g"));
  EXPECT_FALSE(pattern->Run("h"));
  EXPECT_FALSE(pattern->Run("i"));
  EXPECT_FALSE(pattern->Run("j"));
  EXPECT_FALSE(pattern->Run("k"));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_FALSE(pattern->Run("m"));
  EXPECT_FALSE(pattern->Run("n"));
  EXPECT_FALSE(pattern->Run("o"));
  EXPECT_FALSE(pattern->Run("p"));
  EXPECT_FALSE(pattern->Run("q"));
  EXPECT_FALSE(pattern->Run("r"));
  EXPECT_FALSE(pattern->Run("s"));
  EXPECT_FALSE(pattern->Run("t"));
  EXPECT_FALSE(pattern->Run("u"));
  EXPECT_FALSE(pattern->Run("v"));
  EXPECT_FALSE(pattern->Run("w"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("y"));
  EXPECT_FALSE(pattern->Run("z"));
  EXPECT_FALSE(pattern->Run("0"));
  EXPECT_FALSE(pattern->Run("1"));
  EXPECT_FALSE(pattern->Run("2"));
  EXPECT_FALSE(pattern->Run("3"));
  EXPECT_FALSE(pattern->Run("4"));
  EXPECT_FALSE(pattern->Run("5"));
  EXPECT_FALSE(pattern->Run("6"));
  EXPECT_FALSE(pattern->Run("7"));
  EXPECT_FALSE(pattern->Run("8"));
  EXPECT_FALSE(pattern->Run("9"));
  EXPECT_FALSE(pattern->Run("_"));
  EXPECT_TRUE(pattern->Run("."));
  EXPECT_TRUE(pattern->Run("-"));
  EXPECT_TRUE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\w"));
}

TEST_P(ParserTest, Spacing) {
  auto const status_or_pattern = Parse("\\s");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\f"));
  EXPECT_TRUE(pattern->Run("\n"));
  EXPECT_TRUE(pattern->Run("\r"));
  EXPECT_TRUE(pattern->Run("\t"));
  EXPECT_TRUE(pattern->Run("\v"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("s"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\s"));
}

TEST_P(ParserTest, NotSpacing) {
  auto const status_or_pattern = Parse("\\S");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("\f"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("\r"));
  EXPECT_FALSE(pattern->Run("\t"));
  EXPECT_FALSE(pattern->Run("\v"));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("s"));
  EXPECT_TRUE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\s"));
}

TEST_P(ParserTest, HorizontalTab) {
  auto const status_or_pattern = Parse("\\t");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\t"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("t"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\t"));
}

TEST_P(ParserTest, CarriageReturn) {
  auto const status_or_pattern = Parse("\\r");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\r"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("r"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\r"));
}

TEST_P(ParserTest, LineFeed) {
  auto const status_or_pattern = Parse("\\n");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("\t"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("n"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\n"));
}

TEST_P(ParserTest, VerticalTab) {
  auto const status_or_pattern = Parse("\\v");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\v"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("v"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\v"));
}

TEST_P(ParserTest, FormFeed) {
  auto const status_or_pattern = Parse("\\f");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\f"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("f"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\f"));
}

TEST_P(ParserTest, InvalidHexCode) {
  EXPECT_THAT(Parse("\\xZ0"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("\\x0Z"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST_P(ParserTest, HexCode1) {
  auto const status_or_pattern = Parse("\\x12");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\x12"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\x12"));
}

TEST_P(ParserTest, HexCode2) {
  auto const status_or_pattern = Parse("\\xAF");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\xAF"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\xAF"));
}

TEST_P(ParserTest, HexCode3) {
  auto const status_or_pattern = Parse("\\xaf");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("\xAF"));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("\\xaf"));
}

TEST_P(ParserTest, AnyCharacter) {
  auto const status_or_pattern = Parse(".");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST_P(ParserTest, EmptyCharacterClass) {
  auto const status_or_pattern = Parse("[]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("ipsum"));
  EXPECT_FALSE(pattern->Run("[]"));
}

TEST_P(ParserTest, NegatedEmptyCharacterClass) {
  auto const status_or_pattern = Parse("[^]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("^"));
  EXPECT_FALSE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("ipsum"));
  EXPECT_FALSE(pattern->Run("[^]"));
}

TEST_P(ParserTest, CharacterClass) {
  auto const status_or_pattern = Parse("[lorem\xAF]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("o"));
  EXPECT_TRUE(pattern->Run("r"));
  EXPECT_TRUE(pattern->Run("e"));
  EXPECT_TRUE(pattern->Run("m"));
  EXPECT_TRUE(pattern->Run("\xAF"));
  EXPECT_FALSE(pattern->Run("\xBF"));
  EXPECT_FALSE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("[lorem]"));
}

TEST_P(ParserTest, NegatedCharacterClass) {
  auto const status_or_pattern = Parse("[^lorem\xAF]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_FALSE(pattern->Run("o"));
  EXPECT_FALSE(pattern->Run("r"));
  EXPECT_FALSE(pattern->Run("e"));
  EXPECT_FALSE(pattern->Run("m"));
  EXPECT_FALSE(pattern->Run("\xAF"));
  EXPECT_TRUE(pattern->Run("\xBF"));
  EXPECT_TRUE(pattern->Run("^"));
  EXPECT_FALSE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("^lorem"));
  EXPECT_FALSE(pattern->Run("[^lorem]"));
}

TEST_P(ParserTest, CharacterClassWithCircumflex) {
  auto const status_or_pattern = Parse("[ab^cd]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("^"));
  EXPECT_TRUE(pattern->Run("c"));
  EXPECT_TRUE(pattern->Run("d"));
  EXPECT_FALSE(pattern->Run("ab^cd"));
}

TEST_P(ParserTest, NegatedCharacterClassWithCircumflex) {
  auto const status_or_pattern = Parse("[^ab^cd]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("^"));
  EXPECT_FALSE(pattern->Run("c"));
  EXPECT_FALSE(pattern->Run("d"));
  EXPECT_TRUE(pattern->Run("x"));
  EXPECT_TRUE(pattern->Run("y"));
  EXPECT_FALSE(pattern->Run("ab^cd"));
}

TEST_P(ParserTest, CharacterClassWithEscapes) {
  auto const status_or_pattern = Parse("[a\\\\\\^\\$\\.\\(\\)\\[\\]\\{\\}\\|b]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("\\"));
  EXPECT_TRUE(pattern->Run("^"));
  EXPECT_TRUE(pattern->Run("$"));
  EXPECT_TRUE(pattern->Run("."));
  EXPECT_TRUE(pattern->Run("("));
  EXPECT_TRUE(pattern->Run(")"));
  EXPECT_TRUE(pattern->Run("["));
  EXPECT_TRUE(pattern->Run("]"));
  EXPECT_TRUE(pattern->Run("{"));
  EXPECT_TRUE(pattern->Run("}"));
  EXPECT_TRUE(pattern->Run("|"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("y"));
}

TEST_P(ParserTest, NegatedCharacterClassWithEscapes) {
  auto const status_or_pattern = Parse("[^a\\\\\\^\\$\\.\\(\\)\\[\\]\\{\\}\\|b]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("\\"));
  EXPECT_FALSE(pattern->Run("^"));
  EXPECT_FALSE(pattern->Run("$"));
  EXPECT_FALSE(pattern->Run("."));
  EXPECT_FALSE(pattern->Run("("));
  EXPECT_FALSE(pattern->Run(")"));
  EXPECT_FALSE(pattern->Run("["));
  EXPECT_FALSE(pattern->Run("]"));
  EXPECT_FALSE(pattern->Run("{"));
  EXPECT_FALSE(pattern->Run("}"));
  EXPECT_FALSE(pattern->Run("|"));
  EXPECT_TRUE(pattern->Run("x"));
  EXPECT_TRUE(pattern->Run("y"));
}

TEST_P(ParserTest, CharacterClassWithMoreEscapes) {
  auto const status_or_pattern = Parse("[\\t\\r\\n\\v\\f\\b\\x12\\xAF]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_TRUE(pattern->Run("\t"));
  EXPECT_TRUE(pattern->Run("\r"));
  EXPECT_TRUE(pattern->Run("\n"));
  EXPECT_TRUE(pattern->Run("\v"));
  EXPECT_TRUE(pattern->Run("\f"));
  EXPECT_TRUE(pattern->Run("\b"));
  EXPECT_TRUE(pattern->Run("\x12"));
  EXPECT_TRUE(pattern->Run("\xAF"));
  EXPECT_FALSE(pattern->Run("x"));
  EXPECT_FALSE(pattern->Run("y"));
}

TEST_P(ParserTest, NegatedCharacterClassWithMoreEscapes) {
  auto const status_or_pattern = Parse("[^\\t\\r\\n\\v\\f\\b\\x12\\xAF]");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("\t"));
  EXPECT_FALSE(pattern->Run("\r"));
  EXPECT_FALSE(pattern->Run("\n"));
  EXPECT_FALSE(pattern->Run("\v"));
  EXPECT_FALSE(pattern->Run("\f"));
  EXPECT_FALSE(pattern->Run("\b"));
  EXPECT_FALSE(pattern->Run("\x12"));
  EXPECT_FALSE(pattern->Run("\xAF"));
  EXPECT_TRUE(pattern->Run("x"));
  EXPECT_TRUE(pattern->Run("y"));
}

TEST_P(ParserTest, InvalidEscapeCodesInCharacterClass) {
  EXPECT_THAT(Parse("[\\"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\]"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\x"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\x]"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\x0Z]"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\xZ0]"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("[\\a]"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST_P(ParserTest, InvalidSpecialCharacter) {
  EXPECT_THAT(Parse("*"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("+"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("?"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse(")"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("]"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST_P(ParserTest, CharacterSequence) {
  auto const status_or_pattern = Parse("lorem");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST_P(ParserTest, CharacterSequenceWithDot) {
  auto const status_or_pattern = Parse("lo.em");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_TRUE(pattern->Run("lo-em"));
  EXPECT_TRUE(pattern->Run("lovem"));
  EXPECT_FALSE(pattern->Run("lodolorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST_P(ParserTest, KleeneStar) {
  auto const status_or_pattern = Parse("a*");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("aa"));
  EXPECT_TRUE(pattern->Run("aaa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("aba"));
  EXPECT_FALSE(pattern->Run("aabaa"));
}

TEST_P(ParserTest, CharacterSequenceWithStar) {
  auto const status_or_pattern = Parse("lo*rem");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lrem"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_TRUE(pattern->Run("loorem"));
  EXPECT_TRUE(pattern->Run("looorem"));
  EXPECT_FALSE(pattern->Run("larem"));
  EXPECT_FALSE(pattern->Run("loremlorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST_P(ParserTest, KleenePlus) {
  auto const status_or_pattern = Parse("a+");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("aa"));
  EXPECT_TRUE(pattern->Run("aaa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("aba"));
  EXPECT_FALSE(pattern->Run("aabaa"));
}

TEST_P(ParserTest, CharacterSequenceWithPlus) {
  auto const status_or_pattern = Parse("lo+rem");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_FALSE(pattern->Run("lrem"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_TRUE(pattern->Run("loorem"));
  EXPECT_TRUE(pattern->Run("looorem"));
  EXPECT_FALSE(pattern->Run("larem"));
  EXPECT_FALSE(pattern->Run("loremlorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST_P(ParserTest, Maybe) {
  auto const status_or_pattern = Parse("a?");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("ba"));
}

TEST_P(ParserTest, CharacterSequenceWithMaybe) {
  auto const status_or_pattern = Parse("lo?rem");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lrem"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("loorem"));
  EXPECT_FALSE(pattern->Run("looorem"));
  EXPECT_FALSE(pattern->Run("larem"));
  EXPECT_FALSE(pattern->Run("loremlorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST_P(ParserTest, EmptyOrEmpty) {
  auto const status_or_pattern = Parse("|");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("b"));
}

TEST_P(ParserTest, EmptyOrA) {
  auto const status_or_pattern = Parse("|a");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("aaa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("ba"));
}

TEST_P(ParserTest, AOrEmpty) {
  auto const status_or_pattern = Parse("a|");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("aaa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("ba"));
}

TEST_P(ParserTest, AOrB) {
  auto const status_or_pattern = Parse("a|b");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("aaa"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("a|b"));
  EXPECT_FALSE(pattern->Run("ba"));
  EXPECT_FALSE(pattern->Run("aba"));
  EXPECT_FALSE(pattern->Run("bab"));
}

TEST_P(ParserTest, LoremOrIpsum) {
  auto const status_or_pattern = Parse("lorem|ipsum");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("i"));
  EXPECT_TRUE(pattern->Run("ipsum"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("lorem|ipsum"));
  EXPECT_FALSE(pattern->Run("ipsumlorem"));
  EXPECT_FALSE(pattern->Run("ipsum|lorem"));
}

TEST_P(ParserTest, EmptyBrackets) {
  auto const status_or_pattern = Parse("()");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
}

TEST_P(ParserTest, UnmatchedBrackets) {
  EXPECT_THAT(Parse("("), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse(")"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse(")("), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("(()"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("())"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST_P(ParserTest, Brackets) {
  auto const status_or_pattern = Parse("(a)");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
}

TEST_P(ParserTest, IpsumInBrackets) {
  auto const status_or_pattern = Parse("lorem(ipsum)dolor");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("ipsum"));
  EXPECT_FALSE(pattern->Run("dolor"));
  EXPECT_FALSE(pattern->Run("loremdolor"));
  EXPECT_FALSE(pattern->Run("loremidolor"));
  EXPECT_TRUE(pattern->Run("loremipsumdolor"));
}

// TODO

TEST_P(ParserTest, EpsilonLoop) {
  auto const status_or_pattern = Parse("(|a)+");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("aa"));
  EXPECT_TRUE(pattern->Run("aaa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("bb"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("ba"));
}

// TODO

TEST_P(ParserTest, HeavyBacktracker) {
  auto const status_or_pattern = Parse(
      "a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_FALSE(pattern->Run("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
}

INSTANTIATE_TEST_SUITE_P(ParserTest, ParserTest, Values(false, true));

}  // namespace
