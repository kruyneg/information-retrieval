#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engine/indexing/types.h"
#include "utils/hash_table.h"

namespace linguistics {
class Preprocessor;
}
namespace indexing {
class InvertedIndex;
}  // namespace indexing

namespace query {

class RankedQuery {
 public:
  static RankedQuery Parse(const std::string& query,
                           const linguistics::Preprocessor& preprocessor);

  explicit RankedQuery(const std::vector<std::string>& terms,
                       const std::vector<std::vector<std::string>>& phrases);

  std::vector<indexing::DocID> Execute(const indexing::InvertedIndex& index);

 private:
  std::vector<std::vector<std::string>> phrases_;
  utils::HashTable<uint32_t> query_tf_;
};

}  // namespace query