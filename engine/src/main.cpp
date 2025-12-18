#include <iostream>
#include <locale>

#include "engine/engine.h"
#include "storage/document.h"

int main() {
  std::locale::global(std::locale("ru_RU.UTF-8"));

  auto engine = CreateEngine();
  engine.BuildIndex();
  while (true) {
    std::string query;
    std::cout << "Search:" << std::flush;
    std::getline(std::cin, query);
    for (const auto& doc : engine.Search(query)) {
      std::cout << doc.url << std::endl;
      if (doc.text.size() > 500) {
        std::cout << doc.text.substr(0, 500) << "..." << std::endl;
      } else {
        std::cout << doc.text << std::endl;
      }
    }
  }
}