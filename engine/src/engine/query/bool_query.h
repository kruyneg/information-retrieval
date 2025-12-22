#pragma once

#include <string>
#include <vector>

#include "engine/query/ast.h"

namespace linguistics {
class Preprocessor;
}
namespace indexing {
class InvertedIndex;
class CompressedPostingList;
}  // namespace indexing

namespace query {

class BoolQuery {
 public:
  static BoolQuery Parse(const std::string& query,
                         const linguistics::Preprocessor& preprocessor);

  explicit BoolQuery(std::unique_ptr<ASTNode>&& tree);

  indexing::CompressedPostingList Execute(const indexing::InvertedIndex& index);

 private:
  std::unique_ptr<ASTNode> tree_;
};

}  // namespace query