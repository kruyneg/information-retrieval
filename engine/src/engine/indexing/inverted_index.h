#pragma once

#include <string>
#include <unordered_map>

#include "engine/indexing/posting_list.h"

namespace indexing {

class InvertedIndex {
 public:
  InvertedIndex();

  void AddTerm(const std::string& term, const std::string& doc_id);
  const PostingList& GetPostings(const std::string& term) const;

 private:
  std::unordered_map<std::string, PostingList> index_;
};

}  // namespace indexing
