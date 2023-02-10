//
// Created by Bach Dao.
//

#ifndef KV_HASH_KV_FLAT_MAP_H
#define KV_HASH_KV_FLAT_MAP_H

#include "kv_hash/utils.h"
#include <memory>

namespace kv_hash {
template <typename K, typename V, typename Hash, typename KeyEqual>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
class kv_flat_map {
  struct map_entry {
    // -1 : free
    // -2 : sentinel
    int8_t probeLen_ = -1;
    K key_;
    V val_;
  };
  std::unique_ptr<map_entry[]> ptrStorage_ = nullptr;
  size_t size_ = 1;

  size_t get_initial_index(const K &key);

  size_t probe(size_t initialIdx);

  template <typename Key, typename Val>
    requires std::is_same_v<Key, K> && std::is_same_v<Val, V>
  void emplace_at(Key &&key, Val &&val, size_t idx);

  void move_entry(size_t initialIdx);

public:
  template <typename Key, typename Val>
    requires std::is_same_v<Key, K> && std::is_same_v<Val, V>
  void insert(Key &&key, Val &&val);
};

template <typename K, typename V, typename Hash, typename KeyEqual>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
                                template <typename Key, typename Val>
             requires std::is_same_v<Key, K> && std::is_same_v<Val, V>
void kv_flat_map<K, V, Hash, KeyEqual>::insert(Key &&key, Val &&val) {
  auto initialIdx = get_initial_index(key);
  auto finalIdx = probe(initialIdx);
  return emplace_at(std::forward<Key>(key), std::forward<Val>(val), finalIdx);
}

template <typename K, typename V, typename Hash, typename KeyEqual>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
size_t kv_flat_map<K, V, Hash, KeyEqual>::probe(size_t initialIdx) {
  auto curProbeLen = 0;
  while (true) {
    auto probeLen = ptrStorage_[initialIdx + curProbeLen].probeLen_;
    if (curProbeLen > probeLen) {
      if (probeLen >= 0) { // this slot isn't empty
        move_entry(initialIdx);
      }
      break;
    }
    curProbeLen++;
  }
  return initialIdx + curProbeLen;
}

template <typename K, typename V, typename Hash, typename KeyEqual>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
size_t kv_flat_map<K, V, Hash, KeyEqual>::get_initial_index(const K &key) {
  // assume table has size is power-of-2
  auto hashVal = Hash{}(key);
  return hashVal & (size_ - 1);
}

template <typename K, typename V, typename Hash, typename KeyEqual>
  requires kv::HashFn<Hash, K> && kv::KeyEqualFn<KeyEqual, K>
                                template <typename Key, typename Val>
             requires std::is_same_v<Key, K> && std::is_same_v<Val, V>
void kv_flat_map<K, V, Hash, KeyEqual>::emplace_at(Key &&key, Val &&val,
                                                   size_t idx) {}
} // namespace kv_hash

#endif // KV_HASH_KV_FLAT_MAP_H
