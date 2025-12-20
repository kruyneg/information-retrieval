#pragma once

#include "engine/indexing/inverted_index.h"
#include "linguistics/preprocessor.h"
#include "storage/doc_storage.h"
#include "storage/index_storage.h"

class Engine {
 public:
  Engine(std::unique_ptr<storage::DocStorage>&& storage,
         std::unique_ptr<storage::IndexStorage>&& index);

  void BuildIndex();
  void LoadIndex();

  std::vector<storage::Document> SearchBoolean(const std::string& query,
                                               size_t limit = SIZE_MAX) const;

  std::vector<storage::Document> SearchRanked(const std::string& query,
                                              size_t limit = SIZE_MAX) const;

 private:
  std::vector<storage::Document> GetDocsFromIDs(
      const std::vector<indexing::DocID>& doc_ids) const;

  std::unique_ptr<storage::DocStorage> doc_storage_;
  std::unique_ptr<storage::IndexStorage> index_storage_;

  indexing::InvertedIndex index_;
  linguistics::Preprocessor preprocessor_;
};

Engine CreateEngine();
