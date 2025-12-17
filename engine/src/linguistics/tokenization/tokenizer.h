#pragma once

#include <string>
#include <vector>

namespace linguistics {

class Tokenizer {
 public:
  virtual std::vector<std::string> Tokenize(const std::string& text) const = 0;
};

};  // namespace linguistics