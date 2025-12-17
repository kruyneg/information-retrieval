#include "mock_lemmatizer.h"

namespace linguistics {

std::string MockLemmatizer::Lemmatize(const std::string& text) const {
  return text;
}

}  // namespace linguistics
