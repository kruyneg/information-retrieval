#include "engine/query/ranked_query.h"

#include <algorithm>
#include <cmath>

#include "engine/indexing/inverted_index.h"
#include "engine/indexing/posting_list.h"
#include "linguistics/preprocessor.h"

namespace query {

RankedQuery RankedQuery::Parse(const std::string& query,
                               const linguistics::Preprocessor& preprocessor) {
  const auto terms = preprocessor.Preprocess(query);
  return RankedQuery(std::move(terms));
}

RankedQuery::RankedQuery(const std::vector<std::string>& terms) {
  for (const auto& term : terms) {
    ++query_tf_[term];
  }
}

std::vector<indexing::DocID> RankedQuery::Execute(
    const indexing::InvertedIndex& index) {
  std::unordered_map<indexing::DocID, double> doc_scores;
  std::unordered_map<indexing::DocID, double> doc_norms;
  double query_norm = 0.0;

  for (const auto& [term, tf] : query_tf_) {
    const auto& posting_list = index.GetPostings(term);
    const double idf = std::log((1.0 + index.GetDocsCount()) /
                                (1.0 + posting_list.postings().size())) +
                       1.0;
    const double log_tf = 1.0 + std::log(tf);
    const double query_weight = log_tf * idf;
    query_norm += query_weight * query_weight;

    for (const auto& posting : posting_list.postings()) {
      const double doc_log_tf = 1.0 + std::log(posting.tf);
      const double doc_weight = doc_log_tf * idf;
      doc_scores[posting.doc_id] += doc_weight * query_weight;
      doc_norms[posting.doc_id] += doc_weight * doc_weight;
    }
  }

  query_norm = std::sqrt(query_norm);

  std::vector<std::pair<indexing::DocID, double>> ranked;
  ranked.reserve(doc_scores.size());

  for (const auto& [doc_id, score] : doc_scores) {
    const double doc_norm = std::sqrt(doc_norms[doc_id]);
    if (doc_norm == 0.0 || query_norm == 0.0) {
      continue;
    }
    const double normalized_score = score / (doc_norm * query_norm);
    ranked.emplace_back(doc_id, normalized_score);
  }

  std::sort(ranked.begin(), ranked.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

  std::vector<indexing::DocID> result;
  result.reserve(ranked.size());
  for (auto& [doc_id, _] : ranked) {
    result.push_back(std::move(doc_id));
  }
  return result;
}

}  // namespace query
