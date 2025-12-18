#include <iostream>
#include <locale>

#include "engine/engine.h"
#include "storage/document.h"

int main(int argc, char** argv) {
  std::locale::global(std::locale("ru_RU.UTF-8"));

  const bool use_boolean = argc > 1 && std::string(argv[1]) == "--boolean";

  auto engine = CreateEngine();
  engine.BuildIndex();
  while (true) {
    std::string query;
    std::cout << "\033[1;35mSearch:\033[m " << std::flush;
    std::getline(std::cin, query);
    if (use_boolean) {
      for (const auto& doc : engine.SearchBoolean(query)) {
        std::cout << "\033[4;36m" << doc.url << "\033[m" << std::endl;
        if (doc.text.size() > 500) {
          std::cout << doc.text.substr(0, 500) << "..." << std::endl;
        } else {
          std::cout << doc.text << std::endl;
        }
      }
    } else {
      for (const auto& doc : engine.SearchRanked(query)) {
        std::cout << "\033[4;36m" << doc.url << "\033[m" << std::endl;
        if (doc.text.size() > 500) {
          std::cout << doc.text.substr(0, 500) << "..." << std::endl;
        } else {
          std::cout << doc.text << std::endl;
        }
      }
    }
  }
}