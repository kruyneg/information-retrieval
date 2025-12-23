#pragma once

#include <optional>

#include "linguistics/lemmatization/lemmatizer.h"
#include "utils/hash_table.h"

namespace linguistics {

class DictLemmatizer : public Lemmatizer {
 public:
  static const std::string kDefaultRuPath;
  static const std::string kDefaultEnPath;

  DictLemmatizer(const std::string& ru_dict_path = kDefaultRuPath,
                 const std::string& en_dict_path = kDefaultEnPath);

  std::string Lemmatize(const std::string& word) const override;

  void LoadDict(const std::string& dict_path);

 private:
  std::string Stem(const std::string& word) const;

  utils::HashTable<std::string> dict_;
};

}  // namespace linguistics
