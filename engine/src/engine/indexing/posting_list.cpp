#include "engine/indexing/posting_list.h"

#include <algorithm>

namespace indexing {

PostingList::PostingList() = default;

PostingList::PostingList(std::vector<Posting>&& list)
    : list_(std::move(list)) {}

[[nodiscard]] std::vector<uint32_t> PostingList::docs() const {
  std::vector<uint32_t> result;
  result.reserve(list_.size());
  for (const auto& posting : list_) {
    result.push_back(posting.doc_id);
  }
  return result;
}

[[nodiscard]] const std::vector<Posting>& PostingList::postings() const {
  return list_;
}

PostingList PostingList::Intersect(const PostingList& other) const {
  PostingList result;

  for (size_t i = 0, j = 0; i < list_.size() && j < other.list_.size();) {
    if (list_[i].doc_id == other.list_[j].doc_id) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
      ++j;
    } else if (list_[i].doc_id < other.list_[j].doc_id) {
      ++i;
    } else {
      ++j;
    }
  }
  return result;
}

PostingList PostingList::Merge(const PostingList& other) const {
  PostingList result;

  for (size_t i = 0, j = 0; i < list_.size() || j < other.list_.size();) {
    if (i == list_.size()) {
      result.list_.push_back({other.list_[j].doc_id, 0});
      ++j;
    } else if (j == other.list_.size()) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
    } else if (list_[i].doc_id == other.list_[j].doc_id) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
      ++j;
    } else if (list_[i].doc_id < other.list_[j].doc_id) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
    } else {
      result.list_.push_back({other.list_[j].doc_id, 0});
      ++j;
    }
  }
  return result;
}

PostingList PostingList::Substract(const PostingList& other) const {
  PostingList result;

  for (size_t i = 0, j = 0; i < list_.size();) {
    if (j == other.list_.size()) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
    } else if (list_[i].doc_id == other.list_[j].doc_id) {
      ++i;
      ++j;
    } else if (list_[i].doc_id < other.list_[j].doc_id) {
      result.list_.push_back({list_[i].doc_id, 0});
      ++i;
    } else {
      ++j;
    }
  }
  return result;
}

PostingList PostingList::operator&(const PostingList& other) const {
  return Intersect(other);
}
PostingList PostingList::operator|(const PostingList& other) const {
  return Merge(other);
}
PostingList PostingList::operator-(const PostingList& other) const {
  return Substract(other);
}

}  // namespace indexing
