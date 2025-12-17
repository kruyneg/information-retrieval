#pragma once

#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/collection.hpp>
#include <mongocxx/v_noabi/mongocxx/cursor.hpp>

#include "storage/storage.h"

namespace mongo = mongocxx::v_noabi;

namespace storage {

class MongoDocumentCursor : public Storage::Cursor {
 public:
  MongoDocumentCursor(mongo::collection col);
  std::optional<Document> Next() override;

 private:
  mongo::cursor cursor_;
  std::optional<mongo::cursor::iterator> itr_opt_;
};

class MongoStorage : public Storage {
 public:
  MongoStorage(const std::string& uri, const std::string& db_name);

  std::unique_ptr<Storage::Cursor> GetCursor() const override;

 private:
  mongo::client client_;
  mongo::collection articles_;
};

}  // namespace storage
