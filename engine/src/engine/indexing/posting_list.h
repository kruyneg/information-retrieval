#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace indexing {

struct Posting {
  std::string doc_id;
  uint32_t tf;
};

class PostingList {
 public:
  PostingList();

  void Add(const std::string& doc_id);

  [[nodiscard]] std::vector<std::string> docs() const;
  [[nodiscard]] const std::vector<Posting>& postings() const;

  PostingList Intersect(const PostingList& other) const;
  PostingList Merge(const PostingList& other) const;
  PostingList Substract(const PostingList& other) const;

  PostingList operator&(const PostingList& other) const;
  PostingList operator|(const PostingList& other) const;
  PostingList operator-(const PostingList& other) const;

 private:
  std::vector<Posting> list_;
};

}  // namespace indexing
