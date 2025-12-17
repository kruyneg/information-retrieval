#include <iostream>
#include <locale>

#include "linguistics/tokenization/tokenizer.h"
#include "linguistics/tokenization/tokenizer_impl.h"
#include "storage/document.h"
#include "storage/mongo_storage.h"

int main() {
  std::locale::global(std::locale("ru_RU.UTF-8"));

  std::unique_ptr<linguistics::Tokenizer> tokenizer =
      std::make_unique<linguistics::TokenizerImpl>();

  storage::MongoStorage store("mongodb://user:password@localhost:27017/",
                              "parser_db");

  auto tokens = tokenizer->Tokenize(store.GetCursor()->Next().value().text);

  for (auto token : tokens) {
    std::cout << std::string(token.begin(), token.end()) << std::endl;
  }
}