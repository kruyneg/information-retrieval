#include "linguistics/tokenization/tokenizer_impl.h"

#include <codecvt>
#include <locale>

#include "linguistics/tokenization/rules/abbreviation_rule.h"
#include "linguistics/tokenization/rules/number_rule.h"
#include "linguistics/tokenization/rules/word_rule.h"

namespace {

inline std::u16string Utf8ToU16(const std::string& utf8) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return conv.from_bytes(utf8);
}

inline std::string U16ToUtf8(const std::u16string& u16) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return conv.to_bytes(u16);
}

}  // namespace

namespace linguistics {

TokenizerImpl::TokenizerImpl() {
  rules_.emplace_back(std::make_unique<AbbreviationRule>());
  rules_.emplace_back(std::make_unique<NumberRule>());
  rules_.emplace_back(std::make_unique<WordRule>());
}

std::vector<std::string> TokenizerImpl::Tokenize(
    const std::string& text) const {
  std::vector<std::string> tokens;

  const auto text16 = Utf8ToU16(text);

  size_t pos = 0;
  while (pos < text16.size()) {
    bool matched = false;
    for (const auto& rule : rules_) {
      if (rule->Match(text16, pos)) {
        matched = true;
        auto token = U16ToUtf8(rule->Extract(text16, &pos));
        if (!token.empty()) {
          tokens.emplace_back(std::move(token));
          break;
        }
      }
    }
    if (!matched) {
      ++pos;
    }
  }

  return tokens;
}

}  // namespace linguistics
