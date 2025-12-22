#include "linguistics/lemmatization/stemming.h"

#include <algorithm>
#include <vector>

#include "linguistics/utils.h"

namespace {

bool IsVowel(char c) {
  switch (c) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return true;
    default:
      return false;
  }
}

void ReplaceSuffix(std::string* s, const std::string& suffix,
                   const std::string& replacement) {
  s->replace(s->size() - suffix.size(), suffix.size(), replacement);
}

bool ContainsVowel(const std::string& s) {
  for (char c : s) {
    if (IsVowel(c)) return true;
  }
  return false;
}

int Measure(const std::string& s) {
  int m = 0;
  bool prev_vowel = false;

  for (char c : s) {
    const bool is_vowel = IsVowel(c);
    if (!is_vowel && prev_vowel) ++m;
    prev_vowel = is_vowel;
  }
  return m;
}

bool EndsWithDoubleConsonant(const std::string& s) {
  if (s.size() < 2) return false;
  const char c1 = s[s.size() - 1];
  const char c2 = s[s.size() - 2];
  return c1 == c2 && !IsVowel(c1);
}

bool EndsWithCvc(const std::string& s) {
  if (s.size() < 3) return false;
  const char c1 = s[s.size() - 1];
  const char c2 = s[s.size() - 2];
  const char c3 = s[s.size() - 3];

  return !IsVowel(c1) && IsVowel(c2) && !IsVowel(c3) && c1 != 'w' &&
         c1 != 'x' && c1 != 'y';
}

void ToLowerAscii(std::string* s) {
  for (char& c : *s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
}

bool EndsWith(const std::string& s, const std::string& suffix) {
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

}  // namespace

namespace linguistics {

std::string StemRu(const std::string& word) {
  if (word.size() < 4) {
    return word;
  }

  auto result = word;

  NormalizeYo(result);

  static const std::vector<std::string> noun = {
      "иями", "ями", "ами", "иях", "ях", "ах", "ов", "ев", "ей", "ия",
      "ья",   "ие",  "ье",  "ы",   "а",  "я",  "о",  "е",  "и",  "у"};

  static const std::vector<std::string> verb = {
      "илась", "ылась", "илась", "ился", "илась", "ился", "ила", "ыла",
      "ена",   "ить",   "ать",   "ять",  "еть",   "ишь",  "ую",  "ю",
      "ал",    "ала",   "али",   "ло",   "ли",    "л"};

  static const std::vector<std::string> adj = {
      "ейший", "ейше", "ей", "ая", "ое", "ой", "ые", "ий", "ый", "ого", "ему"};

  auto sufs = noun;
  sufs.insert(sufs.end(), verb.begin(), verb.end());
  sufs.insert(sufs.end(), adj.begin(), adj.end());
  std::sort(sufs.begin(), sufs.end(),
            [](const auto& a, const auto& b) { return a.size() > b.size(); });

  for (const auto& suf : sufs) {
    if (EndsWith(result, suf)) {
      result.resize(result.size() - suf.size());
      return result;
    }
  }

  return result;
}

std::string StemEn(const std::string& word) {
  if (word.size() < 3) return word;

  std::string s = word;
  ToLowerAscii(&s);

  // Step 1a
  if (EndsWith(s, "sses")) {
    ReplaceSuffix(&s, "sses", "ss");
  } else if (EndsWith(s, "ies")) {
    ReplaceSuffix(&s, "ies", "i");
  } else if (EndsWith(s, "ss")) {
    // no-op
  } else if (EndsWith(s, "s")) {
    s.pop_back();
  }

  // Step 1b
  bool step1b_success = false;

  if (EndsWith(s, "eed")) {
    const std::string stem = s.substr(0, s.size() - 3);
    if (Measure(stem) > 0) {
      ReplaceSuffix(&s, "eed", "ee");
    }
  } else if (EndsWith(s, "ed") && ContainsVowel(s.substr(0, s.size() - 2))) {
    s.resize(s.size() - 2);
    step1b_success = true;
  } else if (EndsWith(s, "ing") && ContainsVowel(s.substr(0, s.size() - 3))) {
    s.resize(s.size() - 3);
    step1b_success = true;
  }

  if (step1b_success) {
    if (EndsWith(s, "at") || EndsWith(s, "bl") || EndsWith(s, "iz")) {
      s.push_back('e');
    } else if (EndsWithDoubleConsonant(s) && s.back() != 'l' &&
               s.back() != 's' && s.back() != 'z') {
      s.pop_back();
    } else if (Measure(s) == 1 && EndsWithCvc(s)) {
      s.push_back('e');
    }
  } else {
    // Step 1c
    if (EndsWith(s, "y") && ContainsVowel(s.substr(0, s.size() - 1))) {
      s[s.size() - 1] = 'i';
    }
  }
  return s;
}

}  // namespace linguistics
