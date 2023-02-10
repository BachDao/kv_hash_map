//
// Created by Bach Dao.
//

#include "kv_hash/flat_hash_map.h"
#include <string>
#include <unordered_map>
template <typename K, typename V>
using hash_map = kv::flat_hash_map<K, V, std::hash<K>, std::equal_to<K>,
                                   std::allocator<std::pair<K, V>>>;
int main() {
  auto round = 1000000;
  hash_map<std::string, int> map;
  for (int i = 0; i < round; ++i) {
    map.insert({std::to_string(i), i});
  }
  return 0;
}
