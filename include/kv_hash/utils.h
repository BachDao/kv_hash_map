
#ifndef KV_HASH_UTILS_H
#define KV_HASH_UTILS_H
#include <concepts>
namespace kv {
template <typename Fn, typename Key>
concept HashFn = requires(Fn fn, Key key) {
                   Fn{};
                   { fn(key) } -> std::convertible_to<size_t>;
                 };

template <typename Fn, typename Key>
concept KeyEqualFn = requires(Fn fn, Key k1, Key k2) {
                     Fn{};
                     { fn(k1, k2) } -> std::convertible_to<bool>;
                   };
template <typename To, typename From> To to_integral(From v) {
  return static_cast<To>(v);
}

} // namespace kv
#endif // KV_HASH_UTILS_H
