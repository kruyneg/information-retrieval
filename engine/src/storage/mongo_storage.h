#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>

#include "storage/storage.h"

namespace storage {

class MongoDocumentCursor : public Storage::Cursor {
 public:
  MongoDocumentCursor(mongocxx::collection col);
  std::optional<Document> Next() override;

 private:
  mongocxx::cursor cursor_;
  std::optional<mongocxx::cursor::iterator> itr_opt_;
};

class MongoStorage : public Storage {
 public:
  MongoStorage(const std::string& uri, const std::string& db_name);

  std::unique_ptr<Storage::Cursor> GetCursor() const override;

  Document GetDocByID(const std::string& doc_id) override;

 private:
  mongocxx::client client_;
  mongocxx::collection articles_;
};

}  // namespace storage
