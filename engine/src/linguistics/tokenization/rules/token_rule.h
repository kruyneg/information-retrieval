#pragma once

#include <string>

namespace linguistics {

class TokenRule {
 public:
  virtual bool Match(const std::u16string& text, size_t pos) const = 0;
  virtual std::u16string Extract(const std::u16string& text,
                                 size_t* pos) const = 0;
};

}  // namespace linguistics
