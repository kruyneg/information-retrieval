#include "engine/indexing/inverted_index.h"

namespace indexing {

InvertedIndex::InvertedIndex() = default;

void InvertedIndex::AddTerm(const std::string& term,
                            const std::string& doc_id) {
  index_[term].Add(doc_id);
}

const PostingList& InvertedIndex::GetPostings(const std::string& term) const {
  static PostingList empty;

  auto itr = index_.find(term);
  if (itr != index_.end()) {
    return itr->second;
  }
  return empty;
}

}  // namespace indexing
