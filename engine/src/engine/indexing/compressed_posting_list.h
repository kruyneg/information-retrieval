#pragma once

#include <iterator>
#include <vector>

#include "engine/indexing/types.h"

namespace storage {
class FileIndexStorage;
}

namespace indexing {

class PostingList;

class CompressedPostingList {
 public:
  class Iterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Posting;
    using difference_type = std::ptrdiff_t;
    using pointer = const Posting*;
    using reference = const Posting&;

    explicit Iterator(const CompressedPostingList* list = nullptr);

    reference operator*() const;
    pointer operator->() const;

    Iterator& operator++();
    Iterator operator++(int);

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    void SkipTo(DocID target);

   private:
    bool IsEnd() const;

    const CompressedPostingList* list_ = nullptr;
    size_t byte_idx_ = 0;
    size_t skip_idx_ = 0;
    Posting current_;
  };

  void Add(DocID doc_id, uint32_t tf);

  PostingList Decompress() const;

  void BuildSkips();

  size_t size() const;

  CompressedPostingList Intersect(const CompressedPostingList& other) const;
  CompressedPostingList Merge(const CompressedPostingList& other) const;

  CompressedPostingList operator&(const CompressedPostingList& other) const;
  CompressedPostingList operator|(const CompressedPostingList& other) const;

  Iterator begin() const;
  Iterator end() const;

 private:
  friend class storage::FileIndexStorage;

  struct Skip {
    DocID doc_id;
    size_t offset;
  };

  void VByteEncode(uint32_t value);
  uint32_t VByteDecode(size_t& idx) const;

  std::vector<Skip> skips_;
  size_t skip_step_;

  std::vector<uint8_t> buffer_;
  size_t size_ = 0;
  uint32_t last_doc_id_ = 0;
};

}  // namespace indexing
