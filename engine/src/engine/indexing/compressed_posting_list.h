#pragma once

#include <cstdint>
#include <vector>

#include "engine/indexing/types.h"

namespace indexing {

class PostingList;

class CompressedPostingList {
 public:
  void Add(DocID doc_id, uint32_t tf);

  PostingList Decompress() const;

 private:
  void VByteEncode(uint32_t value);
  uint32_t VByteDecode(size_t& idx) const;

  std::vector<uint8_t> buffer_;
  uint32_t last_doc_id_ = 0;
};

}  // namespace indexing
