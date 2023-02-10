//
// Created by Bach Dao.
//

#ifndef KV_HASH_KV_LINEAR_FLAT_MAP_H
#define KV_HASH_KV_LINEAR_FLAT_MAP_H
#include "kv_hash/utils.h"
#include <memory>
namespace kv {

namespace detail {
template <typename T> struct entry {
  bool isEmpty_;
  entry(const T &val) : isEmpty_(false), val_(val) {}
  entry(T &&val) : isEmpty_(false), val_(std::move(val)) {}
  entry() : isEmpty_(true){};
  union {
    T val_;
  };
};
} // namespace detail
template <typename K, typename V> class map_iterator {
  detail::entry<std::pair<K, V>> *ptrStorage_;
  size_t offset_;

public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::pair<K, V>;
  using pointer = value_type *;
  using reference = value_type &;
  map_iterator(value_type *ptr, size_t offset)
      : ptrStorage_(ptr), offset_(offset) {}
  reference operator*() const { return ptrStorage_[offset_].val_; }
  pointer operator->() { return &ptrStorage_[offset_].val_; }

  map_iterator &operator++() {
    offset_++;
    return *this;
  }
  map_iterator &operator--() {
    offset_--;
    return this;
  }
  friend bool operator==(const map_iterator &lhs, const map_iterator &rhs) {
    return lhs.ptrStorage_ == rhs.ptrStorage_ && lhs.offset_ == rhs.offset_;
  }
  friend bool operator!=(const map_iterator &lhs, const map_iterator &rhs) {
    return !(lhs == rhs);
  }
};

template <typename K, typename V, typename Hash, typename KeyEqual,
          typename Alloc>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
class kv_linear_flat_map {
public:
  using value_type = std::pair<K, V>;
  using iterator = map_iterator<K, V>;
  friend class map_iterator<K, V>;

private:
  using entry = detail::entry<value_type>;

  using alloc_type =
      typename std::allocator_traits<Alloc>::template rebind_alloc<entry>;
  using alloc_traits = std::allocator_traits<alloc_type>;
  std::vector<entry, alloc_type> storage_;
  size_t capacity_ = 0;
  size_t maxProbeLen_ = 0;
  alloc_type alloc_;

  size_t initial_index(const K &key) {
    auto hashVal = Hash{}(key);
    return hashVal % capacity_;
  }
  bool is_empty(size_t idx) {
    auto &e = storage_[idx];
    return e.isEmpty_;
  }
  template <typename ValType = value_type>
  value_type &construct_in_place(size_t idx, ValType &&val) {
    auto ptrEntry = &storage_[idx];
    alloc_traits::construct(alloc_, ptrEntry, std::forward<ValType>(val));
    return ptrEntry->val_;
  }
  void resize();
  iterator iterator_at(size_t idx) { return {storage_.data(), idx}; }
  entry &entry_at(size_t idx) { return storage_[idx].val_; }

  // return slot that contain a "key" or first empty slot
  std::pair<size_t, bool> find_slot(size_t initialIdx, const K &key) {
    auto probeLen = 0;
    while (true) {
      auto curIdx = initialIdx + probeLen;
      if (is_empty(curIdx)) {
        return {curIdx, false};
      }
      auto &e = entry_at(curIdx);
      if (KeyEqual{}(key, e.val_.first)) {
        return {curIdx, true};
      }
      probeLen++;
      if (probeLen > maxProbeLen_) {
        resize();
      }
    }
  }

public:
  kv_linear_flat_map(size_t initialSize = 32, size_t maxProbeLen = 0)
      : capacity_(initialSize), storage_(initialSize),
        maxProbeLen_(maxProbeLen) {
    if (maxProbeLen_ == 0 || maxProbeLen_ > capacity_) {
      maxProbeLen_ = capacity_ / 4;
    }
  }
  iterator begin();
  iterator end();

  iterator insert(const value_type &val) {
    auto idx = initial_index(val.first);
    auto findResult = find_slot(idx, val.first);
    if (findResult.second) { // key already exist
      return iterator_at(findResult.first);
    }
    construct_in_place(findResult.first, val);
  }
  std::pair<iterator, bool> insert(value_type &&val);

  iterator find(const K &key) {
    auto idx = initial_index(key);
    auto result = find_slot(idx, key);
    if (result.second) {
      return iterator_at(result.first);
    }
    return end();
  }
};

} // namespace kv
#endif // KV_HASH_KV_LINEAR_FLAT_MAP_H
