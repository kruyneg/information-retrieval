#include "engine/indexing/inverted_index.h"

#include <stdexcept>

#include "engine/indexing/posting_list.h"

namespace indexing {

InvertedIndex::InvertedIndex() = default;

void InvertedIndex::AddDocument(DocID doc_id,
                                const std::vector<std::string>& terms) {
  doc_lengths_[doc_id] = terms.size();

  std::unordered_map<std::string, std::vector<uint32_t>> term_coords;
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
  auto itr = doc_lengths_.find(doc_id);
  if (itr == doc_lengths_.end()) {
    throw std::runtime_error("InvertedIndex: unknown document");
  }
  return itr->second;
}

}  // namespace indexing
