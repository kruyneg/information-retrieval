#include "linguistics/lemmatization/dict_lemmatizer.h"

#include "linguistics/lemmatization/stemming.h"
#include "linguistics/utils.h"

#include <fstream>
#include <algorithm>
#include <sstream>

namespace linguistics {

const std::string DictLemmatizer::kDefaultRuPath = "data/ru_lemmas.txt";
const std::string DictLemmatizer::kDefaultEnPath = "data/en_lemmas.txt";

DictLemmatizer::DictLemmatizer(
    const std::string& ru_dict_path /* = kDefaultRuPath */,
    const std::string& en_dict_path /* = kDefaultEnPath */) {
  LoadDict(ru_dict_path);
  LoadDict(en_dict_path);
}

void DictLemmatizer::LoadDict(const std::string& dict_path) {
  std::ifstream fin(dict_path);
  if (!fin.is_open()) {
    throw std::runtime_error("DictLemmatizer: can't open dict " + dict_path);
  }

  std::string line;
  while (std::getline(fin, line)) {
    std::istringstream ss(line);
    std::string word, lemma;
    if (ss >> word >> lemma) {
      for (size_t i = 0; i + 1 < word.size(); ++i) {
        if ((unsigned char)word[i] == 0xD1 &&
            (unsigned char)word[i + 1] == 0x91) {
          word.replace(i, 2, "е");
        }
      }
      for (size_t i = 0; i + 1 < lemma.size(); ++i) {
        if ((unsigned char)lemma[i] == 0xD1 &&
            (unsigned char)lemma[i + 1] == 0x91) {
          lemma.replace(i, 2, "е");
        }
      }
      dict_[word] = lemma;
    }
  }
}

std::string DictLemmatizer::Lemmatize(const std::string& word) const {
  auto normalized = ToLower(word);
  auto itr = dict_.find(normalized);
  if (itr != dict_.end()) {
    return itr->second;
  }
  return Stem(normalized);
}

std::string DictLemmatizer::Stem(const std::string& word) const {
  if (IsEnglish(word)) {
    return StemEn(word);
  } else {
    return StemRu(word);
  }
}

}  // namespace linguistics
