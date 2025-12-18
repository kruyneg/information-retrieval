#include "engine/indexing/inverted_index.h"

#include <stdexcept>

namespace indexing {

InvertedIndex::InvertedIndex() = default;

void InvertedIndex::AddDocument(const std::string& doc_id,
                                const std::vector<std::string>& terms) {
  doc_lengths_[doc_id] = terms.size();
  for (const auto& term : terms) {
    index_[term].Add(doc_id);
  }
}

const PostingList& InvertedIndex::GetPostings(const std::string& term) const {
  static PostingList empty;

  auto itr = index_.find(term);
  if (itr != index_.end()) {
    return itr->second;
  }
  return empty;
}

size_t InvertedIndex::GetDocsCount() const { return doc_lengths_.size(); }

u_int32_t InvertedIndex::GetDocLength(const std::string& doc_id) const {
  auto itr = doc_lengths_.find(doc_id);
  if (itr == doc_lengths_.end()) {
    throw std::runtime_error("InvertedIndex: unknown document");
  }
  return itr->second;
}

}  // namespace indexing
