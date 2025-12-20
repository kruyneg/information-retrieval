#include "engine/indexing/inverted_index.h"

#include <stdexcept>

#include "engine/indexing/posting_list.h"

namespace indexing {

InvertedIndex::InvertedIndex() = default;

void InvertedIndex::AddDocument(DocID doc_id,
                                const std::vector<std::string>& terms) {
  doc_lengths_[doc_id] = terms.size();

  std::unordered_map<std::string, uint32_t> term_counts;
  for (const auto& term : terms) {
    term_counts[term]++;
  }

  for (auto& [term, tf] : term_counts) {
    index_[std::move(term)].Add(doc_id, tf);
  }
}

PostingList InvertedIndex::GetPostings(const std::string& term) const {
  auto itr = index_.find(term);
  if (itr != index_.end()) {
    return itr->second.Decompress();
  }
  return {};
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
