#pragma once

#include <string>
#include <vector>

#include "engine/query/ast.h"

namespace linguistics {
class Preprocessor;
}
namespace indexing {
class InvertedIndex;
class PostingList;
}  // namespace indexing

namespace query {

class Query {
 public:
  static Query Parse(const std::string& query,
                     const linguistics::Preprocessor& preprocessor);

  explicit Query(std::unique_ptr<ASTNode>&& tree);

  indexing::PostingList Execute(const indexing::InvertedIndex& index);

 private:
  std::unique_ptr<ASTNode> tree_;
};

}  // namespace query