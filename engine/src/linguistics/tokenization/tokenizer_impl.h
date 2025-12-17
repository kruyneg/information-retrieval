#pragma once

#include <memory>

#include "linguistics/tokenization/rules/token_rule.h"
#include "linguistics/tokenization/tokenizer.h"

namespace linguistics {

class TokenizerImpl : public Tokenizer {
 public:
  TokenizerImpl();

  std::vector<std::string> Tokenize(const std::string& text) const override;

 private:
  std::vector<std::unique_ptr<TokenRule>> rules_;
};

}  // namespace linguistics
