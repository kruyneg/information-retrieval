#pragma once

#include "linguistics/lemmatization/lemmatizer.h"

namespace linguistics {

class MockLemmatizer : public Lemmatizer {
 public:
  std::string Lemmatize(const std::string& text) const override;
};

}  // namespace linguistics
