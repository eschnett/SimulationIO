#ifndef H5HELPERS_HPP
#define H5HELPERS_HPP

// HDF5 helpers

#include "Helpers.hpp"

#include <H5Cpp.h>

#include <algorithm>
#include <cassert>
#include <complex>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if !H5_VERSION_GE(1, 10, 1)
#error                                                                         \
    "The HDF5 C++ API changed significantly between versions 1.10.0-patch1 and 1.10.1. This code requires at least version 1.10.1 of HDF5."
#endif

namespace H5 {

namespace detail {
template <typename T> struct is_vector : std::false_type {};
template <typename T, typename Allocator>
struct is_vector<std::vector<T, Allocator>> : std::true_type {};
} // namespace detail

// Wrapper for hid_t that ensures correct HDF5 reference counting
class hid {
  hid_t m_id;
  void incref() {
    if (valid())
      H5Iinc_ref(m_id);
  }
  void decref() {
    if (valid())
      H5Idec_ref(m_id);
  }

public:
  hid() : m_id(-1) {}
  // hid(hid_t id) : m_id(id) { incref(); }
  hid(hid_t &&id) : m_id(id) { /* no incref */
  }
  hid(const hid &other);
  hid(hid &&other);
  ~hid();
  hid &operator=(const hid &other);
  hid &operator=(hid &&other);
  bool valid() const { return m_id >= 0; }
  hid_t get() const { return m_id; }
  operator hid_t() const { return get(); }
};
// TOOD: phase out this function
inline hid take_hid(hid_t id) { return hid(std::move(id)); }

// H5Literate
namespace detail {
template <typename Op> struct H5L_iterator {
  Op &&op;
  static herr_t call(hid_t g_id, const char *name, const H5L_info_t *info,
                     void *data) {
    return static_cast<H5L_iterator *>(data)->op(Group(g_id), name, info);
  }
  herr_t operator()(const H5Location &loc, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx) {
    auto iret = H5Literate(loc.getId(), index_type, order, idx, call, this);
    assert(iret >= 0);
    return iret;
  }
};
} // namespace detail

template <typename Op>
herr_t iterateElems(const H5Location &loc, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx, Op &&op) {
  auto iret = detail::H5L_iterator<Op>{std::forward<Op>(op)}(loc, index_type,
                                                             order, idx);
  assert(iret >= 0);
  return iret;
}

// Convert a HDF5 datatype class to a string
std::string className(H5T_class_t cls);

// Describe the structure of an HDF5 datatype
std::string dump(const DataType &type);

// Get HDF5 datatype from C++ type

inline IntType getType(const char &) { return IntType(PredType::NATIVE_CHAR); }
inline IntType getType(const signed char &) {
  return IntType(PredType::NATIVE_SCHAR);
}
inline IntType getType(const unsigned char &) {
  return IntType(PredType::NATIVE_UCHAR);
}
inline IntType getType(const short &) {
  return IntType(PredType::NATIVE_SHORT);
}
inline IntType getType(const unsigned short &) {
  return IntType(PredType::NATIVE_USHORT);
}
inline IntType getType(const int &) { return IntType(PredType::NATIVE_INT); }
inline IntType getType(const unsigned int &) {
  return IntType(PredType::NATIVE_UINT);
}
inline IntType getType(const long &) { return IntType(PredType::NATIVE_LONG); }
inline IntType getType(const unsigned long &) {
  return IntType(PredType::NATIVE_ULONG);
}
inline IntType getType(const long long &) {
  return IntType(PredType::NATIVE_LLONG);
}
inline IntType getType(const unsigned long long &) {
  return IntType(PredType::NATIVE_ULLONG);
}
inline FloatType getType(const float &) {
  return FloatType(PredType::NATIVE_FLOAT);
}
inline FloatType getType(const double &) {
  return FloatType(PredType::NATIVE_DOUBLE);
}
inline FloatType getType(const long double &) {
  return FloatType(PredType::NATIVE_LDOUBLE);
}
template <typename T> ArrayType getType(const std::complex<T> &) {
  // If the type is written to a file, and the file is closed, then the type
  // becomes invalid. We thus don't memoize the type.
  hsize_t dims[1]{2};
  return ArrayType(getType(T{}), 1, dims);
}

// Create attribute

template <typename T>
Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const T &value, const H5::DataType &type) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  assert(type.getSize() == sizeof value);
  auto attr = obj.createAttribute(name, type, DataSpace());
  attr.write(type, &value);
  return attr;
}

template <typename T>
Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const T &value) {
  return createAttribute(obj, name, value, getType(value));
}

template <typename T>
Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const std::vector<T> &values,
                          const H5::DataType &type) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  assert(type.getSize() == sizeof values[0]);
  const hsize_t dims = values.size();
  auto attr = obj.createAttribute(name, type, DataSpace(1, &dims));
  // HDF5 is overly cautious
  if (!values.empty())
    attr.write(type, values.data());
  return attr;
}

template <typename T>
Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const std::vector<T> &values) {
  return createAttribute(obj, name, values, getType(values[0]));
}

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const std::string &value);

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const char *value);

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const H5Location &obj_loc,
                          const std::string &obj_name);

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const H5::EnumType &type,
                          const std::string &valuename);

// Read attribute

template <typename T>
Attribute readAttribute(const H5Object &obj, const std::string &name, T &value,
                        const H5::DataType &type) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  assert(type.getSize() == sizeof value);
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  attr.read(type, &value);
  return attr;
}

template <typename T>
Attribute readAttribute(const H5Object &obj, const std::string &name,
                        T &value) {
  return readAttribute(obj, name, value, getType(value));
}

template <typename T>
Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::vector<T> &values, const H5::DataType &type) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  assert(type.getSize() == sizeof(T));
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  auto npoints = space.getSimpleExtentNpoints();
  values.resize(npoints);
  // HDF5 cannote read zero-length attributes into null-pointer buffers
  T dummy;
  attr.read(type, values.empty() ? &dummy : values.data());
  return attr;
}

template <typename T>
Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::vector<T> &values) {
  return readAttribute(obj, name, values, getType(values[0]));
}

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::string &value);

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        /*H5Location &group */ Group &group);

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::string &valuename, const H5::EnumType &type);

template <typename T>
typename std::enable_if<!detail::is_vector<T>::value, T>::type
readAttribute(const H5Object &obj, const std::string &name,
              const H5::DataType &type) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  T value;
  readAttribute(obj, name, value, type);
  return value;
}

template <typename T>
typename std::enable_if<!detail::is_vector<T>::value, T>::type
readAttribute(const H5Object &obj, const std::string &name,
              const H5::EnumType &type) {
  static_assert(std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  T value;
  readAttribute(obj, name, value, type);
  return value;
}

template <typename T>
typename std::enable_if<!detail::is_vector<T>::value, T>::type
readAttribute(const H5Object &obj, const std::string &name) {
  // static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(!detail::is_vector<T>::value, "");
  T value;
  readAttribute(obj, name, value);
  return value;
}

template <typename T>
typename std::enable_if<detail::is_vector<T>::value, T>::type
readAttribute(const H5Object &obj, const std::string &name,
              const H5::DataType &type) {
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  auto size = space.getSimpleExtentNpoints();
  T values(size);
  assert(type.getSize() == sizeof values[0]);
  readAttribute(obj, name, values, type);
  return values;
}

template <typename T>
typename std::enable_if<detail::is_vector<T>::value, T>::type
readAttribute(const H5Object &obj, const std::string &name) {
  static_assert(!std::is_same<T, std::string>::value, "");
  static_assert(detail::is_vector<T>::value, "");
  T values;
  readAttribute(obj, name, values);
  return values;
}

template <typename T>
T readGroupAttribute(const H5Location &loc, const std::string &groupname,
                     const std::string &attrname) {
  auto group = loc.openGroup(groupname);
  return readAttribute<T>(group, attrname);
}

template <typename T>
T getAttribute(const H5Object &obj, const std::string &name,
               const H5::DataType &type) {
  T value;
  readAttribute(obj, name, value, type);
  return value;
}

template <typename T>
T getAttribute(const H5Object &obj, const std::string &name) {
  return getAttribute<T>(obj, name, getType(T{}));
}

DataSet createDataSet(const H5Location &loc, const std::string &name,
                      const std::string &value);

DataSet createDataSet(const H5Location &loc, const std::string &name,
                      const char *value);

DataSet readDataSet(const H5Location &loc, const std::string &name,
                    std::string &value);

// Create a hard link
// Note argument order: first link location, then link target
herr_t createHardLink(const CommonFG &link_loc, const std::string &link_name,
                      const H5Location &obj_loc, const std::string &obj_name);

herr_t createHardLink(const H5Location &link_loc, const std::string &link_path,
                      const std::string &link_name, const H5Location &obj_loc,
                      const std::string &obj_name);

// Create a soft link
// Note argument order: first link location, then link target
herr_t createSoftLink(const CommonFG &link_loc, const std::string &link_name,
                      const std::string &obj_path);

herr_t createSoftLink(const H5Location &link_loc, const std::string &link_path,
                      const std::string &link_name,
                      const std::string &obj_path);

// Create an external link
// Note argument order: first link location, then link target
herr_t createExternalLink(const CommonFG &link_loc,
                          const std::string &link_name,
                          const std::string &file_name,
                          const std::string &obj_name);

// Read external link
void readExternalLink(const CommonFG &link_loc, const std::string &link_name,
                      bool &link_exists, std::string &file_name,
                      std::string &obj_name);

// Write a map (ignoring the keys)
template <typename K, typename T>
Group createGroup(const H5Location &loc, const std::string &name,
                  const std::map<K, T> &m) {
  // We assume that T is a subtype of Common
  auto group = loc.createGroup(name);
  for (const auto &p : m)
    p.second->write(group, loc);
  return group;
}

// This is probably never correct; instead, the group's entries should insert
// themselves into the group
#if 0
template <typename K, typename T>
Group createHardLinkGroup(const CommonFG &loc, const std::string &name,
                          const H5Location &parent, const std::string &path,
                          const std::map<K, T> &m) {
  // We assume that T is a subtype of Common
  auto group = loc.createGroup(name);
  for (const auto &p : m)
    createHardLink(group, p.second->name, parent, path + "/" + p.second->name);
  return group;
}
#endif

// Read a map
template <typename R>
Group readGroup(const H5Location &loc, const std::string &name, R read_object) {
  auto group = loc.openGroup(name);
  hsize_t idx = 0;
  iterateElems(
      group, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
      [&](const Group &group, const std::string &name, const H5L_info_t *info) {
        read_object(group, name);
        return 0;
      });
  return group;
}

template <typename K, typename T>
bool checkGroupNames(const CommonFG &loc, const std::string &name,
                     const std::map<K, T> &m) {
  std::set<std::string> names;
  readGroup(loc, name, [&](const Group &group, const std::string &name) {
    names.insert(name);
  });
  if (names.size() != m.size())
    return false;
  return std::equal(names.begin(), names.end(), m.begin(),
                    [](const std::string &s1, const std::pair<K, T> &sf2) {
                      return s1 == sf2.first;
                    });
}

} // namespace H5

#endif // #ifndef H5HELPERS_HPP
