#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "engine/indexing/posting_list.h"

namespace indexing {

class InvertedIndex {
 public:
  InvertedIndex();

  void AddDocument(const std::string& doc_id,
                   const std::vector<std::string>& terms);
  const PostingList& GetPostings(const std::string& term) const;
  size_t GetDocsCount() const;
  uint32_t GetDocLength(const std::string& doc_id) const;

 private:
  std::unordered_map<std::string, PostingList> index_;
  std::unordered_map<std::string, uint32_t> doc_lengths_;
};

}  // namespace indexing
