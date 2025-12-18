#include "engine/query/query.h"

#include <stack>
#include <stdexcept>

#include "engine/indexing/inverted_index.h"
#include "linguistics/preprocessor.h"

namespace {

std::unique_ptr<query::ASTNode> BuildAST(
    const std::vector<std::string>& tokens) {
  std::stack<std::unique_ptr<query::ASTNode>> node_stack;
  std::stack<char> op_stack;

  auto apply_op = [&](char op) {
    if (node_stack.size() < 2) {
      throw std::runtime_error("BuildAST: insufficient operands for operator");
    }
    auto right = std::move(node_stack.top());
    node_stack.pop();
    auto left = std::move(node_stack.top());
    node_stack.pop();
    if (op == '&') {
      node_stack.push(
          query::ASTNode::MakeAnd(std::move(left), std::move(right)));
    } else if (op == '|') {
      node_stack.push(
          query::ASTNode::MakeOr(std::move(left), std::move(right)));
    }
  };

  for (const auto& token : tokens) {
    const auto type = query::GetTermType(token);
    if (token == "(") {
      op_stack.push('(');
    } else if (token == ")") {
      while (!op_stack.empty() && op_stack.top() != '(') {
        apply_op(op_stack.top());
        op_stack.pop();
      }
      if (op_stack.empty()) {
        throw std::runtime_error("BuildAST: ')' without '('");
      }
    } else if (type == query::NodeType::kTerm) {
      node_stack.push(query::ASTNode::MakeTerm(token));
    } else if (type == query::NodeType::kAnd) {
      while (!op_stack.empty() && op_stack.top() == '&') {
        apply_op(op_stack.top());
        op_stack.pop();
      }
      op_stack.push('&');
    } else {
      while (!op_stack.empty()) {
        apply_op(op_stack.top());
        op_stack.pop();
      }
      op_stack.push('|');
    }
  }
  while (!op_stack.empty()) {
    if (op_stack.top() == '(') {
      throw std::runtime_error("BuildAST: '(' without ')'");
    }
    apply_op(op_stack.top());
    op_stack.pop();
  }
  if (node_stack.size() != 1) {
    throw std::runtime_error("BuildAST: leftover nodes in stack");
  }
  return std::move(node_stack.top());
}

indexing::PostingList ExecuteAST(const query::ASTNode& node,
                                 const indexing::InvertedIndex& index) {
  switch (node.type) {
    case query::NodeType::kTerm: {
      return index.GetPostings(node.term);
    }
    case query::NodeType::kAnd: {
      return ExecuteAST(*node.left, index) & ExecuteAST(*node.right, index);
    }
    case query::NodeType::kOr: {
      return ExecuteAST(*node.left, index) | ExecuteAST(*node.right, index);
    }
    default: {
      throw std::runtime_error("ExecuteAST: unknown NodeType");
    }
  }
}

}  // namespace

namespace query {

Query Query::Parse(const std::string& query,
                   const linguistics::Preprocessor& preprocessor) {
  std::vector<std::string> raw_tokens;
  std::string buffer;
  for (char c : query) {
    if (std::isspace(c)) {
      if (!buffer.empty()) {
        raw_tokens.push_back(buffer);
        buffer.clear();
      }
    } else if (c == '(' || c == ')') {
      if (!buffer.empty()) {
        raw_tokens.push_back(buffer);
        buffer.clear();
      }
      raw_tokens.push_back(std::string(1, c));
    } else {
      buffer.push_back(c);
    }
  }
  if (!buffer.empty()) {
    raw_tokens.push_back(buffer);
  }

  std::vector<std::string> tokens;
  for (const auto& token : raw_tokens) {
    if (GetTermType(token) != NodeType::kTerm || token == "(" || token == ")") {
      tokens.push_back(token);
    } else {
      auto processed = preprocessor.Lemmatize(token);
      tokens.push_back(processed);
    }
  }

  return Query(BuildAST(tokens));
}

Query::Query(std::unique_ptr<query::ASTNode>&& ast) : tree_(std::move(ast)) {}

indexing::PostingList Query::Execute(const indexing::InvertedIndex& index) {
  return ExecuteAST(*tree_, index);
}

}  // namespace query