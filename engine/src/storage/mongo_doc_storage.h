#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>

#include "storage/doc_storage.h"

namespace storage {

class MongoDocumentCursor : public DocStorage::Cursor {
 public:
  MongoDocumentCursor(mongocxx::collection col);
  std::optional<Document> Next() override;

 private:
  mongocxx::cursor cursor_;
  std::optional<mongocxx::cursor::iterator> itr_opt_;
};

class MongoDocStorage : public DocStorage {
 public:
  MongoDocStorage(const std::string& uri, const std::string& db_name);

  std::unique_ptr<DocStorage::Cursor> GetCursor() const override;

  Document GetDocByID(int32_t doc_id) override;

 private:
  mongocxx::client client_;
  mongocxx::collection articles_;
};

}  // namespace storage
