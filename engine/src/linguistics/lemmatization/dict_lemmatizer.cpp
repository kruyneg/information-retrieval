#include "linguistics/lemmatization/dict_lemmatizer.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>

#include "linguistics/lemmatization/stemming.h"
#include "linguistics/utils.h"

namespace {

bool EndsWith(const std::string& s, const std::string& suf) {
  return s.size() >= suf.size() &&
         s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

}  // namespace

namespace linguistics {

const std::string DictLemmatizer::kDefaultRuPath =
    "/home/kruyneg/Programming/InformationRetrieval/engine/data/ru_lemmas.bin";
const std::string DictLemmatizer::kDefaultEnPath =
    "/home/kruyneg/Programming/InformationRetrieval/engine/data/en_lemmas.bin";

DictLemmatizer::DictLemmatizer(
    const std::string& ru_dict_path /* = kDefaultRuPath */,
    const std::string& en_dict_path /* = kDefaultEnPath */) {
  LoadDict(ru_dict_path);
  LoadDict(en_dict_path);
}

void DictLemmatizer::LoadDict(const std::string& dict_path) {
  if (EndsWith(dict_path, ".txt")) {
    std::ifstream fin(dict_path);
    if (!fin.is_open()) {
      throw std::runtime_error("DictLemmatizer: can't open dict " + dict_path);
    }

    std::string line;
    while (std::getline(fin, line)) {
      std::istringstream ss(line);
      std::string word, lemma;
      if (ss >> word >> lemma) {
        NormalizeYo(word);
        NormalizeYo(lemma);
        dict_[std::move(word)] = std::move(lemma);
      }
    }
    return;
  }

  if (EndsWith(dict_path, ".bin")) {
    std::ifstream fin(dict_path, std::ios::binary);
    if (!fin.is_open()) {
      throw std::runtime_error("DictLemmatizer: can't open dict " + dict_path);
    }

    uint32_t count = 0;
    fin.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!fin) {
      throw std::runtime_error("DictLemmatizer: corrupted bin header");
    }

    dict_.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
      uint16_t wl = 0, ll = 0;

      fin.read(reinterpret_cast<char*>(&wl), sizeof(wl));
      std::string word(wl, '\0');
      fin.read(word.data(), wl);

      fin.read(reinterpret_cast<char*>(&ll), sizeof(ll));
      std::string lemma(ll, '\0');
      fin.read(lemma.data(), ll);

      if (!fin) {
        throw std::runtime_error("DictLemmatizer: corrupted bin entry");
      }

      NormalizeYo(word);
      NormalizeYo(lemma);
      dict_[std::move(word)] = std::move(lemma);
    }
    return;
  }

  throw std::runtime_error("DictLemmatizer: unknown dict format " + dict_path);
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
