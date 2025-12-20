#pragma once

#include <string>
#include <unordered_map>

#include "engine/indexing/compressed_posting_list.h"

namespace storage {
class FileIndexStorage;
}

namespace indexing {

class InvertedIndex {
 public:
  InvertedIndex();

  void AddDocument(DocID doc_id, const std::vector<std::string>& terms);
  PostingList GetPostings(const std::string& term) const;
  size_t GetDocsCount() const;
  uint32_t GetDocLength(DocID doc_id) const;

 private:
  friend class storage::FileIndexStorage;

  std::unordered_map<std::string, CompressedPostingList> index_;
  std::unordered_map<DocID, uint32_t> doc_lengths_;
};

}  // namespace indexing
