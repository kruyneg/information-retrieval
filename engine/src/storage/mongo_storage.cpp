#include "storage/mongo_storage.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/uri.hpp>

#include "storage/document.h"

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

Document MongoStorage::GetDocByID(const std::string& doc_id) {
  bsoncxx::oid oid;
  try {
    oid = bsoncxx::oid(doc_id);
  } catch (const bsoncxx::exception& e) {
    throw std::runtime_error("MongoStorage: invalid doc_id format: " + doc_id);
  }

  bsoncxx::builder::basic::document filter_builder;
  filter_builder.append(bsoncxx::builder::basic::kvp("_id", oid));

  auto maybe_doc = articles_.find_one(filter_builder.view());
  if (!maybe_doc) {
    throw std::runtime_error("MongoStorage: document not found for id: " +
                             doc_id);
  }

  auto doc_view = maybe_doc->view();

  Document result;
  result.id = doc_id;

  if (doc_view["url"] && doc_view["url"].type() == bsoncxx::type::k_string) {
    result.url = doc_view["url"].get_string().value;
  } else {
    result.url = "";
  }

  if (doc_view["text"] && doc_view["text"].type() == bsoncxx::type::k_string) {
    result.text = doc_view["text"].get_string().value;
  } else {
    result.text = "";
  }

  return result;
}

std::unique_ptr<Storage::Cursor> MongoStorage::GetCursor() const {
  return std::make_unique<MongoDocumentCursor>(articles_);
}

MongoDocumentCursor::MongoDocumentCursor(mongocxx::collection col)
    : cursor_(col.find(bsoncxx::builder::basic::make_document(),
                       mongocxx::options::find{})),
      itr_opt_(std::nullopt) {}

std::optional<Document> MongoDocumentCursor::Next() {
  if (!itr_opt_.has_value()) {
    itr_opt_ = cursor_.begin();
  }
  if (itr_opt_.value() == cursor_.end()) return std::nullopt;

  auto& doc = *itr_opt_.value();
  ++itr_opt_.value();

  Document result;
  if (auto el = doc["_id"]; el && el.type() == bsoncxx::type::k_oid) {
    result.id = el.get_oid().value.to_string();
  }

  if (auto el = doc["url"]; el && el.type() == bsoncxx::type::k_string) {
    result.url = el.get_string().value;
  }

  if (auto el = doc["text"]; el && el.type() == bsoncxx::type::k_string) {
    result.text = doc["text"].get_string().value.begin();
  }
  return result;
}

}  // namespace storage
