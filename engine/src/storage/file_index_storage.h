#pragma once

#include <filesystem>
#include <fstream>

#include "storage/index_storage.h"

namespace storage {

class FileIndexStorage : public IndexStorage {
 public:
  explicit FileIndexStorage(const std::filesystem::path& filename);

  void SaveIndex(const indexing::InvertedIndex&) override;
  indexing::InvertedIndex LoadIndex() override;

 private:
  std::fstream file_;
};

}  // namespace storage
