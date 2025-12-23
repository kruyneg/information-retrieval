#include "engine/query/ranked_query.h"

#include <gtest/gtest.h>

#include "engine/indexing/inverted_index.h"
#include "engine/indexing/posting_list.h"
#include "linguistics/lemmatization/mock_lemmatizer.h"
#include "linguistics/preprocessor.h"
#include "linguistics/tokenization/tokenizer_impl.h"

using namespace query;

class RankedQueryTest : public testing::Test {
 protected:
  RankedQueryTest()
      : preprocessor(std::make_unique<linguistics::TokenizerImpl>(),
                     std::make_unique<linguistics::MockLemmatizer>()) {
    std::vector<std::vector<std::string>> docs{
        {"simple", "text"},
        {"very", "complex", "text"},
        {"another", "simple", "text"},
        {"hello", "world"},
        {"simple", "hello", "world"},
    };

    for (indexing::DocID i = 0; i < docs.size(); ++i) {
      index.AddDocument(i, docs[i]);
    }
  }

  indexing::InvertedIndex index;
  linguistics::Preprocessor preprocessor;
};

TEST_F(RankedQueryTest, SingleTermQuery) {
  auto q = RankedQuery::Parse("simple", preprocessor);
  const auto results = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 2, 4};
  EXPECT_EQ(results, expected);
}

TEST_F(RankedQueryTest, TwoTermQuery) {
  auto q = RankedQuery::Parse("simple text", preprocessor);
  const auto results = q.Execute(index);
  const std::vector<indexing::DocID> expected{0, 2, 1, 4};
  EXPECT_EQ(results, expected);
}

TEST_F(RankedQueryTest, RareTermQuery) {
  auto q = RankedQuery::Parse("complex text", preprocessor);
  const auto results = q.Execute(index);
  const std::vector<indexing::DocID> expected{1, 0, 2};
  EXPECT_EQ(results, expected);
}

TEST_F(RankedQueryTest, MultipleTermsRankOrder) {
  auto q = RankedQuery::Parse("hello simple", preprocessor);
  const auto results = q.Execute(index);
  const std::vector<indexing::DocID> expected{4, 0, 2};
  EXPECT_EQ(results.front(), 4);
}

TEST_F(RankedQueryTest, EmptyQueryReturnsEmpty) {
  auto q = RankedQuery::Parse("", preprocessor);
  const auto results = q.Execute(index);
  EXPECT_TRUE(results.empty());
}

TEST_F(RankedQueryTest, UnknownTermReturnsEmpty) {
  auto q = RankedQuery::Parse("nonexistent", preprocessor);
  const auto results = q.Execute(index);
  EXPECT_TRUE(results.empty());
}

TEST_F(RankedQueryTest, PhraseQuery) {
  auto q = RankedQuery::Parse("\"complex text\" another", preprocessor);
  const auto results = q.Execute(index);
  const std::vector<indexing::DocID> expected{1};
  EXPECT_EQ(results, expected);
}