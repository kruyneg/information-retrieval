#include "linguistics/preprocessor.h"

#include "linguistics/lemmatization/dict_lemmatizer.h"
#include "linguistics/tokenization/tokenizer_impl.h"

namespace linguistics {

Preprocessor CreatePreprocessor() {
  auto tokenizer = std::make_unique<TokenizerImpl>();
  auto lemmatizer = std::make_unique<DictLemmatizer>();
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

std::vector<std::string> Preprocessor::Tokenize(const std::string& text) const {
  return tokenizer_->Tokenize(text);
}

std::string Preprocessor::Lemmatize(const std::string& word) const {
  return lemmatizer_->Lemmatize(word);
}

}  // namespace linguistics
