#include "engine/indexing/compressed_posting_list.h"

#include "engine/indexing/posting_list.h"

namespace indexing {

void CompressedPostingList::Add(DocID doc_id, uint32_t tf) {
  uint32_t gap = doc_id - last_doc_id_;
  last_doc_id_ = doc_id;
  VByteEncode(gap);
  VByteEncode(tf);
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

}  // namespace indexing
