#include "engine/indexing/inverted_index.h"

#include <stdexcept>

#include "engine/indexing/posting_list.h"

namespace indexing {

InvertedIndex::InvertedIndex() = default;

void InvertedIndex::AddDocument(DocID doc_id,
                                const std::vector<std::string>& terms) {
  if (doc_lengths_.size() <= doc_id) {
    doc_lengths_.resize(doc_id + 1, 0);
  }
  doc_lengths_[doc_id] = terms.size();

  utils::HashTable<std::vector<uint32_t>> term_coords;
  for (uint32_t i = 0; i < terms.size(); ++i) {
    term_coords[terms[i]].push_back(i);
  }

  for (auto& [term, coords] : term_coords) {
    index_[std::move(term)].Add(doc_id, std::move(coords));
  }
}

void InvertedIndex::BuildSkips() {
  for (auto& [_, list] : index_) {
    list.BuildSkips();
  }
}

const CompressedPostingList& InvertedIndex::GetPostings(
    const std::string& term) const {
  static CompressedPostingList empty;

  auto itr = index_.find(term);
  if (itr != index_.end()) {
    return itr->second;
  }
  return empty;
}

size_t InvertedIndex::GetDocsCount() const { return doc_lengths_.size(); }

u_int32_t InvertedIndex::GetDocLength(DocID doc_id) const {
  if (doc_lengths_.size() <= doc_id) {
    throw std::runtime_error("InvertedIndex: unknown document");
  }
  return doc_lengths_[doc_id];
}

}  // namespace indexing
