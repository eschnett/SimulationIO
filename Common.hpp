#ifndef COMMON_HPP
#define COMMON_HPP

#include "Config.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#include <H5Cpp.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SimulationIO {

using std::function;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

// Concatenate two vectors
template <typename T> vector<T> concat(vector<T> x, const vector<T> &y) {
  vector<T> r(move(x));
  r.insert(r.end(), y.begin(), y.end());
  return r;
}

// C++ make_shared requires constructors to be public; we add a field of type
// `hidden` to ensure they are not called accidentally.
struct hidden {};

// Entity relationships

#if 0
// Parents and children; children are "owned" by their parents, and only created
// by their parent
template <typename Child> struct children { map<string, Child *> children; };
template <typename Parent> struct parent { Parent *parent; };

// Links that register with their target
template <typename Source> struct backlinks { map<string, Source *> sources; };
template <typename Target> struct link { Target *target; };

// Unidirectionsl links that don't register with their target
template <typename Source> struct nobacklinks {};
template <typename Target> struct unilink { Target *target; };
#endif

// A map holding weak pointers
// TODO: Move this into a file of its own
template <typename K, typename T> class weak_map {
  typedef std::map<K, std::weak_ptr<T>> map_type;
  map_type impl;

public:
  typedef K key_type;
  typedef std::shared_ptr<T> mapped_type;
  typedef std::pair<const K, std::shared_ptr<T>> value_type;
  typedef typename map_type::size_type size_type;
  typedef typename map_type::difference_type difference_type;

  weak_map() = default;
  weak_map(const weak_map &m) = default;
  weak_map(weak_map &&m) = default;
  weak_map &operator=(const weak_map &m) = default;
  weak_map &operator=(weak_map &&m) = default;

  std::shared_ptr<T> at(const K &k) const { return impl.at(k).lock(); }

  class iterator {
    typename map_type::iterator impl;

    iterator(const typename map_type::iterator &impl) noexcept : impl(impl) {}
    iterator(typename map_type::iterator &&impl) noexcept
        : impl(std::move(impl)) {}

  public:
    // Iterator
    iterator() = default; // extension
    iterator(const iterator &impl) = default;
    iterator(iterator &&impl) = default; // extension
    iterator operator=(const iterator &impl) = default;
    iterator operator=(iterator &&impl) = default; // extension
    void swap(iterator &iter) noexcept { std::swap(impl, iter.impl); }
    bool operator==(const iterator &iter) const { return impl == iter.impl; }

    // InputIterator
    bool operator!=(const iterator &iter) const { return impl != iter.impl; }
    std::pair<const K, std::shared_ptr<T>> operator*() const {
      return {impl->first, impl->second.lock()};
    }
    iterator &operator++() {
      ++impl;
      return *this;
    }
    iterator operator++(int) { return {impl++}; }
    // ForwardIterator
    // BidirectionalIterator
    iterator &operator--() {
      --impl;
      return *this;
    }
    iterator operator--(int) { return {impl--}; }
  };

  class const_iterator {
    typename map_type::const_iterator impl;

    const_iterator(const typename map_type::const_iterator &impl) noexcept
        : impl(impl) {}
    const_iterator(typename map_type::const_iterator &&impl) noexcept
        : impl(std::move(impl)) {}

  public:
    // Iterator
    const_iterator() = default; // extension
    const_iterator(const const_iterator &impl) = default;
    const_iterator(const_iterator &&impl) = default; // extension
    const_iterator operator=(const const_iterator &impl) = default;
    const_iterator operator=(const_iterator &&impl) = default; // extension
    void swap(const_iterator &iter) noexcept { std::swap(impl, iter.impl); }
    bool operator==(const const_iterator &iter) const {
      return impl == iter.impl;
    }

    // InputIterator
    bool operator!=(const const_iterator &iter) const {
      return impl != iter.impl;
    }
    std::pair<const K, const std::shared_ptr<T>> operator*() const {
      return {impl->first, impl->second.lock()};
    }
    const_iterator &operator++() {
      ++impl;
      return *this;
    }
    const_iterator operator++(int) { return {impl++}; }
    // ForwardIterator
    // BidirectionalIterator
    const_iterator &operator--() {
      --impl;
      return *this;
    }
    const_iterator operator--(int) { return {impl--}; }
  };

  iterator begin() noexcept { return iterator(impl.begin()); }
  const_iterator begin() const noexcept { return const_iterator(impl.begin()); }
  const_iterator cbegin() const noexcept { return begin(); }
  iterator end() noexcept { return iterator(impl.end()); }
  const_iterator end() const noexcept { return const_iterator(impl.end()); }
  const_iterator cend() const noexcept { return end(); }

  bool empty() const noexcept { return impl.empty(); }
  std::size_t size() const noexcept { return impl.size(); }
  std::size_t max_size() const noexcept { return impl.max_size(); }

  void clear() { impl.clear(); }
  std::pair<iterator, bool> insert(const value_type &value) {
    auto res =
        impl.insert(typename map_type::value_type(value.first, value.second));
    return {std::move(res.first), std::move(res.second)};
  }
  // template <typename... Args>
  // std::pair<iterator, bool> emplace(Args &&... args);
  void swap(weak_map &m) noexcept { impl.swap(m.impl); }

  size_type count(const K &k) const { return impl.count(k); }
  iterator find(const K &k) { return {impl.find(k)}; }
  const_iterator find(const K &k) const { return {impl.find(k)}; }
};
template <typename K, typename T>
void swap(weak_map<K, T> &x, weak_map<K, T> &y) noexcept {
  x.swap(y);
}
template <typename K, typename T>
void swap(typename weak_map<K, T>::const_iterator &x,
          typename weak_map<K, T>::const_iterator &y) noexcept {
  x.swap(y);
}

// An always empty pseudo-container type indicating that there is no
// back-link
template <typename T> struct NoBackLink {
  constexpr bool nobacklink() const noexcept { return true; }
};

// Common to all file elements

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
class asdf_writer_;
#endif

class Common {
protected:
  // TODO: Make m_name private, provide an HDF5 read routine, handle the type
  // attribute there as well
  string m_name;

public:
  string name() const { return m_name; }
  virtual string type() const = 0;

  virtual bool invariant() const { return !name().empty(); }

protected:
  Common(const string &name) : m_name(name) {}
  Common(hidden) {}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  const asdf_writer_ asdf_writer(ASDF::writer &w) const;
#endif

public:
  virtual ~Common() {}
  virtual ostream &output(ostream &os, int level = 0) const = 0;
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const = 0;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  // virtual string yaml_alias() const = 0;
  virtual vector<string> yaml_path() const = 0;
#endif

  // The association between names and integer values below MUST NOT BE
  // MODIFIED, except that new integer values may be added.
  enum types {
    type_Basis = 1,
    type_BasisVector = 2,
    type_DiscreteField = 3,
    type_DiscreteFieldBlock = 4,
    type_DiscreteFieldBlockComponent = 5,
    type_Discretization = 6,
    type_DiscretizationBlock = 7,
    type_Field = 8,
    type_Manifold = 9,
    type_Project = 10,
    type_TangentSpace = 11,
    type_TensorComponent = 12,
    type_TensorType = 13,
    type_Configuration = 14,
    type_Parameter = 15,
    type_ParameterValue = 16,
    type_CoordinateSystem = 17,
    type_CoordinateField = 18,
    type_SubDiscretization = 19,
  };
};

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
// string quote_alias(const string &alias);

class asdf_writer_ {
  const Common &m_common;
  ASDF::writer &m_writer;

public:
  asdf_writer_() = delete;
  asdf_writer_(const Common &common, ASDF::writer &w);

  ~asdf_writer_();

  template <typename T> void value(const string &name, const T &value) {
    m_writer << YAML::Key << name << YAML::Value << value;
  }

  template <typename T> void alias(const string &name, const T &value) {
    m_writer << YAML::Key << name << YAML::Value
             << ASDF::reference("", value.yaml_path());
  }

  template <typename K, typename T>
  void group(const string &name, const map<K, shared_ptr<T>> &values) {
    m_writer << YAML::Key << name << YAML::Value;
    m_writer << YAML::BeginMap;
    for (const auto &kv : values)
      m_writer << YAML::Key << kv.first << YAML::Value << *kv.second;
    m_writer << YAML::EndMap;
  }

  template <typename K, typename T>
  void alias_group(const string &name, const map<K, shared_ptr<T>> &values) {
    m_writer << YAML::Key << name << YAML::Value;
    m_writer << YAML::BeginMap;
    for (const auto &kv : values)
      m_writer << YAML::Key << kv.first << YAML::Value
               << ASDF::reference("", kv.second->yaml_path());
    m_writer << YAML::EndMap;
  }

  template <typename T>
  void short_sequence(const string &name, const vector<T> &values) {
    m_writer << YAML::Key << name << YAML::Value << YAML::Flow << values;
  }
};
#endif

} // namespace SimulationIO

#endif // #ifndef COMMON_HPP
