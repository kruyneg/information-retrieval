#include "engine/indexing/compressed_posting_list.h"

#include <cmath>

#include "engine/indexing/posting_list.h"

namespace {

void VByteEncode(uint32_t& value, std::vector<uint8_t>& buffer) {
  while (value >= 128) {
    buffer.push_back(value & 0x7f);
    value >>= 7;
  }
  buffer.push_back(value | 0x80);
}

uint32_t VByteDecode(size_t& idx, const std::vector<uint8_t>& buffer) {
  uint32_t value = 0;
  int shift = 0;
  while (idx < buffer.size()) {
    uint8_t byte = buffer[idx++];
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

}  // namespace

namespace indexing {

using DocIterator = indexing::CompressedPostingList::DocIterator;
using CoordIterator = indexing::CompressedPostingList::CoordIterator;

void CompressedPostingList::Add(DocID doc_id,
                                const std::vector<uint32_t>& coords) {
  uint32_t doc_gap = doc_id - last_doc_id_;
  last_doc_id_ = doc_id;
  VByteEncodeDoc(doc_gap);
  VByteEncodeDoc(static_cast<uint32_t>(coord_buffer_.size()));

  const auto tf = static_cast<uint32_t>(coords.size());
  VByteEncodeCoord(tf);
  uint32_t last_coord = 0;
  for (const auto coord : coords) {
    VByteEncodeCoord(coord - last_coord);
    last_coord = coord;
  }

  ++size_;
}

PostingList CompressedPostingList::Decompress() const {
  PostingList result;
  DocID doc_id = 0;
  for (size_t i = 0; i < doc_buffer_.size();) {
    doc_id += VByteDecodeDoc(i);
    size_t j = VByteDecodeDoc(i);
    const auto tf = VByteDecodeCoord(j);
    result.list_.push_back(Posting{doc_id, tf});
  }
  return result;
}

void CompressedPostingList::VByteEncodeDoc(uint32_t value) {
  VByteEncode(value, doc_buffer_);
}

uint32_t CompressedPostingList::VByteDecodeDoc(size_t& idx) const {
  return VByteDecode(idx, doc_buffer_);
}

void CompressedPostingList::VByteEncodeCoord(uint32_t value) {
  VByteEncode(value, coord_buffer_);
}

uint32_t CompressedPostingList::VByteDecodeCoord(size_t& idx) const {
  return VByteDecode(idx, coord_buffer_);
}

void CompressedPostingList::BuildSkips() {
  skip_step_ = static_cast<size_t>(std::sqrt(size_));
  skip_step_ = std::max(skip_step_, 4ul);
  DocID doc_id = 0;
  for (size_t idx = 0, i = 0; idx < size_; ++idx) {
    size_t prev_i = i;
    doc_id += VByteDecodeDoc(i);
    VByteDecodeDoc(i);
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
      result.Add(itr->doc_id, {});
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
      result.Add(jtr->doc_id, {});
      ++jtr;
    } else if (jtr == other.end()) {
      result.Add(itr->doc_id, {});
      ++itr;
    } else if (itr->doc_id == jtr->doc_id) {
      result.Add(itr->doc_id, {});
      ++itr;
      ++jtr;
    } else if (itr->doc_id < jtr->doc_id) {
      result.Add(itr->doc_id, {});
      ++itr;
    } else {
      result.Add(jtr->doc_id, {});
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

DocIterator CompressedPostingList::begin() const { return DocIterator(this); }

DocIterator CompressedPostingList::end() const { return DocIterator(); }

DocIterator::DocIterator(const CompressedPostingList* list /* = nullptr */)
    : list_(list) {
  if (list_ && list->size() > 0) {
    current_.doc_id = list_->VByteDecodeDoc(byte_idx_);
    coord_idx_ = list_->VByteDecodeDoc(byte_idx_);
    current_.tf = list_->VByteDecodeCoord(coord_idx_);
  } else {
    list_ = nullptr;
  }
}

DocIterator::reference DocIterator::operator*() const { return current_; }

DocIterator::pointer DocIterator::operator->() const { return &current_; }

CoordIterator DocIterator::GetCoordItr() const {
  return CoordIterator(list_, current_.tf, coord_idx_);
}

DocIterator& DocIterator::operator++() {
  if (byte_idx_ >= list_->doc_buffer_.size()) {
    list_ = nullptr;
    current_ = {0, 0};
    return *this;
  }
  if (skip_idx_ < list_->skips_.size() &&
      current_.doc_id == list_->skips_[skip_idx_].doc_id) {
    ++skip_idx_;
  }
  current_.doc_id += list_->VByteDecodeDoc(byte_idx_);
  coord_idx_ = list_->VByteDecodeDoc(byte_idx_);
  current_.tf = list_->VByteDecodeCoord(coord_idx_);
  return *this;
}

DocIterator DocIterator::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

bool DocIterator::operator==(const DocIterator& other) const {
  if (IsEnd() && other.IsEnd()) {
    return true;
  }
  return list_ == other.list_ && byte_idx_ == other.byte_idx_;
}

bool DocIterator::operator!=(const DocIterator& other) const {
  return !(*this == other);
}

void DocIterator::SkipTo(DocID target) {
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
  list_->VByteDecodeDoc(byte_idx_);
  coord_idx_ = list_->VByteDecodeDoc(byte_idx_);
  current_.tf = list_->VByteDecodeCoord(coord_idx_);

  while (!IsEnd() && current_.doc_id < target) {
    ++(*this);
  }
}

bool DocIterator::IsEnd() const { return list_ == nullptr; }

CoordIterator::CoordIterator(const CompressedPostingList* list, uint32_t size,
                             size_t idx)
    : list_(list), tf_(size), byte_idx_(idx) {
  current_ = list_->VByteDecodeCoord(byte_idx_);
}

CoordIterator::reference CoordIterator::operator*() const { return current_; }

CoordIterator& CoordIterator::operator++() {
  ++pos_idx_;
  if (pos_idx_ >= tf_) {
    list_ = nullptr;
    return *this;
  }

  current_ += list_->VByteDecodeCoord(byte_idx_);
  return *this;
}

CoordIterator CoordIterator::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

bool CoordIterator::IsEnd() const { return list_ == nullptr; }

}  // namespace indexing
