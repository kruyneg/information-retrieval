#include "engine/indexing/posting_list.h"

#include <gtest/gtest.h>

#include "engine/indexing/compressed_posting_list.h"

using namespace indexing;

TEST(PostingListTest, Intersect) {
  PostingList list1({{1, 1}, {2, 1}, {3, 1}});

  std::vector<DocID> expected_doc_ids{1, 2, 3};
  EXPECT_EQ(list1.docs(), expected_doc_ids);

  PostingList list2({{2, 1}, {3, 1}, {4, 1}, {5, 1}});
  const auto intersection = list1 & list2;
  expected_doc_ids = {2, 3};
  EXPECT_EQ(intersection.docs(), expected_doc_ids);
}

TEST(PostingListTest, Merge) {
  PostingList list1({{1, 1}, {2, 1}, {3, 1}});
  PostingList list2({{2, 1}, {3, 1}, {4, 1}, {5, 1}});
  const auto merged = list1 | list2;

  std::vector<DocID> expected_doc_ids{1, 2, 3, 4, 5};
  EXPECT_EQ(merged.docs(), expected_doc_ids);
}

TEST(PostingListTest, Substract) {
  PostingList list1({{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}});
  PostingList list2({{2, 1}, {3, 1}});
  const auto res = list1 - list2;

  std::vector<DocID> expected_doc_ids{1, 4, 5};
  EXPECT_EQ(res.docs(), expected_doc_ids);
}

TEST(PostingListTest, Decompression) {
  CompressedPostingList compressed;
  compressed.Add(1, {1, 2});
  compressed.Add(3, {1});
  compressed.Add(4, {1});
  compressed.Add(5, {2, 4, 6, 8, 10});

  const auto list = compressed.Decompress();
  const std::vector<Posting> expected{{1, 2}, {3, 1}, {4, 1}, {5, 5}};
  EXPECT_EQ(list.postings(), expected);

  const std::vector<DocID> expected_doc_ids{1, 3, 4, 5};
  EXPECT_EQ(list.docs(), expected_doc_ids);
}
