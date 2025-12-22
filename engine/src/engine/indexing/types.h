#pragma once

#include <cstdint>

namespace indexing {

using DocID = uint32_t;

struct Posting {
  uint32_t doc_id;
  uint32_t tf;

  bool operator==(const Posting&) const = default;
};

}  // namespace indexing
