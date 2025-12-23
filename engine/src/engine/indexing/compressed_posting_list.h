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
  class CoordIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = uint32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const uint32_t*;
    using reference = const uint32_t&;

    CoordIterator(const CompressedPostingList* list, uint32_t size, size_t idx);

    reference operator*() const;

    CoordIterator& operator++();
    CoordIterator operator++(int);

    bool IsEnd() const;

   private:
    const CompressedPostingList* list_ = nullptr;
    uint32_t tf_ = 0;
    size_t byte_idx_ = 0;
    uint32_t pos_idx_ = 0;
    uint32_t current_;
  };

  class DocIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Posting;
    using difference_type = std::ptrdiff_t;
    using pointer = const Posting*;
    using reference = const Posting&;

    explicit DocIterator(const CompressedPostingList* list = nullptr);

    reference operator*() const;
    pointer operator->() const;

    CoordIterator GetCoordItr() const;

    DocIterator& operator++();
    DocIterator operator++(int);

    bool operator==(const DocIterator& other) const;
    bool operator!=(const DocIterator& other) const;

    void SkipTo(DocID target);

    bool IsEnd() const;

   private:
    const CompressedPostingList* list_ = nullptr;
    size_t byte_idx_ = 0;
    size_t coord_idx_ = 0;
    size_t skip_idx_ = 0;
    Posting current_;
  };

  void Add(DocID doc_id, const std::vector<uint32_t>& coords);

  PostingList Decompress() const;

  void BuildSkips();

  size_t size() const;

  CompressedPostingList Intersect(const CompressedPostingList& other) const;
  CompressedPostingList Merge(const CompressedPostingList& other) const;

  CompressedPostingList operator&(const CompressedPostingList& other) const;
  CompressedPostingList operator|(const CompressedPostingList& other) const;

  DocIterator begin() const;
  DocIterator end() const;

 private:
  friend class storage::FileIndexStorage;

  struct Skip {
    DocID doc_id;
    size_t offset;
  };

  void VByteEncodeDoc(uint32_t value);
  uint32_t VByteDecodeDoc(size_t& idx) const;
  void VByteEncodeCoord(uint32_t value);
  uint32_t VByteDecodeCoord(size_t& idx) const;

  std::vector<Skip> skips_;
  size_t skip_step_;

  std::vector<uint8_t> doc_buffer_;
  std::vector<uint8_t> coord_buffer_;

  size_t size_ = 0;
  uint32_t last_doc_id_ = 0;
};

}  // namespace indexing
