#pragma once

#include "engine/indexing/inverted_index.h"
#include "linguistics/preprocessor.h"
#include "storage/doc_storage.h"

class Engine {
 public:
  explicit Engine(std::unique_ptr<storage::DocStorage>&& storage);

  void BuildIndex();

  std::vector<storage::Document> SearchBoolean(const std::string& query) const;

  std::vector<storage::Document> SearchRanked(const std::string& query) const;

 private:
  std::vector<storage::Document> GetDocsFromIDs(
      const std::vector<std::string>& doc_ids) const;

  std::unique_ptr<storage::DocStorage> storage_;
  indexing::InvertedIndex index_;
  linguistics::Preprocessor preprocessor_;
};

Engine CreateEngine();
