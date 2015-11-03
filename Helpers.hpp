#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
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

// Quote a string
inline std::string quote(const std::string &str) {
  std::ostringstream buf;
  buf << "\"";
  for (char ch : str) {
    if (ch == '"' || ch == '\\')
      buf << '\\';
    buf << ch;
  }
  buf << "\"";
  return buf.str();
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

// Output a vector
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &values) {
  os << "[";
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i > 0)
      os << ",";
    os << values.at(i);
  }
  os << "]";
  return os;
}
}

#define HELPERS_HPP_DONE
#endif // #ifndef HELPERS_HPP
#ifndef HELPERS_HPP_DONE
#error "Cyclic include depencency"
#endif
