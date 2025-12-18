#include "engine/engine.h"

#include "query/query.h"
#include "storage/document.h"
#include "storage/mongo_storage.h"

Engine CreateEngine() {
  auto storage = std::make_unique<storage::MongoStorage>(
      "mongodb://user:password@localhost:27017/", "parser_db");
  return Engine(std::move(storage));
}

Engine::Engine(std::unique_ptr<storage::Storage>&& storage)
    : storage_(std::move(storage)),
      preprocessor_(linguistics::CreatePreprocessor()) {}

void Engine::BuildIndex() {
  auto cursor = storage_->GetCursor();

  auto doc_opt = cursor->Next();
  while (doc_opt.has_value()) {
    const auto& doc = doc_opt.value();
    const auto terms = preprocessor_.Preprocess(doc.text);
    for (const auto& term : terms) {
      index_.AddTerm(term, doc.id);
    }
    doc_opt = cursor->Next();
  }
}

std::vector<storage::Document> Engine::Search(
    const std::string& query_text) const {
  auto query = query::Query::Parse(query_text, preprocessor_);
  const auto posting_list = query.Execute(index_);

  std::vector<storage::Document> result;
  for (const auto& doc_id : posting_list.docs()) {
    result.push_back(storage_->GetDocByID(doc_id));
  }

  return result;
}
