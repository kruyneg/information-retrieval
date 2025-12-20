#pragma once

namespace indexing {
class InvertedIndex;
}

namespace storage {

class IndexStorage {
 public:
  virtual void SaveIndex(const indexing::InvertedIndex&) = 0;
  virtual indexing::InvertedIndex LoadIndex() = 0;
};

}  // namespace storage
