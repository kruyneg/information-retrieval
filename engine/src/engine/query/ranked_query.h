#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace linguistics {
class Preprocessor;
}
namespace indexing {
class InvertedIndex;
class PostingList;
}  // namespace indexing

namespace query {

class RankedQuery {
 public:
  static RankedQuery Parse(const std::string& query,
                           const linguistics::Preprocessor& preprocessor);

  explicit RankedQuery(const std::vector<std::string>& terms);

  std::vector<std::string> Execute(const indexing::InvertedIndex& index);

 private:
  std::unordered_map<std::string, uint32_t> query_tf_;
};

}  // namespace query