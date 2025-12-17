#pragma once

#include <string>

namespace linguistics {

class Lemmatizer {
 public:
  virtual std::string Lemmatize(const std::string&) const = 0;
};

}  // namespace linguistics
