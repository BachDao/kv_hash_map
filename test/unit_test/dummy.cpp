//
// Created by Bach Dao.
//
#include "kv_hash/flat_hash_map.h"
#include "gtest/gtest.h"
template <typename K, typename V>
using hash_map = kv::flat_hash_map<K, V, std::hash<K>, std::equal_to<K>,
                                   std::allocator<std::pair<K, V>>>;

TEST(FLAT_MAP, INSERT) {
  hash_map<std::string, int> map;

  for (int i = 0; i < 64; ++i) {
    map.insert({std::to_string(i), i});
  }
  for (int i = 0; i < 64; ++i) {
    auto item = map.find(std::to_string(i));
    EXPECT_TRUE(item != map.end());
    EXPECT_TRUE(item->second == i);
  }
}