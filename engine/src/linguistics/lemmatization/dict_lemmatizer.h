#pragma once

#include <optional>
#include <unordered_map>

#include "linguistics/lemmatization/lemmatizer.h"

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

  std::unordered_map<std::string, std::string> dict_;
};

}  // namespace linguistics
