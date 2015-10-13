#ifndef HDF5HELPERS_HPP
#define HDF5HELPERS_HPP

// HDF5 helpers

#include <H5Cpp.h>

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace H5 {

// Convert CommonFG to H5Location
namespace detail {
struct H5LocationDeleter {
  void operator()(H5Location *loc) const {
    auto objtype = H5Iget_type(loc->getId());
    switch (objtype) {
    case H5I_FILE:
      delete static_cast<H5File *>(loc);
      break;
    case H5I_GROUP:
      delete static_cast<Group *>(loc);
      break;
    default:
      assert(0);
    }
  }
};
}
inline std::unique_ptr<H5Location, detail::H5LocationDeleter>
getLocation(const CommonFG &fg) {
  auto locid = fg.getLocId();
  assert(locid >= 0);
  auto objtype = H5Iget_type(locid);
  switch (objtype) {
  case H5I_FILE: {
    return {new H5File(locid), detail::H5LocationDeleter()};
  }
  case H5I_GROUP:
    return {new Group(locid), detail::H5LocationDeleter()};
  default:
    assert(0);
  }
}

// Get HDF5 datatype from C++ type

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

// Create attribute

template <typename T>
Attribute createAttribute(const H5Location &loc, const std::string &name,
                          const T &value) {
  auto attr = loc.createAttribute(name, getType(value), DataSpace());
  attr.write(getType(value), &value);
  return attr;
}

template <typename T>
Attribute createAttribute(const H5Location &loc, const std::string &name,
                          const std::vector<T> &values) {
  const hsize_t dims = values.size();
  T dummy;
  auto attr = loc.createAttribute(name, getType(dummy), DataSpace(1, &dims));
  // HDF5 is overly cautious
  if (!values.empty())
    attr.write(getType(dummy), values.data());
  return attr;
}

inline Attribute createAttribute(const H5Location &loc, const std::string &name,
                                 const std::string &value) {
  auto type = StrType(PredType::C_S1, H5T_VARIABLE);
  auto attr = loc.createAttribute(name, type, DataSpace());
  attr.write(type, H5std_string(value));
  return attr;
}

inline Attribute createAttribute(const H5Location &loc, const std::string &name,
                                 const char *value) {
  return createAttribute(loc, name, std::string(value));
}

inline Attribute createAttribute(const H5Location &loc, const std::string &name,
                                 const H5Location &obj_loc,
                                 const std::string &obj_name) {
  auto type = DataType(PredType::STD_REF_OBJ);
  auto attr = loc.createAttribute(name, type, DataSpace());
  hobj_ref_t reference;
  auto herr =
      H5Rcreate(&reference, obj_loc.getId(), obj_name.c_str(), H5R_OBJECT, -1);
  assert(!herr);
  attr.write(type, &reference);
  return attr;
}

// Read attribute

template <typename T>
Attribute readAttribute(const H5Location &loc, const std::string &name,
                        T &value) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  attr.read(getType(value), &value);
  return attr;
}

template <typename T>
Attribute readAttribute(const H5Location &loc, const std::string &name,
                        std::vector<T> &values) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  auto npoints = space.getSimpleExtentNpoints();
  values.resize(npoints);
  // HDF5 is overly cautious
  if (!values.empty())
    attr.read(getType(values[0]), values.data());
  return attr;
}

inline Attribute readAttribute(const H5Location &loc, const std::string &name,
                               std::string &value) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  auto type = StrType(PredType::C_S1, H5T_VARIABLE);
  H5std_string buf;
  attr.read(type, buf);
  value = buf;
  return attr;
}

inline Attribute readAttribute(const H5Location &loc, const std::string &name,
                               /*H5Location &ob*/ Group &obj) {
  auto attr = loc.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  auto type = DataType(PredType::STD_REF_OBJ);
  hobj_ref_t reference;
  attr.read(type, &reference);
  auto hid = H5Rdereference(loc.getId(), H5R_OBJECT, &reference);
  assert(hid >= 0);
  H5O_type_t obj_type;
  auto herr = H5Rget_obj_type(loc.getId(), H5R_OBJECT, &reference, &obj_type);
  assert(!herr);
  switch (obj_type) {
  case H5O_TYPE_GROUP:
    obj = Group(hid);
    break;
  // case H5O_TYPE_DATASET:
  //   obj = DataSet(hid);
  //   break;
  // case H5O_TYPE_NAMED_DATATYPE:
  //   obj = DataType(hid);
  //   break;
  default:
    assert(0);
  }
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
  herr_t operator()(const H5Location &loc, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx) {
    auto iret = H5Literate(loc.getId(), index_type, order, idx, call, this);
    assert(iret >= 0);
    return iret;
  }
};
}

template <typename Op>
herr_t iterateElems(const H5Location &loc, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx, Op &&op) {
  auto iret = detail::H5L_iterator<Op>{std::forward<Op>(op)}(loc, index_type,
                                                             order, idx);
  assert(iret >= 0);
  return iret;
}

// Write a map (ignoring the keys)
template <typename Key, typename T>
Group createGroup(const CommonFG &loc, const std::string &name,
                  const std::map<Key, T *> &m) {
  // We assume that Key is string-like, and that T is a subtype of Common
  auto group = loc.createGroup(name);
  for (const auto &p : m)
    p.second->write(group, *getLocation(loc));
  return group;
}

// Read a map
template <typename R, typename Key, typename T>
Group readGroup(const CommonFG &loc, const std::string &name, R read_object,
                std::map<Key, T *> &m) {
  // We assume that Key is string-like, and that T is a subtype of Common
  auto group = loc.openGroup(name);
  hsize_t idx = 0;
  iterateElems(
      group, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
      [&](const Group &group, const std::string &name, const H5L_info_t *info) {
        read_object(name, group);
        return 0;
      });
  return group;
}

// Create a hard link
inline herr_t createHardLink(const H5Location &obj_loc,
                             const std::string &obj_name,
                             const CommonFG &link_loc,
                             const std::string &link_name) {
  auto lcpl = H5Pcreate(H5P_LINK_CREATE);
  assert(lcpl >= 0);
  auto lapl = H5Pcreate(H5P_LINK_ACCESS);
  assert(lapl >= 0);
  auto herr =
      H5Lcreate_hard(obj_loc.getId(), obj_name.c_str(), link_loc.getLocId(),
                     link_name.c_str(), lcpl, lapl);
  assert(herr >= 0);
  auto lcpl_herr = H5Pclose(lcpl);
  assert(!lcpl_herr);
  auto lapl_herr = H5Pclose(lapl);
  assert(!lapl_herr);
  return herr;
}
}

#endif // #ifndef HDF5HELPERS_HPP
