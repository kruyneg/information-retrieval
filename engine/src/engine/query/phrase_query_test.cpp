#include "engine/query/phrase_query.h"

#include <gtest/gtest.h>

#include "engine/indexing/posting_list.h"
#include "linguistics/lemmatization/mock_lemmatizer.h"
#include "linguistics/preprocessor.h"
#include "linguistics/tokenization/tokenizer_impl.h"

using namespace query;

class PhraseQueryTest : public testing::Test {
 public:
  PhraseQueryTest()
      : preprocessor(std::make_unique<linguistics::TokenizerImpl>(),
                     std::make_unique<linguistics::MockLemmatizer>()) {
    std::vector<std::vector<std::string>> docs{
        {"simple", "text"},
        {"very", "complex", "text"},
        {"simple", "another", "text"},
        {"hello", "world"},
    };
    for (indexing::DocID i = 0; i < docs.size(); ++i) {
      index.AddDocument(i, docs[i]);
    }
  }

 protected:
  indexing::InvertedIndex index;
  linguistics::Preprocessor preprocessor;
};

TEST_F(PhraseQueryTest, OneWordQuery) {
  const auto res = ExecutePhraseQuery({"text"}, index);
  const std::vector<indexing::DocID> expected{0, 1, 2};

  EXPECT_EQ(res.Decompress().docs(), expected);
}

TEST_F(PhraseQueryTest, PhraseQuery) {
  const auto res = ExecutePhraseQuery({"simple", "text"}, index);
  const std::vector<indexing::DocID> expected{0};

  EXPECT_EQ(res.Decompress().docs(), expected);
}

TEST_F(PhraseQueryTest, WrongOrderQuery) {
  const auto res = ExecutePhraseQuery({"text", "simple"}, index);

  EXPECT_TRUE(res.Decompress().docs().empty());
}

TEST_F(PhraseQueryTest, NonexistentWordQuery) {
  const auto res = ExecutePhraseQuery({"nonexistent"}, index);

  EXPECT_TRUE(res.Decompress().docs().empty());
}
