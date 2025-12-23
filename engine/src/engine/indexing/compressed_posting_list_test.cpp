#include "engine/indexing/compressed_posting_list.h"

#include <gtest/gtest.h>

using namespace indexing;

TEST(CompressedPostingListTest, ReadItr) {
  CompressedPostingList list;
  const std::vector<std::pair<DocID, std::vector<uint32_t>>> expected{
      {1, {1}}, {2, {1, 2}}, {3, {1}}, {4, {3, 4, 5}}};
  for (const auto& [doc_id, coords] : expected) {
    list.Add(doc_id, coords);
  }

  EXPECT_EQ(list.size(), 4ul);

  size_t i = 0;
  for (auto posting : list) {
    EXPECT_EQ(posting.doc_id, expected[i].first);
    EXPECT_EQ(posting.tf, expected[i++].second.size());
  }

  i = 0;
  for (auto itr = list.begin(); itr != list.end(); ++itr) {
    EXPECT_EQ(itr->doc_id, expected[i].first);
    EXPECT_EQ(itr->tf, expected[i].second.size());

    size_t j = 0;
    for (auto jtr = itr.GetCoordItr(); !jtr.IsEnd(); ++jtr) {
      EXPECT_EQ(*jtr, expected[i].second[j++]);
    }
    ++i;
  }
}

TEST(CompressedPostingListTest, SkipToItr) {
  CompressedPostingList list;
  // skip step == 8
  for (indexing::DocID i = 0; i < 64; ++i) {
    list.Add(i, {0});
  }

  // Without skip table
  {
    auto itr = list.begin();
    EXPECT_EQ(itr->doc_id, 0);
    itr.SkipTo(8);
    EXPECT_EQ(itr->doc_id, 8);
    itr.SkipTo(10);
    EXPECT_EQ(itr->doc_id, 10);
    itr.SkipTo(16);
    EXPECT_EQ(itr->doc_id, 16);
    itr.SkipTo(32);
    EXPECT_EQ(itr->doc_id, 32);
    itr.SkipTo(44);
    EXPECT_EQ(itr->doc_id, 44);
    itr.SkipTo(8);
    EXPECT_EQ(itr->doc_id, 44);
  }

  // With Skip table
  list.BuildSkips();
  {
    auto itr = list.begin();
    EXPECT_EQ(itr->doc_id, 0);
    itr.SkipTo(8);
    EXPECT_EQ(itr->doc_id, 8);
    itr.SkipTo(10);
    EXPECT_EQ(itr->doc_id, 10);
    itr.SkipTo(16);
    EXPECT_EQ(itr->doc_id, 16);
    itr.SkipTo(32);
    EXPECT_EQ(itr->doc_id, 32);
    itr.SkipTo(44);
    EXPECT_EQ(itr->doc_id, 44);
    itr.SkipTo(8);
    EXPECT_EQ(itr->doc_id, 44);
  }
}

TEST(CompressedPostingListTest, Intersect) {
  CompressedPostingList list1;
  list1.Add(1, {0});
  list1.Add(2, {0});
  list1.Add(3, {0});
  CompressedPostingList list2;
  list2.Add(2, {0});
  list2.Add(3, {0});
  list2.Add(4, {0});
  list2.Add(5, {0});

  const auto intersection = list1 & list2;
  const std::vector<indexing::DocID> expected_doc_ids = {2, 3};
  int i = 0;
  for (auto [doc_id, _] : intersection) {
    EXPECT_EQ(doc_id, expected_doc_ids[i++]);
  }
}

TEST(CompressedPostingListTest, Merge) {
  CompressedPostingList list1;
  list1.Add(1, {0});
  list1.Add(2, {0});
  list1.Add(3, {0});
  CompressedPostingList list2;
  list2.Add(2, {0});
  list2.Add(3, {0});
  list2.Add(4, {0});
  list2.Add(5, {0});

  const auto merged = list1 | list2;
  const std::vector<indexing::DocID> expected_doc_ids = {1, 2, 3, 4, 5};
  int i = 0;
  for (auto [doc_id, _] : merged) {
    EXPECT_EQ(doc_id, expected_doc_ids[i++]);
  }
}
