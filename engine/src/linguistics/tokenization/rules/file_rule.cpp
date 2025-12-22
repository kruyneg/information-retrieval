#include "linguistics/tokenization/rules/file_rule.h"

#include <wctype.h>

#include "linguistics/utils.h"

namespace linguistics {

bool FileRule::Match(const std::u16string& text, size_t pos) const {
  auto c = text[pos];

  if (c == u'/' || c == u'~') {
    return true;
  }
  if (pos + 3 < text.size() && c == u'.' &&
      (text[pos + 1] == u'/' ||
       (text[pos + 1] == u'.' && text[pos + 2] == u'/'))) {
    return true;
  }
  if (pos + 2 < text.size() && IsLetter(c) && text[pos + 1] == u':' &&
      (text[pos + 2] == u'\\' || text[pos + 2] == u'/')) {
    return true;
  }
  return false;
}

std::u16string FileRule::Extract(const std::u16string& text,
                                 size_t* pos) const {
  if (!pos) {
    return u"";
  }

  size_t i = *pos;
  size_t start = i;
  auto is_path_char = [](char16_t c) {
    return iswalnum(c) || c == u'_' || c == u'-' || c == u'/' || c == u'\\' ||
           c == u'.' || c == u':' || c == u'~';
  };
  while (i < text.size() && is_path_char(text[i])) {
    ++i;
  }
  *pos = i;
  return text.substr(start, i - start);
}

}  // namespace linguistics
