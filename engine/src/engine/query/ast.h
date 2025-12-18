#pragma once

#include <memory>
#include <string>

namespace query {

enum class NodeType {
  kTerm,
  kAnd,
  kOr,
};

NodeType GetTermType(const std::string& term);

struct ASTNode {
  static std::unique_ptr<ASTNode> MakeTerm(const std::string& term);
  static std::unique_ptr<ASTNode> MakeAnd(std::unique_ptr<ASTNode> left,
                                          std::unique_ptr<ASTNode> right);
  static std::unique_ptr<ASTNode> MakeOr(std::unique_ptr<ASTNode> left,
                                         std::unique_ptr<ASTNode> right);

  NodeType type;
  std::string term;
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
};

}  // namespace query
