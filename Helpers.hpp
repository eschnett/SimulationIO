#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace SimulationIO {

// Integer exponentiation
inline int ipow(int base, int exp) {
  assert(base >= 0 && exp >= 0);
  int res = 1;
  while (exp--)
    res *= base;
  return res;
}

// Convert a weak_ptr to a shared_ptr, deducing the element type
template <typename T> std::shared_ptr<T> shared(const std::weak_ptr<T> &x) {
  return std::shared_ptr<T>(x);
}

// Insert an element into a map, ensuring that the key does not yet exist
template <typename Key, typename Value, typename Key1, typename Value1>
typename std::map<Key, Value>::iterator
checked_emplace(std::map<Key, Value> &m, Key1 &&key, Value1 &&value) {
  auto res = m.insert(
      std::make_pair(std::forward<Key1>(key), std::forward<Value1>(value)));
  auto iter = std::move(res.first);
  auto did_insert = std::move(res.second);
  assert(did_insert);
  return iter;
}

// Indented output
const int indentsize = 2;
const char indentchar = ' ';
inline std::string indent(int level) {
  return std::string(level * indentsize, indentchar);
}
}

#define HELPERS_HPP_DONE
#endif // #ifndef HELPERS_HPP
#ifndef HELPERS_HPP_DONE
#error "Cyclic include depencency"
#endif
