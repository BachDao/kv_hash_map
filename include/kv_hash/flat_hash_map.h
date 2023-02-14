//
// Created by Bach Dao.
//

#ifndef KV_HASH_FLAT_HASH_MAP_H
#define KV_HASH_FLAT_HASH_MAP_H

#include "kv_hash/utils.h"
#include <cassert>
#include <exception>
#include <memory>
#include <span>

namespace kv {

#define KV_DEBUG

enum class map_entry_state : size_t {
  empty,
  full,
  deleted,
  invalid,
};

size_t mod(size_t a, size_t n) { return a % n; }

template <typename K, typename V> struct map_entry {
  map_entry_state state_;
  std::pair<K, V> val_;
  map_entry(const std::pair<K, V> &val)
      : state_(map_entry_state::full), val_(val) {}
};

template <typename EntryType, typename Alloc, typename Hash> class storage {
  using alloc_type =
      typename std::allocator_traits<Alloc>::template rebind_alloc<EntryType>;
  using alloc_traits = std::allocator_traits<alloc_type>;

  size_t capacity_;
  EntryType *ptrData_;
  alloc_type alloc_;

public:
  storage(size_t capacity) : capacity_(capacity) {
    ptrData_ = alloc_traits::allocate(alloc_, capacity_);
  }

  ~storage() {
    if (ptrData_ != nullptr)
      alloc_traits::deallocate(alloc_, ptrData_, capacity_);
  }

  storage(storage &&rhs) {
    if (ptrData_)
      alloc_traits::deallocate(alloc_, ptrData_, capacity_);

    capacity_ = rhs.capacity_;
    ptrData_ = rhs.ptrData_;
    rhs.ptrData_ = nullptr;
    rhs.capacity_ = 0;
  }
  storage &operator=(storage &&rhs) {}

  EntryType &operator[](size_t idx) {
    assert(idx < capacity_ && idx >= 0 && "index out of range");
    return ptrData_[idx];
  }

  std::tuple<size_t, map_entry_state, size_t>
  probe_impl(size_t initialIdx, std::function<bool(EntryType &)> pred,
             size_t maxProbeLen, EntryType *ptrData, size_t capacity) {
    auto probeLen = 0;
    auto idx = mod(initialIdx, capacity);
    while (probeLen <= maxProbeLen) {
      auto &e = ptrData[idx];
      if (pred(e))
        return {idx, e.state_, probeLen};
      probeLen++;
      idx = mod(initialIdx + probeLen, capacity);
    }
    return {idx, map_entry_state::invalid, probeLen};
  }

  auto probe(size_t initialIdx, std::function<bool(EntryType &)> pred,
             size_t maxProbeLen) {
    return probe_impl(initialIdx, pred, maxProbeLen, ptrData_, capacity_);
  }

  std::tuple<size_t, size_t, size_t> resize(size_t newCap) {
    auto newSt = alloc_traits::allocate(alloc_, newCap);
    memset(newSt, 0, sizeof(EntryType) * newCap);

    auto pred = [](EntryType &e) { return e.state_ == map_entry_state::empty; };
    auto maxProbeLen = 0;
    auto beginIdx = SIZE_T_MAX;
    auto endIdx = 0;

    for (int i = 0; i < capacity_; ++i) {
      auto &entry = ptrData_[i];
      if (entry.state_ != map_entry_state::full)
        continue;
      auto key = entry.val_.first;
      auto hashVal = Hash{}(key);

      auto [idx, _, probeLen] =
          probe_impl(hashVal, pred, SIZE_T_MAX, newSt, newCap);
      if (probeLen > maxProbeLen)
        maxProbeLen = probeLen;
      if (idx < beginIdx)
        beginIdx = idx;
      if (idx > endIdx)
        endIdx = idx;
      alloc_traits::construct(alloc_, &newSt[idx], std::move(entry));
    }

    alloc_traits::deallocate(alloc_, ptrData_, capacity_);

    capacity_ = newCap;
    ptrData_ = newSt;

    return {maxProbeLen, beginIdx, endIdx};
  }
  size_t capacity() const { return capacity_; }
  void emplace(size_t idx, const EntryType &e) {
    alloc_traits::construct(alloc_, &ptrData_[idx], e);
  }
};

template <typename HashMap, typename K, typename V> class map_iterator {
  size_t idx_;
  HashMap &hm_;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<K, V>;
  using reference_type = value_type &;
  using pointer_type = value_type *;
  using difference_type = std::ptrdiff_t;

  map_iterator(HashMap &hm, size_t idx) : hm_(hm), idx_(idx) {}

  value_type &operator*() { return hm_.entry_at(idx_).val_; }

  map_iterator &operator->() { return *this; }

  map_iterator &operator++() {
    idx_ = hm_.get_next_entry_idx(idx_);
    return *this;
  }

  map_iterator operator++(int) {
    auto prevIdx = idx_;
    idx_ = hm_.get_next_entry_idx(idx_);
    return {hm_, prevIdx};
  }

  friend bool operator==(const map_iterator &lhs, const map_iterator &rhs) {
    return &lhs.hm_ == &rhs.hm_ && lhs.idx_ == rhs.idx_;
  }

  friend bool operator!=(const map_iterator &lhs, const map_iterator &rhs) {
    return !(lhs == rhs);
  }
};

template <typename K, typename V, typename Hash, typename KeyEqual,
          typename Alloc>
class flat_hash_map {
public:
  using value_type = std::pair<K, V>;
  using reference_type = value_type &;
  using iterator = map_iterator<flat_hash_map, K, V>;

private:
  friend class map_iterator<flat_hash_map, K, V>;
  using map_entry = map_entry<K, V>;
  size_t beginIdx_ = SIZE_T_MAX;
  size_t endIdx_ = 0;

  size_t size_ = 0;
  size_t maxProbeLen_ = 0;
  static constexpr double maxLoadFactor_ = 0.8;
  storage<map_entry, Alloc, Hash> storage_;

  void update_boundary(size_t idx) {
    if (beginIdx_ > idx)
      beginIdx_ = idx;
    if (endIdx_ < idx)
      endIdx_ = idx;
  }

  size_t normalize_index(size_t idx, size_t cap) { return idx % cap; }

  iterator iterator_at(size_t idx) { return iterator(*this, idx); }

  map_entry &entry_at(size_t idx) { return storage_[idx]; }

  std::pair<size_t, map_entry_state> probe(size_t initialIdx, const K &key) {
    auto pred = [&](map_entry &e) {
      return e.state_ == map_entry_state::empty ||
             KeyEqual{}(key, e.val_.first);
    };
    auto [idx, entryState, probeLen] =
        storage_.probe(initialIdx, pred, maxProbeLen_);
    return {idx, entryState};
  }

  std::tuple<size_t, map_entry_state, size_t> find_empty_slot(size_t initialIdx,
                                                              const K &key) {
    auto pred = [&](map_entry &e) {
      return e.state_ == map_entry_state::empty;
    };
    return storage_.probe(initialIdx, pred, storage_.capacity());
  }

  std::pair<size_t, bool> find_or_prepare_insert(size_t hashVal, const K &key) {
    auto [idx, entryState] = probe(hashVal, key);

    // entry doesn't exist in table
    if (entryState == map_entry_state::full) {
      return {idx, true};
    }
    if (entryState == map_entry_state::empty) {
      return {idx, false};
    }
    assert(entryState == map_entry_state::invalid);
    {
      // start from last tried index
      auto [emptyIdx, _, probeLen] =
          find_empty_slot(hashVal + maxProbeLen_, key);
      assert(entry_at(emptyIdx).state_ == map_entry_state::empty);
      maxProbeLen_ += probeLen;
      return {emptyIdx, false};
    }
  }

  // construct entry from "val" at "idx"
  iterator emplace(size_t idx, const value_type &val) {
    storage_.emplace(idx, val);
    size_++;
    update_boundary(idx);
    return iterator_at(idx);
  }
  void resize_if_necessary() {
    auto capacity = storage_.capacity();
    auto loadFactor = (double)size_ / (double)capacity;
    if (loadFactor > maxLoadFactor_) {
      auto newCap = capacity * 2;
      auto [maxProbeLen, beginIdx, endIdx] = storage_.resize(newCap);
      maxProbeLen_ = maxProbeLen;
      beginIdx_ = beginIdx;
      endIdx_ = endIdx;
    }
  }
  size_t get_next_entry_index(size_t curIdx) {
    auto pred = [](map_entry &e) { return e.state_ == map_entry_state::full; };
    auto [idx, entryState, _] =
        storage_.probe(curIdx, pred, storage_.capacity());
    if (entryState == map_entry_state::full) {
      return idx;
    }
    return endIdx_ + 1;
  }

public:
  flat_hash_map(size_t capacity = 32) : storage_(capacity) {
    maxProbeLen_ = capacity / 4;
  }

  std::pair<iterator, bool> insert(const value_type &val) {
    resize_if_necessary();
    auto hashVal = Hash{}(val.first);
    auto [idx, exist] = find_or_prepare_insert(hashVal, val.first);
    if (exist) {
      return {iterator_at(idx), false};
    }
    return {emplace(idx, val), true};
  }

  iterator find(const K &key) {
    auto hashVal = Hash{}(key);
    auto pred = [&](map_entry &e) { return KeyEqual{}(e.val_.first, key); };
    auto [idx, entryState, probeLen] =
        storage_.probe(hashVal, pred, maxProbeLen_);
    if (entryState == map_entry_state::invalid)
      return end();
    return iterator_at(idx);
  }
  iterator begin() { return {*this, beginIdx_}; }
  iterator end() { return {*this, endIdx_ + 1}; }
};

} // namespace kv
#endif // KV_HASH_FLAT_HASH_MAP_H
