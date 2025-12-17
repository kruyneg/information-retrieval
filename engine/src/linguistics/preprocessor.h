#pragma once

#include <memory>

#include "linguistics/lemmatization/lemmatizer.h"
#include "linguistics/tokenization/tokenizer.h"

namespace linguistics {

class Preprocessor {
 public:
  Preprocessor(std::unique_ptr<Tokenizer>&& tokenizer,
               std::unique_ptr<Lemmatizer>&& lemmatizer);

  std::vector<std::string> Preprocess(const std::string& text) const;

 private:
  std::unique_ptr<Tokenizer> tokenizer_;
  std::unique_ptr<Lemmatizer> lemmatizer_;
};

Preprocessor CreatePreprocessor();

}  // namespace linguistics
