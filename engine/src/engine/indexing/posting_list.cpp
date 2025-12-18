#include "engine/indexing/posting_list.h"

#include <algorithm>

namespace indexing {

PostingList::PostingList() = default;

void PostingList::Add(const std::string& doc_id) {
  if (list_.empty() || list_.back() != doc_id) {
    list_.push_back(doc_id);
  }
}

[[nodiscard]] const std::vector<std::string>& PostingList::docs() const {
  return list_;
}

PostingList PostingList::Intersect(const PostingList& other) const {
  PostingList result;

  for (size_t i = 0, j = 0; i < list_.size() && j < other.list_.size();) {
    if (list_[i] == other.list_[j]) {
      result.Add(list_[i]);
      ++i;
      ++j;
    } else if (list_[i] < other.list_[j]) {
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
      result.Add(other.list_[j]);
      ++j;
    } else if (j == other.list_.size()) {
      result.Add(list_[i]);
      ++i;
    } else if (list_[i] == other.list_[j]) {
      result.Add(list_[i]);
      ++i;
      ++j;
    } else if (list_[i] < other.list_[j]) {
      result.Add(list_[i]);
      ++i;
    } else {
      result.Add(other.list_[j]);
      ++j;
    }
  }
  return result;
}

PostingList PostingList::Substract(const PostingList& other) const {
  PostingList result;

  for (size_t i = 0, j = 0; j < other.list_.size();) {
    if (i == list_.size()) {
      result.Add(other.list_[j]);
      ++j;
    } else if (list_[i] == other.list_[j]) {
      ++i;
      ++j;
    } else if (list_[i] < other.list_[j]) {
      ++i;
    } else {
      result.Add(other.list_[j]);
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
PostingList PostingList::operator^(const PostingList& other) const {
  return Substract(other);
}

}  // namespace indexing
