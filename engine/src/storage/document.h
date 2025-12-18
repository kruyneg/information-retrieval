#pragma once

#include <string>

namespace storage {

struct Document {
  std::string id;
  std::string url;
  std::string text;
};

}  // namespace storage