#include "engine/query/ast.h"

#include "linguistics/utils.h"

namespace query {

NodeType GetTermType(const std::string& term) {
  auto lower_term = linguistics::ToLower(term);
  if (lower_term == "and" || lower_term == "и" || lower_term == "&" ||
      lower_term == "&&") {
    return NodeType::kAnd;
  } else if (lower_term == "or" || lower_term == "или" || lower_term == "|" ||
             lower_term == "||") {
    return NodeType::kOr;
  } else if (lower_term.front() == '"' && lower_term.back() == '"') {
    return NodeType::kPhrase;
  }
  return NodeType::kTerm;
}

std::unique_ptr<ASTNode> ASTNode::MakeTerm(const std::string& term) {
  auto node = std::make_unique<ASTNode>();
  node->type = NodeType::kTerm;
  node->terms = {term};
  return node;
}

std::unique_ptr<ASTNode> ASTNode::MakePhrase(
    const std::vector<std::string>& phrase) {
  auto node = std::make_unique<ASTNode>();
  node->type = NodeType::kPhrase;
  node->terms = phrase;
  return node;
}

std::unique_ptr<ASTNode> ASTNode::MakeAnd(std::unique_ptr<ASTNode> left,
                                          std::unique_ptr<ASTNode> right) {
  auto node = std::make_unique<ASTNode>();
  node->type = NodeType::kAnd;
  node->left = std::move(left);
  node->right = std::move(right);
  return node;
}

std::unique_ptr<ASTNode> ASTNode::MakeOr(std::unique_ptr<ASTNode> left,
                                         std::unique_ptr<ASTNode> right) {
  auto node = std::make_unique<ASTNode>();
  node->type = NodeType::kOr;
  node->left = std::move(left);
  node->right = std::move(right);
  return node;
}

}  // namespace query
