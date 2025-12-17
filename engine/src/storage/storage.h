#pragma once

#include <memory>
#include <optional>

namespace storage {

class Document;

class Storage {
 public:
  class Cursor {
   public:
    virtual std::optional<Document> Next() = 0;
  };

  virtual std::unique_ptr<Cursor> GetCursor() const = 0;
};

}  // namespace storage