#include "linguistics/utils.h"

#include <string>

namespace linguistics {

bool IsEnglish(const std::string& word) {
  return tolower(word[0]) >= 'a' && tolower(word[0]) <= 'z';
}

void NormalizeYo(std::string& text) {
  for (size_t i = 0; i + 1 < text.size(); ++i) {
    if ((unsigned char)text[i] == 0xD1 && (unsigned char)text[i + 1] == 0x91) {
      text.replace(i, 2, "е");
    }
  }
}

std::string ToLower(const std::string& input) {
  std::string result;
  result.reserve(input.size());

  for (size_t i = 0; i < input.size();) {
    unsigned char c = static_cast<unsigned char>(input[i]);

    // ASCII
    if (c < 0x80) {
      if (c >= 'A' && c <= 'Z')
        result.push_back(static_cast<char>(c + 32));
      else
        result.push_back(static_cast<char>(c));
      ++i;
      continue;
    }

    // Cyrillic (2 bytes)
    if (c == 0xD0 && i + 1 < input.size()) {
      unsigned char c2 = static_cast<unsigned char>(input[i + 1]);

      if (c2 >= 0x90 && c2 <= 0x9F) {
        // А–П
        result.push_back(static_cast<char>(0xD0));
        result.push_back(static_cast<char>(c2 + 0x20));
      } else if (c2 >= 0xA0 && c2 <= 0xAF) {
        // Р–Я
        result.push_back(static_cast<char>(0xD1));
        result.push_back(static_cast<char>(c2 - 0x20));
      } else if (c2 == 0x81) {
        // Ё
        result.push_back(static_cast<char>(0xD1));
        result.push_back(static_cast<char>(0x91));
      } else {
        result.push_back(static_cast<char>(c));
        result.push_back(static_cast<char>(c2));
      }

      i += 2;
      continue;
    }

    if (c == 0xD1 && i + 1 < input.size()) {
      result.push_back(static_cast<char>(c));
      result.push_back(static_cast<char>(input[i + 1]));
      i += 2;
      continue;
    }

    result.push_back(static_cast<char>(c));
    ++i;
  }

  return result;
}

bool IsLetter(char16_t c) {
  return (c >= u'A' && c <= u'Z') || (c >= u'a' && c <= u'z') ||
         (c >= u'А' && c <= u'Я') || (c >= u'а' && c <= u'я');
}

}  // namespace linguistics
