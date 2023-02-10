//
// Created by Bach Dao.
//

#ifndef KV_HASH_FLAT_HASH_MAP_H
#define KV_HASH_FLAT_HASH_MAP_H

#include <exception>
#include <memory>
namespace kv {
namespace detail {

enum class entry_state : int8_t { empty, full, deleted };

template <typename K, typename V> struct map_entry {
  using value_type = std::pair<K, V>;
  entry_state state_;
  union {
    std::pair<K, V> val_;
  };
  map_entry(entry_state state, const std::pair<K, V> &val)
      : state_(state), val_(val) {}
  map_entry() : state_(entry_state::empty) {}
  ~map_entry() {}
};

} // namespace detail

template <typename K, typename V, typename Hash, typename KeyEqual,
          typename Alloc>
class flat_hash_map {
public:
  class iterator {
    flat_hash_map &map_;
    size_t offset_;

    detail::map_entry<K, V> &entry_at(size_t offset) const {
      if (offset == map_.capacity_)
        throw std::logic_error("out of bound");
      return map_.storage_[offset];
    }

  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<K, V>;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(flat_hash_map &map, size_t offset) : map_(map), offset_(offset) {}

    reference operator*() const { return entry_at(offset_).val_; }

    pointer operator->() { return &entry_at(offset_).val_; }

    iterator &operator++() {
      while (true) {
        offset_++;
        if (offset_ == map_.end_) {
          return *this;
        }
        if (entry_at(offset_).state_ == detail::entry_state::full) {
          return *this;
        }
      }
    }
    iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    iterator &operator--() {
      while (true) {
        if (offset_ == 0) {
          return map_.begin();
        }
        offset_--;
        if (entry_at(offset_).state_ == detail::entry_state::full) {
          return this;
        }
      }
    }
    iterator operator--(int) {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend bool operator==(const iterator &lhs, const iterator &rhs) {
      return &lhs.map_ == &rhs.map_ && lhs.offset_ == rhs.offset_;
    }

    friend bool operator!=(const iterator &lhs, const iterator &rhs) {
      return !(lhs == rhs);
    }
  };
  using value_type = typename detail::map_entry<K, V>::value_type;

private:
  using alloc_type = typename std::allocator_traits<
      Alloc>::template rebind_alloc<detail::map_entry<K, V>>;
  using alloc_traits = std::allocator_traits<alloc_type>;
  using entry_type = detail::map_entry<K, V>;

  alloc_type alloc_;
  size_t capacity_;
  size_t size_ = 0;
  size_t maxProbeLen_;
  static constexpr size_t probeLenRatio_ = 4;
  size_t begin_ = SIZE_T_MAX;
  size_t end_ = 0;
  std::vector<entry_type, alloc_type> storage_;

  auto get_hash(const K &key) { return Hash{}(key); }
  size_t get_real_index(size_t idx) { return idx % capacity_; }
  size_t initial_index(size_t hashVal) { return hashVal % capacity_; }

  bool is_empty(size_t idx) {
    auto &e = storage_[idx];
    return e.state_ == detail::entry_state::empty;
  }

  template <typename ValType = value_type>
  iterator construct_in_place(size_t idx, ValType &&val) {
    alloc_traits::construct(alloc_, &entry_at(idx), detail::entry_state::full,
                            std::forward<ValType>(val));
    return iterator_at(idx);
  }

  void resize() {
    capacity_ *= 2;
    flat_hash_map newMap(capacity_);
    for (auto it = begin(); it != end(); it++) {
      newMap.insert(*it);
    }
    storage_ = std::move(newMap.storage_);
    begin_ = newMap.begin_;
    end_ = newMap.end_;
  }

  iterator iterator_at(size_t idx) { return {*this, idx}; }

  entry_type &entry_at(size_t idx) { return storage_[idx]; }

  // return slot that contain a "key" or first empty slot
  std::pair<size_t, bool> find_slot(size_t hashVal, const K &key) {
    auto probeLen = 0;
    auto initialIdx = get_real_index(hashVal);
    while (true) {
      auto curIdx = get_real_index(initialIdx + probeLen);
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
        initialIdx = get_real_index(hashVal);
        probeLen = 0;
      }
    }
  }
  void update_boundary(size_t idx) {
    if (idx < begin_)
      begin_ = idx;
    if (idx >= end_)
      end_ = idx + 1;
  }

public:
  flat_hash_map(size_t capacity = 32, size_t probeLen = 0)
      : capacity_(capacity), storage_(capacity_), maxProbeLen_(probeLen) {
    if (maxProbeLen_ == 0) {
      maxProbeLen_ = capacity_ / probeLenRatio_;
    }
  }

  std::pair<iterator, bool> insert(const value_type &val) {
    auto hashVal = get_hash(val.first);
    auto findResult = find_slot(hashVal, val.first);
    if (findResult.second) { // key already exist
      return {iterator_at(findResult.first), false};
    }
    update_boundary(findResult.first);
    size_++;
    return {construct_in_place(findResult.first, val), true};
  }
  iterator find(const K &key) {
    auto hashVal = get_hash(key);
    auto findResult = find_slot(hashVal, key);
    if (findResult.second) {
      return iterator_at(findResult.first);
    }
    return end();
  }
  iterator begin() {
    if (begin_ == SIZE_T_MAX)
      return end();
    return {*this, begin_};
  }
  iterator end() { return {*this, end_}; }
};
} // namespace kv
#endif // KV_HASH_FLAT_HASH_MAP_H
