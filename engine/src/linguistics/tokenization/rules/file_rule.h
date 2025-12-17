#pragma once

#include "linguistics/tokenization/rules/token_rule.h"

namespace linguistics {

class FileRule : public TokenRule {
 public:
  bool Match(const std::u16string& text, size_t pos) const override;
  std::u16string Extract(const std::u16string& text,
                         size_t* pos) const override;
};

}  // namespace linguistics
