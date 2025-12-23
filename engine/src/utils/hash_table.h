#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace utils {

inline uint64_t Mix64(uint64_t x) {
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  x ^= x >> 33;
  return x;
}

struct StringHasher {
  uint64_t operator()(std::string_view s) const {
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : s) {
      h ^= c;
      h *= 1099511628211ULL;
    }
    return Mix64(h);
  }
};

template <typename T>
class HashTable {
 public:
  using key_type = std::string;
  using mapped_type = T;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using hasher = StringHasher;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

 private:
  using Bucket = std::list<value_type>;
  using BucketArray = std::vector<Bucket>;
  using BucketIterator = typename BucketArray::iterator;
  using ConstBucketIterator = typename BucketArray::const_iterator;
  using ListIterator = typename Bucket::iterator;
  using ConstListIterator = typename Bucket::const_iterator;

  template <bool IsConst>
  class IteratorImpl {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename HashTable::value_type;
    using difference_type = typename HashTable::difference_type;
    using pointer = typename std::conditional<IsConst, const value_type*,
                                              value_type*>::type;
    using reference = typename std::conditional<IsConst, const value_type&,
                                                value_type&>::type;
    using BucketIter = typename std::conditional<IsConst, ConstBucketIterator,
                                                 BucketIterator>::type;
    using ListIter = typename std::conditional<IsConst, ConstListIterator,
                                               ListIterator>::type;

    IteratorImpl() = default;

    IteratorImpl(BucketIter bucket_it, BucketIter bucket_end, ListIter list_it)
        : bucket_it_(bucket_it), bucket_end_(bucket_end), list_it_(list_it) {}

    template <bool WasConst,
              typename = typename std::enable_if<IsConst && !WasConst>::type>
    IteratorImpl(const IteratorImpl<WasConst>& other)
        : bucket_it_(other.bucket_it_),
          bucket_end_(other.bucket_end_),
          list_it_(other.list_it_) {}

    reference operator*() const { return *list_it_; }
    pointer operator->() const { return &(*list_it_); }

    IteratorImpl& operator++() {
      ++list_it_;
      if (list_it_ == bucket_it_->end()) {
        do {
          ++bucket_it_;
        } while (bucket_it_ != bucket_end_ && bucket_it_->empty());

        if (bucket_it_ != bucket_end_) {
          list_it_ = bucket_it_->begin();
        }
      }
      return *this;
    }

    IteratorImpl operator++(int) {
      IteratorImpl temp = *this;
      ++(*this);
      return temp;
    }

    bool operator==(const IteratorImpl& other) const {
      if (bucket_it_ == bucket_end_ && other.bucket_it_ == other.bucket_end_) {
        return true;
      }
      return bucket_it_ == other.bucket_it_ && list_it_ == other.list_it_;
    }

    bool operator!=(const IteratorImpl& other) const {
      return !(*this == other);
    }

   private:
    friend class HashTable;
    template <bool B>
    friend class IteratorImpl;

    BucketIter bucket_it_;
    BucketIter bucket_end_;
    ListIter list_it_;
  };

 public:
  using iterator = IteratorImpl<false>;
  using const_iterator = IteratorImpl<true>;

  explicit HashTable(size_type bucket_count = 8)
      : buckets_(bucket_count > 0 ? bucket_count : 1),
        size_(0),
        max_load_factor_(1.0f) {}

  std::pair<iterator, bool> insert(const value_type& value) {
    auto it = find(value.first);
    if (it != end()) {
      return {it, false};
    }
    CheckRehash(size_ + 1);
    size_type idx = GetIndex(value.first);
    buckets_[idx].push_front(value);
    ++size_;
    return {
        iterator(buckets_.begin() + idx, buckets_.end(), buckets_[idx].begin()),
        true};
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    auto it = find(value.first);
    if (it != end()) {
      return {it, false};
    }
    CheckRehash(size_ + 1);
    size_type idx = GetIndex(value.first);
    buckets_[idx].push_front(std::move(value));
    ++size_;
    return {
        iterator(buckets_.begin() + idx, buckets_.end(), buckets_[idx].begin()),
        true};
  }

  mapped_type& operator[](const key_type& key) {
    auto it = find(key);
    if (it == end()) {
      it = insert({key, mapped_type()}).first;
    }
    return it->second;
  }

  mapped_type& at(const key_type& key) {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("HashTable::at: key not found");
    }
    return it->second;
  }

  const mapped_type& at(const key_type& key) const {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("HashTable::at: key not found");
    }
    return it->second;
  }

  size_type erase(const key_type& key) {
    size_type idx = GetIndex(key);
    auto& bucket = buckets_[idx];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
      if (it->first == key) {
        bucket.erase(it);
        --size_;
        return 1;
      }
    }
    return 0;
  }

  void clear() {
    for (auto& bucket : buckets_) {
      bucket.clear();
    }
    size_ = 0;
  }

  iterator find(const key_type& key) {
    size_type idx = GetIndex(key);
    auto& bucket = buckets_[idx];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
      if (it->first == key) {
        return iterator(buckets_.begin() + idx, buckets_.end(), it);
      }
    }
    return end();
  }

  const_iterator find(const key_type& key) const {
    size_type idx = GetIndex(key);
    const auto& bucket = buckets_[idx];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
      if (it->first == key) {
        return const_iterator(buckets_.begin() + idx, buckets_.end(), it);
      }
    }
    return end();
  }

  size_type count(const key_type& key) const {
    return find(key) != end() ? 1 : 0;
  }

  iterator begin() {
    auto it = buckets_.begin();
    while (it != buckets_.end() && it->empty()) {
      ++it;
    }
    if (it == buckets_.end()) {
      return end();
    }
    return iterator(it, buckets_.end(), it->begin());
  }

  const_iterator begin() const {
    auto it = buckets_.begin();
    while (it != buckets_.end() && it->empty()) {
      ++it;
    }
    if (it == buckets_.end()) {
      return end();
    }
    return const_iterator(it, buckets_.end(), it->begin());
  }

  const_iterator cbegin() const { return begin(); }

  iterator end() { return iterator(buckets_.end(), buckets_.end(), {}); }

  const_iterator end() const {
    return const_iterator(buckets_.end(), buckets_.end(), {});
  }

  const_iterator cend() const { return end(); }

  bool empty() const { return size_ == 0; }
  size_type size() const { return size_; }
  size_type max_size() const { return buckets_.max_size(); }
  size_type bucket_count() const { return buckets_.size(); }
  float load_factor() const {
    return static_cast<float>(size_) / buckets_.size();
  }

  void max_load_factor(float mlf) { max_load_factor_ = mlf; }
  float max_load_factor() const { return max_load_factor_; }

  void rehash(size_type count) {
    size_type min_size = static_cast<size_type>(size_ / max_load_factor_);
    size_type new_count = std::max(count, min_size);
    new_count = std::max(new_count, size_type(1));

    BucketArray new_buckets(new_count);
    auto old_buckets = std::move(buckets_);
    buckets_ = std::move(new_buckets);
    size_ = 0;

    for (auto& bucket : old_buckets) {
      for (auto& node : bucket) {
        size_type idx = GetIndex(node.first);
        buckets_[idx].push_front(std::move(node));
        ++size_;
      }
    }
  }

  void reserve(size_type count) {
    rehash(static_cast<size_type>(std::ceil(count / max_load_factor_)));
  }

 private:
  BucketArray buckets_;
  size_type size_;
  float max_load_factor_;
  hasher hash_function_;

  size_type GetIndex(const key_type& key) const {
    return hash_function_(key) % buckets_.size();
  }

  void CheckRehash(size_type needed_size) {
    if (static_cast<float>(needed_size) / buckets_.size() > max_load_factor_) {
      rehash(buckets_.size() * 2);
    }
  }
};

}  // namespace utils
