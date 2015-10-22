#include "DiscreteFieldBlockData.hpp"

#include "H5Helpers.hpp"

#include <sstream>

namespace SimulationIO {

using std::ostringstream;

void DiscreteFieldBlockData::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  this->discretefieldblock = discretefieldblock;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   discretefieldblock->discretefield.lock()
                                       ->field.lock()
                                       ->project.lock()
                                       ->enumtype) == "DiscreteFieldBlockData");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretefieldblock", "name") ==
         discretefieldblock->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  tensorcomponent =
      discretefieldblock->discretefield.lock()
          ->field.lock()
          ->tensortype->tensorcomponents.at(
              H5::readGroupAttribute<string>(group, "tensorcomponent", "name"));
  {
    data_type = type_empty; // fallback
    auto lapl = H5::take_hid(H5Pcreate(H5P_LINK_ACCESS));
    assert(lapl.valid());
    auto exists = H5Lexists(group.getLocId(), "data", lapl);
    assert(exists >= 0);
    if (exists) {
      // Check whether it is an external link
      bool have_extlink;
      H5::readExternalLink(group, "data", have_extlink, data_extlink_filename,
                           data_extlink_objname);
      if (have_extlink) {
        data_type = type_extlink;
      } else {
        herr_t herr;
        H5O_info_t info;
        herr = H5Oget_info_by_name(group.getLocId(), "data", &info, lapl);
        assert(!herr);
        assert(info.type == H5O_TYPE_DATASET);
        data_dataset = group.openDataSet("data");
        data_datatype = H5::DataType(H5Dget_type(data_dataset.getId()));
        data_dataspace = data_dataset.getSpace();
        data_type = type_dataset;
      }
    }
  }
  tensorcomponent->noinsert(shared_from_this());
}

string DiscreteFieldBlockData::getPath() const {
  const auto &discretefield = discretefieldblock.lock()->discretefield;
  const auto &field = discretefield.lock()->field;
  ostringstream buf;
  buf << "fields/" << field.lock()->name << "/discretefields/"
      << discretefield.lock()->name << "/discretefieldblocks/"
      << discretefieldblock.lock()->name << "/discretefieldblockdata/" << name;
  return buf.str();
}
string DiscreteFieldBlockData::getName() const { return "data"; }

void DiscreteFieldBlockData::setData() {
  data_type = type_empty;
  data_dataspace = H5::DataSpace();
  data_datatype = H5::DataType();
  data_dataset = H5::DataSet();
  data_extlink_filename.clear();
  data_extlink_objname.clear();
  data_copy_loc = H5::hid();
  data_copy_name.clear();
}

void DiscreteFieldBlockData::setData(const H5::DataType &datatype,
                                     const H5::DataSpace &dataspace) {
  if (data_type != type_empty)
    setData();
  data_type = type_dataset;
  data_datatype = datatype;
  data_dataspace = dataspace;
}

void DiscreteFieldBlockData::setData(const string &filename,
                                     const string &objname) {
  if (data_type != type_empty)
    setData();
  data_type = type_extlink;
  data_extlink_filename = filename;
  data_extlink_objname = objname;
}

void DiscreteFieldBlockData::setData(const H5::H5Location &loc,
                                     const string &name) {
  if (data_type != type_empty)
    setData();
  data_type = type_copy;
  data_copy_loc = loc.getId();
  data_copy_name = name;
}

ostream &DiscreteFieldBlockData::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockData " << quote(name)
     << ": DiscreteFieldBlock " << quote(discretefieldblock.lock()->name)
     << " TensorComponent " << quote(tensorcomponent->name) << "\n";
  os << indent(level + 1) << "data: ";
  switch (data_type) {
  case type_empty:
    os << "empty\n";
    break;
  case type_dataset:
#warning "TODO: output datatype, dataspace"
    os << "dataset\n";
    break;
  case type_extlink:
    os << "external link to " << quote(data_extlink_filename) << ":"
       << quote(data_extlink_objname) << "\n";
    break;
  case type_copy:
    os << "copy of (?):" << quote(data_copy_name) << "\n";
    break;
  default:
    assert(0);
  }
  return os;
}

void DiscreteFieldBlockData::write(const H5::CommonFG &loc,
                                   const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", discretefieldblock.lock()
                                         ->discretefield.lock()
                                         ->field.lock()
                                         ->project.lock()
                                         ->enumtype,
                      "DiscreteFieldBlockData");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefieldblock", parent, ".");
  H5::createHardLink(
      group, "tensorcomponent", parent,
      string("discretefield/field/tensortype/tensorcomponents/") +
          tensorcomponent->name);
  switch (data_type) {
  case type_empty: // do nothing
    break;
  case type_dataset:
    data_dataset = group.createDataSet("data", data_datatype, data_dataspace);
    break;
  case type_extlink:
    H5::createExternalLink(group, "data", data_extlink_filename,
                           data_extlink_objname);
    break;
  case type_copy: {
    auto ocpypl = H5::take_hid(H5Pcreate(H5P_OBJECT_COPY));
    assert(ocpypl.valid());
    auto lcpl = H5::take_hid(H5Pcreate(H5P_LINK_CREATE));
    assert(lcpl.valid());
    herr_t herr = H5Ocopy(data_copy_loc, data_copy_name.c_str(), group.getId(),
                          "data", ocpypl, lcpl);
    assert(!herr);
    break;
  }
  default:
    assert(0);
  }
}
}
