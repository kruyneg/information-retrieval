#include <iostream>
#include <locale>

#include "linguistics/preprocessor.h"
#include "storage/document.h"
#include "storage/mongo_storage.h"

int main() {
  std::locale::global(std::locale("ru_RU.UTF-8"));

  auto prep = linguistics::CreatePreprocessor();

  storage::MongoStorage store("mongodb://user:password@localhost:27017/",
                              "parser_db");

  auto tokens = prep.Preprocess(store.GetCursor()->Next().value().text);

  for (auto token : tokens) {
    std::cout << std::string(token.begin(), token.end()) << std::endl;
  }
}