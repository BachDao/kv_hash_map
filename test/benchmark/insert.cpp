//
// Created by Bach Dao.
//
#include "kv_hash/flat_hash_map.h"
#include <benchmark/benchmark.h>
#include <unordered_map>

static void std_unordered_map(benchmark::State &s) {
  std::unordered_map<std::string, int> map;
  for (auto _ : s) {
    auto round = s.range(0);
    for (int i = 0; i < round; ++i) {
      map.insert({std::to_string(i), i});
    }
  }
}
template <typename K, typename V>
using hash_map = kv::flat_hash_map<K, V, std::hash<K>, std::equal_to<K>,
                                   std::allocator<std::pair<K, V>>>;
static void kv_flat_map(benchmark::State &s) {
  hash_map<std::string, int> map;
  for (auto _ : s) {
    auto round = s.range(0);
    for (int i = 0; i < round; ++i) {
      map.insert({std::to_string(i), i});
    }
  }
}

static constexpr size_t TotalRound = 10000;

BENCHMARK(kv_flat_map)
    ->Unit(benchmark::kMillisecond)
    ->RangeMultiplier(2)
    ->Range(100, TotalRound);

BENCHMARK(std_unordered_map)
    ->Unit(benchmark::kMillisecond)
    ->RangeMultiplier(2)
    ->Range(100, TotalRound);
BENCHMARK_MAIN();
