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

TEST_P(ParserTest, InvalidSpecialCharacter) {
  EXPECT_THAT(Parse("*"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("+"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("?"), StatusIs(absl::StatusCode::kInvalidArgument));
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

INSTANTIATE_TEST_SUITE_P(ParserTest, ParserTest, Values(false, true));

}  // namespace
