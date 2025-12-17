#include "linguistics/tokenization/rules/word_rule.h"

#include <wctype.h>

namespace linguistics {

bool WordRule::Match(const std::u16string& text, size_t pos) const {
  if (!iswalpha(wchar_t(text[pos]))) {
    return false;
  }
  return true;
}

std::u16string WordRule::Extract(const std::u16string& text,
                                 size_t* pos) const {
  if (!pos) {
    return u"";
  }
  std::u16string result;
  for (; *pos < text.size(); ++*pos) {
    wchar_t c = text[*pos];
    if (iswalpha(c) || iswdigit(c) || c == u'\'' || c == u'-' || c == u'+' ||
        c == u'#') {
      result.push_back(c);
    } else if (c == '.') {
      if (*pos + 1 < text.size() && iswalpha(text[*pos - 1]) &&
          iswalpha(text[*pos + 1])) {
        result.push_back(c);
      } else {
        break;
      }
    } else {
      break;
    }
  }
  return result;
}

}  // namespace linguistics
