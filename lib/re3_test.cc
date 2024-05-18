#include "absl/status/status.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/parser.h"
#include "lib/testing.h"

namespace {

using ::re3::Parse;
using ::testing::status::StatusIs;

TEST(ParserTest, Empty) {
  auto const status_or_pattern = Parse("");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("hello"));
  EXPECT_TRUE(pattern->Run(""));
}

TEST(ParserTest, SimpleCharacter) {
  auto const status_or_pattern = Parse("a");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST(ParserTest, AnotherSimpleCharacter) {
  auto const status_or_pattern = Parse("b");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST(ParserTest, AnyCharacter) {
  auto const status_or_pattern = Parse(".");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
  EXPECT_FALSE(pattern->Run(""));
}

TEST(ParserTest, InvalidSpecialCharacter) {
  EXPECT_THAT(Parse("*"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("+"), StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(Parse("?"), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(ParserTest, CharacterSequence) {
  auto const status_or_pattern = Parse("lorem");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("l"));
  EXPECT_TRUE(pattern->Run("lorem"));
  EXPECT_FALSE(pattern->Run("loremipsum"));
  EXPECT_FALSE(pattern->Run("dolorloremipsum"));
}

TEST(ParserTest, CharacterSequenceWithDot) {
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

TEST(ParserTest, KleeneStar) {
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

TEST(ParserTest, KleenePlus) {
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

TEST(ParserTest, Maybe) {
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

TEST(ParserTest, EmptyOrEmpty) {
  auto const status_or_pattern = Parse("|");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("b"));
}

TEST(ParserTest, EmptyOrA) {
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

TEST(ParserTest, AOrEmpty) {
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

TEST(ParserTest, AOrB) {
  auto const status_or_pattern = Parse("a|b");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("aaa"));
  EXPECT_TRUE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
  EXPECT_FALSE(pattern->Run("ba"));
  EXPECT_FALSE(pattern->Run("aba"));
  EXPECT_FALSE(pattern->Run("bab"));
}

TEST(ParserTest, EmptyBrackets) {
  auto const status_or_pattern = Parse("()");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_TRUE(pattern->Run(""));
  EXPECT_FALSE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("aa"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("ab"));
}

TEST(ParserTest, Brackets) {
  auto const status_or_pattern = Parse("(a)");
  EXPECT_OK(status_or_pattern);
  auto const& pattern = status_or_pattern.value();
  EXPECT_FALSE(pattern->Run(""));
  EXPECT_TRUE(pattern->Run("a"));
  EXPECT_FALSE(pattern->Run("b"));
  EXPECT_FALSE(pattern->Run("anchor"));
  EXPECT_FALSE(pattern->Run("banana"));
}

// TODO

}  // namespace
