#pragma once

#include <string>

#include "engine/indexing/compressed_posting_list.h"
#include "utils/hash_table.h"

namespace storage {
class FileIndexStorage;
}

namespace indexing {

class InvertedIndex {
 public:
  InvertedIndex();

  void AddDocument(DocID doc_id, const std::vector<std::string>& terms);
  void BuildSkips();

  const CompressedPostingList& GetPostings(const std::string& term) const;
  size_t GetDocsCount() const;
  uint32_t GetDocLength(DocID doc_id) const;

 private:
  friend class storage::FileIndexStorage;

  utils::HashTable<CompressedPostingList> index_;
  std::vector<uint32_t> doc_lengths_;
};

}  // namespace indexing
