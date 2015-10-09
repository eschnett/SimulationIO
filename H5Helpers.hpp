#ifndef HDF5HELPERS_HPP
#define HDF5HELPERS_HPP

// HDF5 helpers

#include <H5Cpp.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace H5 {

// Get HDF5 datatype from C++ types

inline DataType getType(const char &) { return IntType(PredType::NATIVE_CHAR); }
inline DataType getType(const signed char &) {
  return IntType(PredType::NATIVE_SCHAR);
}
inline DataType getType(const unsigned char &) {
  return IntType(PredType::NATIVE_UCHAR);
}
inline DataType getType(const short &) {
  return IntType(PredType::NATIVE_SHORT);
}
inline DataType getType(const unsigned short &) {
  return IntType(PredType::NATIVE_USHORT);
}
inline DataType getType(const int &) { return IntType(PredType::NATIVE_INT); }
inline DataType getType(const unsigned int &) {
  return IntType(PredType::NATIVE_UINT);
}
inline DataType getType(const long &) { return IntType(PredType::NATIVE_LONG); }
inline DataType getType(const unsigned long &) {
  return IntType(PredType::NATIVE_ULONG);
}
inline DataType getType(const long long &) {
  return IntType(PredType::NATIVE_LLONG);
}
inline DataType getType(const unsigned long long &) {
  return IntType(PredType::NATIVE_ULLONG);
}
inline DataType getType(const float &) {
  return FloatType(PredType::NATIVE_FLOAT);
}
inline DataType getType(const double &) {
  return FloatType(PredType::NATIVE_DOUBLE);
}
inline DataType getType(const long double &) {
  return FloatType(PredType::NATIVE_LDOUBLE);
}

// Write attributes
template <typename T,
          typename std::enable_if<std::is_fundamental<T>::value> * = nullptr>
Attribute create_attribute(const H5Location &loc, const std::string &name,
                           const T &value) {
  auto attr = loc.createAttribute(name, getType(value), DataSpace());
  attr.write(getType(value), &value);
  return attr;
}

template <typename T>
Attribute create_attribute(const H5Location &loc, const std::string &name,
                           const std::vector<T> &values) {
  const hsize_t dims = values.size();
  T dummy;
  auto attr = loc.createAttribute(name, getType(dummy), DataSpace(1, &dims));
  const void *buf =
      values.empty() ? &dummy : values.data(); // HDF5 is overly cautious
  attr.write(getType(dummy), buf);
  return attr;
}

// Read attributes
template <typename T,
          typename std::enable_if<std::is_fundamental<T>::value> * = nullptr>
Attribute read_attribute(H5Location &loc, const std::string &name, T &value) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  attr.read(getType(value), &value);
  return attr;
}

template <typename T>
Attribute read_attribute(H5Location &loc, const std::string &name,
                         std::vector<T> &values) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  auto npoints = space.getSimpleExtentNpoints();
  values.resize(npoints);
  T dummy;
  void *buf =
      values.empty() ? &dummy : values.data(); // HDF5 is overly cautious
  attr.read(getType(dummy), buf);
  return attr;
}

// H5Literate
namespace detail {
template <typename Op> struct H5L_iterator {
  Op &&op;
  static herr_t call(hid_t g_id, const char *name, const H5L_info_t *info,
                     void *data) {
    return static_cast<H5L_iterator *>(data)->op(Group(g_id), name, info);
  }
  herr_t operator()(const Group &group, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx) {
    return H5Literate(group.getId(), index_type, order, idx, call, this);
  }
};
}

template <typename Op>
herr_t iterateElems(const Group &group, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx, Op &&op) {
  return detail::H5L_iterator<Op>{std::forward<Op>(op)}(group.getId(),
                                                        index_type, order, idx);
}

// Write a map (ignoring the keys)
template <typename Key, typename T>
Group create_group(const CommonFG &loc, const std::string &name,
                   const std::map<Key, T *> &m) {
  // We assume that Key is string-like, and that T is a subtype of Common
  auto group = loc.createGroup(name);
  for (const auto &p : m)
    p.second->write(group);
  return group;
}

// Read a map
template <typename R, typename Key, typename T>
Group read_group(const CommonFG &loc, const std::string &name, R read_object,
                 std::map<Key, T *> &m) {
  // We assume that Key is string-like, and that T is a subtype of Common
  auto group = loc.openGroup(name);
  hsize_t idx = 0;
  iterateElems(
      group, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
      [&](Group group, const std::string &name, const H5L_info_t *info) {
        read_object(name, group);
        return 0;
      });
  return group;
}
}

#endif // #ifndef HDF5HELPERS_HPP
