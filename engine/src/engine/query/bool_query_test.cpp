#include "engine/query/bool_query.h"

#include <gtest/gtest.h>

#include "engine/indexing/inverted_index.h"
#include "engine/indexing/posting_list.h"
#include "linguistics/lemmatization/mock_lemmatizer.h"
#include "linguistics/preprocessor.h"
#include "linguistics/tokenization/tokenizer_impl.h"

using namespace query;

class BoolQueryTest : public testing::Test {
 public:
  BoolQueryTest()
      : preprocessor(std::make_unique<linguistics::TokenizerImpl>(),
                     std::make_unique<linguistics::MockLemmatizer>()) {
    std::vector<std::vector<std::string>> docs{
        {"simple", "text"},
        {"very", "complex", "text"},
        {"another", "simple", "text"},
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

TEST_F(BoolQueryTest, OneWordQuery) {
  auto q = BoolQuery::Parse("text", preprocessor);
  const auto list = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 1, 2};
  EXPECT_EQ(list.Decompress().docs(), expected);
}

TEST_F(BoolQueryTest, AndQuery) {
  auto q = BoolQuery::Parse("text & simple", preprocessor);
  const auto list = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 2};
  EXPECT_EQ(list.Decompress().docs(), expected);
}

TEST_F(BoolQueryTest, OrQuery) {
  auto q = BoolQuery::Parse("text | world", preprocessor);
  const auto list = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 1, 2, 3};
  EXPECT_EQ(list.Decompress().docs(), expected);
}

TEST_F(BoolQueryTest, ComplexQuery) {
  auto q = BoolQuery::Parse("hello | simple & text", preprocessor);
  const auto list = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 2, 3};
  EXPECT_EQ(list.Decompress().docs(), expected);
}
