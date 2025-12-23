#pragma once

#include "engine/indexing/compressed_posting_list.h"
#include "engine/indexing/inverted_index.h"

namespace query {

indexing::CompressedPostingList ExecutePhraseQuery(
    const std::vector<std::string>& terms,
    const indexing::InvertedIndex& index);

}  // namespace query
