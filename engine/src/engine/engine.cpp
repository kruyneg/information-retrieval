#include "engine/engine.h"

#include <iostream>

#include "engine/query/bool_query.h"
#include "engine/query/ranked_query.h"
#include "indexing/posting_list.h"
#include "storage/document.h"
#include "storage/file_index_storage.h"
#include "storage/mongo_doc_storage.h"

Engine CreateEngine() {
  auto doc_storage = std::make_unique<storage::MongoDocStorage>(
      "mongodb://user:password@localhost:27017/", "parser_db");
  auto index_storage = std::make_unique<storage::FileIndexStorage>(
      "/home/kruyneg/Programming/InformationRetrieval/engine/data/index.bin");
  return Engine(std::move(doc_storage), std::move(index_storage));
}

Engine::Engine(std::unique_ptr<storage::DocStorage>&& storage,
               std::unique_ptr<storage::IndexStorage>&& index)
    : doc_storage_(std::move(storage)),
      index_storage_(std::move(index)),
      preprocessor_(linguistics::CreatePreprocessor()) {}

void Engine::BuildIndex() {
  std::cout << "Start Building Index" << std::endl;
  auto cursor = doc_storage_->GetCursor();

  size_t docs_num = 0;

  auto doc_opt = cursor->Next();
  while (doc_opt.has_value()) {
    const auto& doc = doc_opt.value();
    const auto terms = preprocessor_.Preprocess(doc.text);
    index_.AddDocument(doc.id, terms);
    doc_opt = cursor->Next();

    if (++docs_num % 1000 == 0) {
      std::cout << "\rprocessed " << docs_num << " documents..." << std::flush;
    }
  }
  index_.BuildSkips();
  index_storage_->SaveIndex(index_);

  std::cout << "\nIndex saved, processed " << docs_num << " documents"
            << std::endl;
}

void Engine::LoadIndex() { index_ = index_storage_->LoadIndex(); }

std::vector<storage::Document> Engine::SearchBoolean(
    const std::string& query_text, size_t limit /* = SIZE_MAX */) const {
  auto query = query::BoolQuery::Parse(query_text, preprocessor_);
  const auto posting_list = query.Execute(index_);

  std::vector<indexing::DocID> docs;
  docs.reserve(std::min(posting_list.size(), limit));
  for (auto [doc_id, _] : posting_list) {
    docs.push_back(doc_id);
    if (docs.size() == limit) {
      break;
    }
  }
  return GetDocsFromIDs(docs);
}

std::vector<storage::Document> Engine::SearchRanked(
    const std::string& query_text, size_t limit /* = SIZE_MAX */) const {
  auto query = query::RankedQuery::Parse(query_text, preprocessor_);
  auto ranked_list = query.Execute(index_);

  ranked_list.resize(limit);
  return GetDocsFromIDs(ranked_list);
}

std::vector<storage::Document> Engine::GetDocsFromIDs(
    const std::vector<indexing::DocID>& doc_ids) const {
  std::vector<storage::Document> result;
  result.reserve(doc_ids.size());
  for (const auto& doc_id : doc_ids) {
    result.push_back(doc_storage_->GetDocByID(static_cast<int32_t>(doc_id)));
  }
  return result;
}
