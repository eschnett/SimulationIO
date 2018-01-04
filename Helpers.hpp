#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace SimulationIO {

// Integer exponentiation
template <typename T, typename U> inline T ipow(T base, U exp) {
  assert(exp >= 0);
  T res = 1;
  while (exp--)
    res *= base;
  return res;
}

// Indented output
const int indentsize = 2;
const char indentchar = ' ';
inline std::string indent(int level) {
  return std::string(level * indentsize, indentchar);
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

template <typename T, typename... Args>
std::unique_ptr<T> make_unique1(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Convert a vector
template <typename R, typename T>
std::vector<R> make_vector(const std::vector<T> &v) {
  std::vector<R> r(v.size());
  for (std::size_t i = 0; i < r.size(); ++i)
    r[i] = v[i];
  return r;
}

// Reverse a vector
template <typename T> void reverse(std::vector<T> &v) {
  std::reverse(v.begin(), v.end());
}
template <typename T> std::vector<T> reversed(std::vector<T> r) {
  reverse(r);
  return r;
}

// Insert an element into a map, ensuring that the key does not yet exist
template <typename Key, typename Value, typename Key1, typename Value1>
typename std::map<Key, Value>::iterator
checked_emplace(std::map<Key, Value> &m, Key1 &&key, Value1 &&value,
                const std::string &location, const std::string &entry) {
  auto res = m.insert(
      std::make_pair(std::forward<Key1>(key), std::forward<Value1>(value)));
  auto iter = std::move(res.first);
  auto did_insert = std::move(res.second);
  if (__builtin_expect(!did_insert, false)) {
    std::ostringstream buf;
    const auto &key2 = iter->first; // "key" was forwarded and is now empty
    buf << "Key \"" << key2 << "\" exists already in map \"" << entry
        << "\" of object \"" << location << "\"";
    throw std::domain_error(buf.str());
  }
  assert(did_insert);
  return iter;
}

// Extract keys from a map
template <typename Key, typename Value>
std::vector<Key> keys(const std::map<Key, Value> &m) {
  std::vector<Key> r;
  for (const auto &kv : m)
    r.push_back(kv.first);
  return r;
}

// Output a vector
namespace Output {
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
} // namespace Output
} // namespace SimulationIO

#define HELPERS_HPP_DONE
#endif // #ifndef HELPERS_HPP
#ifndef HELPERS_HPP_DONE
#error "Cyclic include depencency"
#endif
