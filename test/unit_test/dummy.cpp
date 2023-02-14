//
// Created by Bach Dao.
//
#include "kv_hash/flat_hash_map.h"
#include "gtest/gtest.h"
template <typename K, typename V>
using hash_map = kv::flat_hash_map<K, V, std::hash<K>, std::equal_to<K>,
                                   std::allocator<std::pair<K, V>>>;

TEST(FLAT_MAP, INSERT) {
  hash_map<int, int> map{16};
  size_t round = 100;

  for (int i = 0; i < round; ++i) {
    map.insert({(i), i});
  }

  for (int i = 0; i < round; ++i) {
    auto it = map.find((i));
    assert(it != map.end());
    assert((*it).second == i);
  }
}
