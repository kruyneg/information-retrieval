#pragma once

#include <memory>
#include <optional>

namespace storage {

class Document;

class DocStorage {
 public:
  class Cursor {
   public:
    virtual std::optional<Document> Next() = 0;
  };

  virtual std::unique_ptr<Cursor> GetCursor() const = 0;
  virtual Document GetDocByID(int32_t doc_id) = 0;
};

}  // namespace storage