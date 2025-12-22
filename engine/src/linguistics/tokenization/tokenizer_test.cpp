#include "linguistics/tokenization/tokenizer_impl.h"
#include "third_party/gtest/googletest/include/gtest/gtest.h"

using namespace linguistics;

class TokenizerTest : public ::testing::Test {
 protected:
  TokenizerImpl tokenizer;
};

TEST_F(TokenizerTest, WordTokenization) {
  std::string text = "Simple words for test";

  const auto res = tokenizer.Tokenize(text);
  const std::vector<std::string> expected{"Simple", "words", "for", "test"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, Punctuation) {
  std::string text = "Simple, text. With - punctuation...?!";

  const auto res = tokenizer.Tokenize(text);
  const std::vector<std::string> expected{"Simple", "text", "With",
                                          "punctuation"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, MultipleSpaces) {
  const auto res = tokenizer.Tokenize("   Simple   words    test   ");
  const std::vector<std::string> expected{"Simple", "words", "test"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, EmptyString) {
  const auto res = tokenizer.Tokenize("");
  EXPECT_TRUE(res.empty());
}

TEST_F(TokenizerTest, OnlyWhitespace) {
  const auto res = tokenizer.Tokenize("     \t \n ");
  EXPECT_TRUE(res.empty());
}

TEST_F(TokenizerTest, SingleWord) {
  const auto res = tokenizer.Tokenize("token");
  const std::vector<std::string> expected{"token"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, HandlesWhitespaceCharacters) {
  const auto res = tokenizer.Tokenize("hello\tworld\nagain");
  const std::vector<std::string> expected{"hello", "world", "again"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, AllowsDigitsInTokens) {
  const auto res = tokenizer.Tokenize("C++ python3 100");
  const std::vector<std::string> expected{"C++", "python3", "100"};
  EXPECT_EQ(res, expected);
}

TEST_F(TokenizerTest, Utf8Text) {
  const auto res = tokenizer.Tokenize("Простой текст");
  const std::vector<std::string> expected{"Простой", "текст"};
  EXPECT_EQ(res, expected);
}
