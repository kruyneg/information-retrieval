#include "storage/mongo_storage.h"

#include <bsoncxx/v_noabi/bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/v_noabi/mongocxx/options/find.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>

#include "storage/document.h"

namespace bson = bsoncxx::v_noabi;

namespace storage {

MongoStorage::MongoStorage(const std::string& uri, const std::string& db_name)
    : client_(mongocxx::uri{uri}) {
  try {
    auto database = client_[db_name];

    database.run_command(bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("ping", 1)));

    articles_ = database["articles"];
  } catch (const mongocxx::exception& e) {
    throw std::runtime_error(std::string("MongoDB connection failed: ") +
                             e.what());
  }
}

std::unique_ptr<Storage::Cursor> MongoStorage::GetCursor() const {
  return std::make_unique<MongoDocumentCursor>(articles_);
}

MongoDocumentCursor::MongoDocumentCursor(mongo::collection col)
    : cursor_(col.find(bson::builder::basic::make_document(),
                       mongo::options::find{})),
      itr_opt_(std::nullopt) {}

std::optional<Document> MongoDocumentCursor::Next() {
  if (!itr_opt_.has_value()) {
    itr_opt_ = cursor_.begin();
  }
  if (itr_opt_.value() == cursor_.end()) return std::nullopt;

  auto& doc = *itr_opt_.value();
  ++itr_opt_.value();

  Document result;
  if (auto el = doc["url"]; el && el.type() == bsoncxx::type::k_string) {
    result.url = std::string(el.get_string().value);
  }

  if (auto el = doc["text"]; el && el.type() == bsoncxx::type::k_string) {
    result.text = std::string(doc["text"].get_string().value.begin(),
                              doc["text"].get_string().value.end());
  }
  return result;
}

}  // namespace storage
