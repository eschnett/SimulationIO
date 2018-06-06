#include "H5Helpers.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5

namespace H5 {

// Wrapper for hid_t that ensures correct HDF5 reference counting
hid::hid(const hid &other) : m_id(other.m_id) { incref(); }

hid::hid(hid &&other) : m_id(other.m_id) { other.m_id = -1; }

hid::~hid() { decref(); }

hid &hid::operator=(const hid &other) {
  if (other.m_id != m_id) {
    decref();
    m_id = other.m_id;
    incref();
  }
  return *this;
}

hid &hid::operator=(hid &&other) {
  decref();
  m_id = other.m_id;
  other.m_id = -1;
  return *this;
}

// Convert a HDF5 datatype class to a string
std::string className(H5T_class_t cls) {
  switch (cls) {
  case H5T_INTEGER:
    return "integer";
  case H5T_FLOAT:
    return "float";
  case H5T_TIME:
    return "time";
  case H5T_STRING:
    return "string";
  case H5T_BITFIELD:
    return "bitfield";
  case H5T_OPAQUE:
    return "opaque";
  case H5T_COMPOUND:
    return "compound";
  case H5T_REFERENCE:
    return "reference";
  case H5T_ENUM:
    return "enum";
  case H5T_VLEN:
    return "vlen";
  case H5T_ARRAY:
    return "array";
  default:
    assert(0);
  }
  assert(0);
}

// Describe the structure of an HDF5 datatype
std::string dump(const DataType &type) {
  using namespace SimulationIO;
  using namespace Output;
  switch (type.getClass()) {
  case H5T_INTEGER:
    return "integer";
  case H5T_FLOAT:
    return "float";
  case H5T_TIME:
    return "time";
  case H5T_STRING:
    return "string";
  case H5T_BITFIELD:
    return "bitfield";
  case H5T_OPAQUE:
    return "opaque";
  case H5T_COMPOUND: {
    const CompType &comptype(*static_cast<const CompType *>(&type));
    std::ostringstream buf;
    buf << "compound{";
    int n = comptype.getNmembers();
    for (int i = 0; i < n; ++i) {
      std::string mname = comptype.getMemberName(i);
      size_t offset = comptype.getMemberOffset(i);
      DataType mtype = comptype.getMemberDataType(i);
      buf << mname << ":" << dump(mtype) << "@" << offset << ";";
    }
    buf << "}";
    return buf.str();
  }
  case H5T_REFERENCE:
    return "reference";
  case H5T_ENUM:
    return "enum";
  case H5T_VLEN:
    return "vlen";
  case H5T_ARRAY: {
    const ArrayType &arraytype(*static_cast<const ArrayType *>(&type));
    std::ostringstream buf;
    int ndims = H5Tget_array_ndims(arraytype.getId());
    std::vector<hsize_t> dims(ndims);
    int iret = H5Tget_array_dims(arraytype.getId(), dims.data());
    assert(iret == ndims);
    // DataType etype = ???;
    buf << /*dump(etype)*/ "[unknown-element-type]"
        << ":array" << dims;
    return buf.str();
  }
  default:
    assert(0);
  }
}

// Create attribute

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const std::string &value) {
  // auto type = StrType(PredType::C_S1, H5T_VARIABLE);
  auto type = StrType(PredType::C_S1, value.size() + 1);
  auto attr = obj.createAttribute(name, type, DataSpace());
  attr.write(type, H5std_string(value));
  return attr;
}

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const char *value) {
  return createAttribute(obj, name, std::string(value));
}

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const H5Location &obj_loc,
                          const std::string &obj_name) {
  auto type = DataType(PredType::STD_REF_OBJ);
  auto attr = obj.createAttribute(name, type, DataSpace());
  hobj_ref_t reference;
  auto herr =
      H5Rcreate(&reference, obj_loc.getId(), obj_name.c_str(), H5R_OBJECT, -1);
  assert(!herr);
  attr.write(type, &reference);
  return attr;
}

Attribute createAttribute(const H5Object &obj, const std::string &name,
                          const H5::EnumType &type,
                          const std::string &valuename) {
  auto attr = obj.createAttribute(name, type, DataSpace());
  int value;
  assert(type.getSize() == sizeof value);
  type.valueOf(valuename, &value);
  attr.write(type, &value);
  return attr;
}

// Read attribute

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::string &value) {
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  auto type = attr.getStrType();
  auto size = type.getSize();
  H5std_string buf;
  attr.read(StrType(PredType::C_S1, size), buf);
  value = buf;
  return attr;
}

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        /*H5Location &group */ Group &group) {
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  auto type = DataType(PredType::STD_REF_OBJ);
  hobj_ref_t reference;
  attr.read(type, &reference);
  auto hid = H5Rdereference1(obj.getId(), H5R_OBJECT, &reference);
  assert(hid >= 0);
  H5O_type_t obj_type;
  auto herr = H5Rget_obj_type(obj.getId(), H5R_OBJECT, &reference, &obj_type);
  assert(!herr);
  switch (obj_type) {
  case H5O_TYPE_GROUP:
    group = Group(hid);
    break;
  // case H5O_TYPE_DATASET:
  //   group = DataSet(hid);
  //   break;
  // case H5O_TYPE_NAMED_DATATYPE:
  //   group = DataType(hid);
  //   break;
  default:
    assert(0);
  }
  return attr;
}

Attribute readAttribute(const H5Object &obj, const std::string &name,
                        std::string &valuename, const H5::EnumType &type) {
  auto attr = obj.openAttribute(name);
  auto space = attr.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  int value;
  assert(type.getSize() == sizeof value);
  attr.read(type, &value);
  valuename = type.nameOf(&value, 100);
  return attr;
}

DataSet createDataSet(const H5Location &loc, const std::string &name,
                      const std::string &value) {
  auto type = StrType(PredType::C_S1, value.size() + 1);
  auto dataset = loc.createDataSet(name, type, DataSpace());
  dataset.write(H5std_string(value), type);
  return dataset;
}

DataSet createDataSet(const H5Location &loc, const std::string &name,
                      const char *value) {
  return createDataSet(loc, name, std::string(value));
}

DataSet readDataSet(const H5Location &loc, const std::string &name,
                    std::string &value) {
  auto dataset = loc.openDataSet(name.c_str());
  auto space = dataset.getSpace();
  assert(space.getSimpleExtentType() == H5S_SCALAR);
  auto type = dataset.getStrType();
  auto size = type.getSize();
  H5std_string buf;
  dataset.read(buf, StrType(PredType::C_S1, size));
  value = buf;
  return dataset;
}

// Create a hard link
// Note argument order: first link location, then link target
herr_t createHardLink(const CommonFG &link_loc, const std::string &link_name,
                      const H5Location &obj_loc, const std::string &obj_name) {
  auto lcpl = take_hid(H5Pcreate(H5P_LINK_CREATE));
  assert(lcpl.valid());
  auto lapl = take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto herr =
      H5Lcreate_hard(obj_loc.getId(), obj_name.c_str(), link_loc.getLocId(),
                     link_name.c_str(), lcpl, lapl);
  assert(herr >= 0);
  return herr;
}

herr_t createHardLink(const H5Location &link_loc, const std::string &link_path,
                      const std::string &link_name, const H5Location &obj_loc,
                      const std::string &obj_name) {
  return createHardLink(link_loc.openGroup(link_path), link_name, obj_loc,
                        obj_name);
}

// Create a soft link
// Note argument order: first link location, then link target
herr_t createSoftLink(const CommonFG &link_loc, const std::string &link_name,
                      const std::string &obj_path) {
  auto lcpl = take_hid(H5Pcreate(H5P_LINK_CREATE));
  assert(lcpl.valid());
  auto lapl = take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto herr = H5Lcreate_soft(obj_path.c_str(), link_loc.getLocId(),
                             link_name.c_str(), lcpl, lapl);
  assert(herr >= 0);
  return herr;
}

herr_t createSoftLink(const H5Location &link_loc, const std::string &link_path,
                      const std::string &link_name,
                      const std::string &obj_path) {
  return createSoftLink(link_loc.openGroup(link_path), link_name, obj_path);
}

// Create an external link
// Note argument order: first link location, then link target
herr_t createExternalLink(const CommonFG &link_loc,
                          const std::string &link_name,
                          const std::string &file_name,
                          const std::string &obj_name) {
  auto lcpl = take_hid(H5Pcreate(H5P_LINK_CREATE));
  assert(lcpl.valid());
  auto lapl = take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto herr =
      H5Lcreate_external(file_name.c_str(), obj_name.c_str(),
                         link_loc.getLocId(), link_name.c_str(), lcpl, lapl);
  assert(herr >= 0);
  return herr;
}

// Read external link
void readExternalLink(const CommonFG &link_loc, const std::string &link_name,
                      bool &link_exists, std::string &file_name,
                      std::string &obj_name) {
  link_exists = false;
  auto lapl = take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto exists = H5Lexists(link_loc.getLocId(), link_name.c_str(), lapl);
  assert(exists >= 0);
  if (exists) {
    herr_t herr;
    H5L_info_t info;
    herr = H5Lget_info(link_loc.getLocId(), link_name.c_str(), &info, lapl);
    assert(!herr);
    if (info.type == H5L_TYPE_EXTERNAL) {
      std::vector<char> buf(info.u.val_size);
      herr = H5Lget_val(link_loc.getLocId(), link_name.c_str(), buf.data(),
                        buf.size(), lapl);
      assert(!herr);
      const char *file_name_ptr, *obj_name_ptr;
      unsigned flags;
      herr = H5Lunpack_elink_val(buf.data(), buf.size(), &flags, &file_name_ptr,
                                 &obj_name_ptr);
      assert(!herr);
      link_exists = true;
      file_name = file_name_ptr;
      obj_name = obj_name_ptr;
    }
  }
}

} // namespace H5

#endif
