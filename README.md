## Hashmap experimental

This repo contain my implementation of traditional hash-table with different hash collision strategy, include:

- Normal linear probing
- Robinhood probing
- SIMD-based probing

For each implementation, I compare performance of basic operation (insert, find, ..) with std::unordered_map
(as base case), and with other implementation, read on for more detail.

General information about hash-table and hash collision resolve can be found
at [HashTable](https://en.wikipedia.org/wiki/Hash_table)

<br />

#### Normal linear probing

- **Key** and **Value** are store in continuous array, with other information if necessary (Key's hashed value, probe
  length, ...)
- Hash collision is resolved by keep searching for next empty slot.
- Table is resize when load factor is exceed predefine max load factor, or when probe length is too big. <br />
  
![image info](https://github.com/BachDao/kv_hash_map/blob/main/resoure/flat_map_insertion.png)
![image info](https://github.com/BachDao/kv_hash_map/blob/main/resoure/flat_map_probe_length.png)