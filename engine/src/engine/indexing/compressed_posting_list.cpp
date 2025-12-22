#include "engine/indexing/compressed_posting_list.h"

#include <cmath>

#include "engine/indexing/posting_list.h"

namespace indexing {

using ListIterator = CompressedPostingList::Iterator;

void CompressedPostingList::Add(DocID doc_id, uint32_t tf) {
  uint32_t gap = doc_id - last_doc_id_;
  last_doc_id_ = doc_id;
  VByteEncode(gap);
  VByteEncode(tf);
  ++size_;
}

void CompressedPostingList::VByteEncode(uint32_t value) {
  while (value >= 128) {
    buffer_.push_back(value & 0x7f);
    value >>= 7;
  }
  buffer_.push_back(value | 0x80);
}

PostingList CompressedPostingList::Decompress() const {
  PostingList result;
  DocID doc_id = 0;
  for (size_t i = 0; i < buffer_.size();) {
    doc_id += VByteDecode(i);
    auto tf = VByteDecode(i);
    result.list_.push_back(Posting{doc_id, tf});
  }
  return result;
}

uint32_t CompressedPostingList::VByteDecode(size_t& idx) const {
  uint32_t value = 0;
  int shift = 0;
  while (idx < buffer_.size()) {
    uint8_t byte = buffer_[idx++];
    if (byte & 0x80) {
      value |= (byte & 0x7f) << shift;
      break;
    } else {
      value |= byte << shift;
      shift += 7;
    }
  }
  return value;
}

void CompressedPostingList::BuildSkips() {
  skip_step_ = static_cast<size_t>(std::sqrt(size_));
  skip_step_ = std::max(skip_step_, 4ul);
  DocID doc_id = 0;
  for (size_t idx = 0, i = 0; idx < size_; ++idx) {
    size_t prev_i = i;
    doc_id += VByteDecode(i);
    VByteDecode(i);
    if (idx % skip_step_ == 0) {
      skips_.push_back(Skip{doc_id, prev_i});
    }
  }
}

CompressedPostingList CompressedPostingList::Intersect(
    const CompressedPostingList& other) const {
  CompressedPostingList result;

  auto itr = begin();
  auto jtr = other.begin();
  while (itr != end() && jtr != other.end()) {
    if (itr->doc_id == jtr->doc_id) {
      result.Add(itr->doc_id, 0);
      ++itr;
      ++jtr;
    } else if (itr->doc_id < jtr->doc_id) {
      itr.SkipTo(jtr->doc_id);
    } else {
      jtr.SkipTo(itr->doc_id);
    }
  }

  result.BuildSkips();
  return result;
}

CompressedPostingList CompressedPostingList::Merge(
    const CompressedPostingList& other) const {
  CompressedPostingList result;

  auto itr = begin();
  auto jtr = other.begin();
  while (itr != end() || jtr != other.end()) {
    if (itr == end()) {
      result.Add(jtr->doc_id, 0);
      ++jtr;
    } else if (jtr == other.end()) {
      result.Add(itr->doc_id, 0);
      ++itr;
    } else if (itr->doc_id == jtr->doc_id) {
      result.Add(itr->doc_id, 0);
      ++itr;
      ++jtr;
    } else if (itr->doc_id < jtr->doc_id) {
      result.Add(itr->doc_id, 0);
      ++itr;
    } else {
      result.Add(jtr->doc_id, 0);
      ++jtr;
    }
  }

  result.BuildSkips();
  return result;
}

CompressedPostingList CompressedPostingList::operator&(
    const CompressedPostingList& other) const {
  return Intersect(other);
}

CompressedPostingList CompressedPostingList::operator|(
    const CompressedPostingList& other) const {
  return Merge(other);
}

size_t CompressedPostingList::size() const { return size_; }

ListIterator CompressedPostingList::begin() const { return Iterator(this); }

ListIterator CompressedPostingList::end() const { return Iterator(); }

ListIterator::Iterator(const CompressedPostingList* list /* = nullptr */)
    : list_(list) {
  if (list_ && list->size() > 0) {
    current_.doc_id = list_->VByteDecode(byte_idx_);
    current_.tf = list_->VByteDecode(byte_idx_);
  } else {
    list_ = nullptr;
  }
}

ListIterator::reference ListIterator::operator*() const { return current_; }

ListIterator::pointer ListIterator::operator->() const { return &current_; }

ListIterator& ListIterator::operator++() {
  if (byte_idx_ >= list_->buffer_.size()) {
    list_ = nullptr;
    current_ = {0, 0};
    return *this;
  }
  if (skip_idx_ < list_->skips_.size() &&
      current_.doc_id == list_->skips_[skip_idx_].doc_id) {
    ++skip_idx_;
  }
  current_.doc_id += list_->VByteDecode(byte_idx_);
  current_.tf = list_->VByteDecode(byte_idx_);
  return *this;
}

ListIterator ListIterator::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

bool ListIterator::operator==(const ListIterator& other) const {
  if (IsEnd() && other.IsEnd()) {
    return true;
  }
  return list_ == other.list_ && byte_idx_ == other.byte_idx_;
}

bool ListIterator::operator!=(const ListIterator& other) const {
  return !(*this == other);
}

void ListIterator::SkipTo(DocID target) {
  if ((skip_idx_ < list_->skips_.size() &&
       list_->skips_[skip_idx_].doc_id > target) ||
      (skip_idx_ >= list_->skips_.size())) {
    while (!IsEnd() && current_.doc_id < target) {
      ++(*this);
    }
    return;
  }

  while (skip_idx_ < list_->skips_.size() &&
         list_->skips_[skip_idx_].doc_id <= target) {
    skip_idx_++;
  }

  if (skip_idx_ == 0) {
    return;
  }

  skip_idx_--;
  const auto skip = list_->skips_[skip_idx_];

  byte_idx_ = skip.offset;
  current_.doc_id = skip.doc_id;
  list_->VByteDecode(byte_idx_);
  current_.tf = list_->VByteDecode(byte_idx_);

  while (!IsEnd() && current_.doc_id < target) {
    ++(*this);
  }
}

bool ListIterator::IsEnd() const { return list_ == nullptr; }

}  // namespace indexing
