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

    const auto list_size = posting_list.size();
    file_.write(reinterpret_cast<const char*>(&list_size), sizeof(list_size));

    const auto doc_buf_size = posting_list.doc_buffer_.size();
    file_.write(reinterpret_cast<const char*>(&doc_buf_size),
                sizeof(doc_buf_size));
    file_.write(reinterpret_cast<const char*>(posting_list.doc_buffer_.data()),
                doc_buf_size);

    const auto coord_buf_size = posting_list.coord_buffer_.size();
    file_.write(reinterpret_cast<const char*>(&coord_buf_size),
                sizeof(coord_buf_size));
    file_.write(
        reinterpret_cast<const char*>(posting_list.coord_buffer_.data()),
        coord_buf_size);
  }

  const auto docs_count = index.doc_lengths_.size();
  file_.write(reinterpret_cast<const char*>(&docs_count), sizeof(docs_count));
  file_.write(reinterpret_cast<const char*>(index.doc_lengths_.data()),
              sizeof(uint32_t) * docs_count);
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

    indexing::CompressedPostingList posting_list;

    size_t list_size;
    file_.read(reinterpret_cast<char*>(&list_size), sizeof(list_size));
    posting_list.size_ = list_size;

    size_t doc_buf_size;
    file_.read(reinterpret_cast<char*>(&doc_buf_size), sizeof(doc_buf_size));
    posting_list.doc_buffer_.resize(doc_buf_size);

    file_.read(reinterpret_cast<char*>(posting_list.doc_buffer_.data()),
               doc_buf_size);

    size_t coord_buf_size;
    file_.read(reinterpret_cast<char*>(&coord_buf_size),
               sizeof(coord_buf_size));
    posting_list.coord_buffer_.resize(coord_buf_size);

    file_.read(reinterpret_cast<char*>(posting_list.coord_buffer_.data()),
               coord_buf_size);

    index.index_[std::move(term)] = std::move(posting_list);
  }
  index.BuildSkips();

  size_t docs_count;
  file_.read(reinterpret_cast<char*>(&docs_count), sizeof(docs_count));
  index.doc_lengths_.resize(docs_count);
  file_.read(reinterpret_cast<char*>(index.doc_lengths_.data()),
             sizeof(uint32_t) * docs_count);

  file_.flush();
  file_.seekg(0, std::ios::beg);
  file_.seekp(0, std::ios::beg);

  return index;
}

}  // namespace storage
