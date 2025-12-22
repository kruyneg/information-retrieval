#pragma once

#include <string>
#include <vector>

#include "engine/indexing/types.h"

namespace indexing {

class CompressedPostingList;

class PostingList {
 public:
  PostingList();
  explicit PostingList(std::vector<Posting>&& list);

  [[nodiscard]] std::vector<uint32_t> docs() const;
  [[nodiscard]] const std::vector<Posting>& postings() const;

  PostingList Intersect(const PostingList& other) const;
  PostingList Merge(const PostingList& other) const;
  PostingList Substract(const PostingList& other) const;

  PostingList operator&(const PostingList& other) const;
  PostingList operator|(const PostingList& other) const;
  PostingList operator-(const PostingList& other) const;

 private:
  friend class CompressedPostingList;

  std::vector<Posting> list_;
};

}  // namespace indexing
