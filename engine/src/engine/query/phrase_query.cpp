#include "engine/query/phrase_query.h"

using Iterator = indexing::CompressedPostingList::DocIterator;

namespace {

bool HasPhrase(const std::vector<Iterator>& iters) {
  for (auto base = iters[0].GetCoordItr(); !base.IsEnd(); ++base) {
    bool ok = true;

    for (size_t i = 1; i < iters.size(); ++i) {
      auto coord_itr = iters[i].GetCoordItr();
      while (!coord_itr.IsEnd() && *coord_itr < *base + i) {
        ++coord_itr;
      }

      if (coord_itr.IsEnd() || *coord_itr != *base + i) {
        ok = false;
        break;
      }
    }

    if (ok) {
      return true;
    }
  }
  return false;
}

}  // namespace

namespace query {

indexing::CompressedPostingList ExecutePhraseQuery(
    const std::vector<std::string>& terms,
    const indexing::InvertedIndex& index) {
  indexing::CompressedPostingList result;

  std::vector<Iterator> iters;
  for (auto& term : terms) {
    iters.push_back(index.GetPostings(term).begin());
  }

  while (true) {
    indexing::DocID target = 0;

    for (auto& itr : iters) {
      if (itr.IsEnd()) {
        return result;
      }
      target = std::max(target, itr->doc_id);
    }

    bool synced = true;
    for (auto& itr : iters) {
      if (itr->doc_id != target) {
        itr.SkipTo(target);
        synced = false;
      }
    }
    if (!synced) {
      continue;
    }

    if (HasPhrase(iters)) {
      result.Add(target, {});
    }

    for (auto& itr : iters) {
      ++itr;
    }
  }

  return result;
}

}  // namespace query
