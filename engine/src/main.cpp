#include <iostream>
#include <locale>
#include <set>

#include "engine/engine.h"
#include "storage/document.h"

std::set<std::string> ParseArgs(int argc, char** argv) {
  std::set<std::string> args;
  for (int i = 1; i < argc; ++i) {
    args.insert(argv[i]);
  }
  return args;
}

int main(int argc, char** argv) {
  std::locale::global(std::locale("ru_RU.UTF-8"));

  const auto args = ParseArgs(argc, argv);
  const bool use_boolean = args.contains("--boolean");
  const bool build_index = args.contains("--build");

  auto engine = CreateEngine();
  if (build_index) {
    engine.BuildIndex();
  } else {
    engine.LoadIndex();
  }
  while (true) {
    std::string query;
    std::cout << "\033[1;35mSearch:\033[m " << std::flush;
    std::getline(std::cin, query);
    if (use_boolean) {
      for (const auto& res : engine.SearchBoolean(query, 10)) {
        std::cout << "\033[4;36m" << res.doc.url << "\033[m" << std::endl;
        std::cout << res.snippet << std::endl;
      }
    } else {
      for (const auto& res : engine.SearchRanked(query, 10)) {
        std::cout << "\033[4;36m" << res.doc.url << "\033[m" << std::endl;
        std::cout << res.snippet << std::endl;
      }
    }
  }
}