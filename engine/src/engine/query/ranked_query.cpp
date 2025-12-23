#include "engine/query/ranked_query.h"

#include <algorithm>
#include <cmath>

#include "engine/indexing/compressed_posting_list.h"
#include "engine/indexing/inverted_index.h"
#include "engine/query/phrase_query.h"
#include "linguistics/preprocessor.h"

namespace query {

RankedQuery RankedQuery::Parse(const std::string& query,
                               const linguistics::Preprocessor& preprocessor) {
  std::vector<std::string> raw_phrases;
  bool in_phrase = false;
  for (auto c : query) {
    if (c == '"') {
      if (!in_phrase) {
        raw_phrases.emplace_back();
      }
      in_phrase = !in_phrase;
    }
    if (in_phrase) {
      raw_phrases.back().push_back(c);
    }
  }
  std::vector<std::vector<std::string>> phrases;
  for (auto& phrase : raw_phrases) {
    phrases.emplace_back(preprocessor.Preprocess(phrase));
  }
  const auto terms = preprocessor.Preprocess(query);
  return RankedQuery(std::move(terms), std::move(phrases));
}

RankedQuery::RankedQuery(const std::vector<std::string>& terms,
                         const std::vector<std::vector<std::string>>& phrases)
    : phrases_(phrases) {
  for (const auto& term : terms) {
    ++query_tf_[term];
  }
}

std::vector<indexing::DocID> RankedQuery::Execute(
    const indexing::InvertedIndex& index) {
  std::unordered_map<indexing::DocID, double> doc_scores;
  double query_norm = 0.0;

  indexing::CompressedPostingList phrases_list;
  for (const auto& phrase : phrases_) {
    auto list = query::ExecutePhraseQuery(phrase, index);
    if (phrases_list.size() == 0) {
      phrases_list = std::move(list);
    } else {
      phrases_list = phrases_list & list;
    }
  }

  for (const auto& [term, tf] : query_tf_) {
    auto posting_list = index.GetPostings(std::string(term));
    if (!phrases_.empty()) {
      posting_list = posting_list & phrases_list;
    }
    const double idf =
        std::log((1.0 + index.GetDocsCount()) / (1.0 + posting_list.size())) +
        1.0;
    const double query_tf_weight = 1.0 + std::log(tf);
    const double query_weight = query_tf_weight * idf;
    query_norm += query_weight * query_weight;

    for (const auto& posting : posting_list) {
      const double doc_tf_weight = 1.0 + std::log(posting.tf);
      const double doc_weight = doc_tf_weight * idf;

      doc_scores[posting.doc_id] += doc_weight * query_weight;
    }
  }

  query_norm = std::sqrt(query_norm);
  if (query_norm == 0.0) {
    return {};
  }

  std::vector<std::pair<indexing::DocID, double>> ranked;
  ranked.reserve(doc_scores.size());

  for (const auto& [doc_id, score] : doc_scores) {
    const double doc_length = index.GetDocLength(doc_id);
    if (doc_length == 0.0) {
      continue;
    }
    const double doc_norm = std::sqrt(doc_length);
    const double normalized_score = score / (doc_norm * query_norm);

    ranked.emplace_back(doc_id, normalized_score);
  }

  std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
    if (a.second != b.second) {
      return a.second > b.second;
    }
    return a.first < b.first;
  });

  std::vector<indexing::DocID> result;
  result.reserve(ranked.size());
  for (const auto& [doc_id, _] : ranked) {
    result.push_back(doc_id);
  }
  return result;
}

}  // namespace query
