#pragma once

#include <string>

namespace storage {

struct Document {
  int32_t id;
  std::string url;
  std::string text;
};

}  // namespace storage