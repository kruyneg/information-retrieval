#pragma once

#include <string>
#include <vector>

namespace indexing {

class PostingList {
 public:
  PostingList();

  void Add(const std::string& doc_id);

  [[nodiscard]] const std::vector<std::string>& docs() const;

  PostingList Intersect(const PostingList& other) const;
  PostingList Merge(const PostingList& other) const;
  PostingList Substract(const PostingList& other) const;

  PostingList operator&(const PostingList& other) const;
  PostingList operator|(const PostingList& other) const;
  PostingList operator^(const PostingList& other) const;

 private:
  std::vector<std::string> list_;
};

}  // namespace indexing
