#include "storage/file_index_storage.h"

#include "engine/indexing/inverted_index.h"

namespace storage {

FileIndexStorage::FileIndexStorage(const std::filesystem::path& filename)
    : file_(filename, std::ios::in | std::ios::out | std::ios::binary) {}

void FileIndexStorage::SaveIndex(const indexing::InvertedIndex& index) {
  if (!file_.is_open()) {
    throw std::runtime_error("FileIndexStorage: can't open file");
  }

  const auto terms_count = index.index_.size();
  file_.write(reinterpret_cast<const char*>(&terms_count), sizeof(terms_count));
  for (const auto& [term, posting_list] : index.index_) {
    const auto term_size = term.size();
    file_.write(reinterpret_cast<const char*>(&term_size), sizeof(term_size));
    file_.write(reinterpret_cast<const char*>(term.data()), term_size);

    const auto buf_size = posting_list.buffer_.size();
    file_.write(reinterpret_cast<const char*>(&buf_size), sizeof(buf_size));
    file_.write(reinterpret_cast<const char*>(posting_list.buffer_.data()),
                buf_size);
  }

  const auto docs_count = index.doc_lengths_.size();
  file_.write(reinterpret_cast<const char*>(&docs_count), sizeof(docs_count));
  for (const auto& [doc_id, length] : index.doc_lengths_) {
    file_.write(reinterpret_cast<const char*>(&doc_id), sizeof(doc_id));
    file_.write(reinterpret_cast<const char*>(&length), sizeof(length));
  }
  file_.flush();
  file_.seekg(0, std::ios::beg);
  file_.seekp(0, std::ios::beg);
}

indexing::InvertedIndex FileIndexStorage::LoadIndex() {
  if (!file_.is_open()) {
    throw std::runtime_error("FileIndexStorage: can't open file");
  }

  indexing::InvertedIndex index;

  size_t terms_count;
  file_.read(reinterpret_cast<char*>(&terms_count), sizeof(terms_count));
  for (size_t i = 0; i < terms_count; ++i) {
    size_t term_size;
    file_.read(reinterpret_cast<char*>(&term_size), sizeof(term_size));
    std::string term(term_size, '\0');
    file_.read(reinterpret_cast<char*>(term.data()), term_size);

    size_t buf_size;
    file_.read(reinterpret_cast<char*>(&buf_size), sizeof(buf_size));
    indexing::CompressedPostingList posting_list;
    posting_list.buffer_.resize(buf_size);
    file_.read(reinterpret_cast<char*>(posting_list.buffer_.data()), buf_size);
    index.index_[std::move(term)] = std::move(posting_list);
  }

  size_t docs_count;
  file_.read(reinterpret_cast<char*>(&docs_count), sizeof(docs_count));
  for (size_t i = 0; i < docs_count; ++i) {
    uint32_t doc_id, length;
    file_.read(reinterpret_cast<char*>(&doc_id), sizeof(doc_id));
    file_.read(reinterpret_cast<char*>(&length), sizeof(length));
    index.doc_lengths_[doc_id] = length;
  }

  file_.flush();
  file_.seekg(0, std::ios::beg);
  file_.seekp(0, std::ios::beg);

  return index;
}

}  // namespace storage
