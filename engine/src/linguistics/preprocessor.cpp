#include "linguistics/preprocessor.h"

#include "linguistics/lemmatization/mock_lemmatizer.h"
#include "linguistics/tokenization/tokenizer_impl.h"

namespace linguistics {

Preprocessor CreatePreprocessor() {
  auto tokenizer = std::make_unique<TokenizerImpl>();
  auto lemmatizer = std::make_unique<MockLemmatizer>();
  return Preprocessor(std::move(tokenizer), std::move(lemmatizer));
}

Preprocessor::Preprocessor(std::unique_ptr<Tokenizer>&& tokenizer,
                           std::unique_ptr<Lemmatizer>&& lemmatizer)
    : tokenizer_(std::move(tokenizer)), lemmatizer_(std::move(lemmatizer)) {}

std::vector<std::string> Preprocessor::Preprocess(
    const std::string& text) const {
  auto result = tokenizer_->Tokenize(text);
  for (auto& word : result) {
    word = lemmatizer_->Lemmatize(word);
  }
  return result;
}

}  // namespace linguistics
