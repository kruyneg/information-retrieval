#include "linguistics/tokenization/rules/abbreviation_rule.h"

#include "linguistics/utils.h"

namespace linguistics {

bool AbbreviationRule::Match(const std::u16string& text, size_t pos) const {
  size_t i;
  for (i = pos; pos + 1 < text.size() && IsUpper(text[i]) && text[i + 1] == '.';
       i += 2);
  return i > pos + 1;
}

std::u16string AbbreviationRule::Extract(const std::u16string& text,
                                         size_t* pos) const {
  if (!pos) {
    return u"";
  }
  std::u16string result;
  while (*pos < text.size() && (IsUpper(text[*pos]) || text[*pos] == '.')) {
    if (text[*pos] != '.') {
      result.push_back(text[*pos]);
    }
    ++*pos;
  }
  return result;
}

}  // namespace linguistics
