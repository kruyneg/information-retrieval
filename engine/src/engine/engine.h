#pragma once

#include "engine/indexing/inverted_index.h"
#include "linguistics/preprocessor.h"
#include "storage/storage.h"

class Engine {
 public:
  explicit Engine(std::unique_ptr<storage::Storage>&& storage);

  void BuildIndex();

  std::vector<storage::Document> Search(const std::string& query) const;

 private:
  std::unique_ptr<storage::Storage> storage_;
  indexing::InvertedIndex index_;
  linguistics::Preprocessor preprocessor_;
};

Engine CreateEngine();
