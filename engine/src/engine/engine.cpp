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

std::vector<SearchResult> Engine::SearchBoolean(
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
  return BuildSnippets(GetDocsFromIDs(docs), query.terms());
}

std::vector<SearchResult> Engine::SearchRanked(
    const std::string& query_text, size_t limit /* = SIZE_MAX */) const {
  auto query = query::RankedQuery::Parse(query_text, preprocessor_);
  auto ranked_list = query.Execute(index_);

  if (ranked_list.size() > limit) {
    ranked_list.resize(limit);
  }
  return BuildSnippets(GetDocsFromIDs(ranked_list), query.terms());
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

std::vector<SearchResult> Engine::BuildSnippets(
    const std::vector<storage::Document>& docs,
    const std::vector<std::string>& query_terms,
    size_t window_size /* = 16 */) const {
  std::vector<SearchResult> result;
  result.reserve(docs.size());

  const size_t half = window_size / 2;
  for (const auto& doc : docs) {
    std::vector<uint32_t> positions;
    for (const auto& term : query_terms) {
      const auto& posting_list = index_.GetPostings(term);

      auto itr = posting_list.begin();
      itr.SkipTo(doc.id);
      if (itr.IsEnd() || itr->doc_id != static_cast<uint32_t>(doc.id)) {
        continue;
      }
      for (auto jtr = itr.GetCoordItr(); !jtr.IsEnd(); ++jtr) {
        positions.push_back(*jtr);
      }
    }

    std::string snippet;

    if (positions.empty()) {
      snippet = doc.text.substr(0, 200);
      result.push_back({doc, snippet});
      continue;
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()),
                    positions.end());

    uint32_t best_l = 0;
    uint32_t best_cnt = 0;

    for (size_t l = 0, r = 0; l < positions.size(); ++l) {
      while (r < positions.size() &&
             positions[r] - positions[l] <= window_size) {
        ++r;
      }
      if (r - l > best_cnt) {
        best_cnt = r - l;
        best_l = l;
      }
    }

    size_t center_pos = positions[best_l + best_cnt / 2];

    auto tokens = preprocessor_.Tokenize(doc.text);
    if (tokens.empty()) {
      result.push_back({doc, ""});
      continue;
    }

    size_t left = (center_pos > half) ? center_pos - half : 0;
    size_t right = std::min(tokens.size(), center_pos + half + 1);

    std::ostringstream oss;
    for (size_t i = left; i < right; ++i) {
      auto itr = std::lower_bound(positions.begin(), positions.end(), i);
      if (itr != positions.end() && *itr == i) {
        oss << "\033[1m" << tokens[i] << "\033[0m ";
      } else {
        oss << tokens[i] << ' ';
      }
    }
    result.push_back({doc, oss.str()});
  }

  return result;
}
