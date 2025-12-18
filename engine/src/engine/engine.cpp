#include "engine/engine.h"

#include "engine/query/bool_query.h"
#include "engine/query/ranked_query.h"
#include "storage/document.h"
#include "storage/mongo_doc_storage.h"

Engine CreateEngine() {
  auto storage = std::make_unique<storage::MongoDocStorage>(
      "mongodb://user:password@localhost:27017/", "parser_db");
  return Engine(std::move(storage));
}

Engine::Engine(std::unique_ptr<storage::DocStorage>&& storage)
    : storage_(std::move(storage)),
      preprocessor_(linguistics::CreatePreprocessor()) {}

void Engine::BuildIndex() {
  auto cursor = storage_->GetCursor();

  auto doc_opt = cursor->Next();
  while (doc_opt.has_value()) {
    const auto& doc = doc_opt.value();
    const auto terms = preprocessor_.Preprocess(doc.text);
    index_.AddDocument(doc.id, terms);
    doc_opt = cursor->Next();
  }
}

std::vector<storage::Document> Engine::SearchBoolean(
    const std::string& query_text) const {
  auto query = query::BoolQuery::Parse(query_text, preprocessor_);
  const auto posting_list = query.Execute(index_);

  return GetDocsFromIDs(posting_list.docs());
}

std::vector<storage::Document> Engine::SearchRanked(
    const std::string& query_text) const {
  auto query = query::RankedQuery::Parse(query_text, preprocessor_);
  const auto ranked_list = query.Execute(index_);

  return GetDocsFromIDs(ranked_list);
}

std::vector<storage::Document> Engine::GetDocsFromIDs(
    const std::vector<std::string>& doc_ids) const {
  std::vector<storage::Document> result;
  result.reserve(doc_ids.size());
  for (const auto& doc_id : doc_ids) {
    result.push_back(storage_->GetDocByID(doc_id));
  }
  return result;
}
