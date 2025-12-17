#include "linguistics/tokenization/rules/number_rule.h"

#include <wctype.h>

namespace linguistics {

bool NumberRule::Match(const std::u16string& text, size_t pos) const {
  if (!iswdigit(text[pos])) {
    return false;
  } else if (pos + 1 < text.size() && (text[pos] == '-' || text[pos] == '+') &&
             !iswdigit(text[pos + 1])) {
    return false;
  }
  return true;
}

std::u16string NumberRule::Extract(const std::u16string& text,
                                   size_t* pos) const {
  if (!pos) {
    return u"";
  }
  std::u16string result;
  bool has_dot = false;
  for (; *pos < text.size(); ++*pos) {
    wchar_t c = text[*pos];
    if (iswdigit(c)) {
      result.push_back(c);
    } else if (c == u'.' && !has_dot && *pos + 1 < text.size() &&
               iswdigit(text[*pos + 1])) {
      result.push_back(c);
    } else {
      break;
    }
  }
  return result;
}

}  // namespace linguistics
