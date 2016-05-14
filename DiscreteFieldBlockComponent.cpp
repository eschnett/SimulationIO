#include "DiscreteFieldBlockComponent.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <sstream>

namespace SimulationIO {

using std::ostringstream;

void DiscreteFieldBlockComponent::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  this->discretefieldblock = discretefieldblock;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", discretefieldblock->discretefield.lock()
                                ->field.lock()
                                ->project.lock()
                                ->enumtype) == "DiscreteFieldBlockComponent");
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
  if (group.attrExists("data_origin")) {
    // "data" is an attribute
    H5::readAttribute(group, "data_origin", data_range_origin);
    H5::readAttribute(group, "data_delta", data_range_delta);
    std::reverse(data_range_delta.begin(), data_range_delta.end());
    data_type = type_range;
  } else {
    auto lapl = H5::take_hid(H5Pcreate(H5P_LINK_ACCESS));
    assert(lapl.valid());
    auto exists = H5Lexists(group.getLocId(), "data", lapl);
    assert(exists >= 0);
    if (exists) {
      // "data" is a link
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
    } else {
      // "data" is not present
      data_type = type_empty;
    }
  }
  tensorcomponent->noinsert(shared_from_this());
}

void DiscreteFieldBlockComponent::setData() {
  data_type = type_empty;
  data_dataspace = H5::DataSpace();
  data_datatype = H5::DataType();
  data_dataset = H5::DataSet();
  data_extlink_filename.clear();
  data_extlink_objname.clear();
  data_copy_loc = H5::hid();
  data_copy_name.clear();
}

void DiscreteFieldBlockComponent::setData(const H5::DataType &datatype,
                                          const H5::DataSpace &dataspace) {
  if (data_type != type_empty)
    setData();
  data_type = type_dataset;
  data_datatype = datatype;
  data_dataspace = dataspace;
}

void DiscreteFieldBlockComponent::setData(const string &filename,
                                          const string &objname) {
  if (data_type != type_empty)
    setData();
  data_type = type_extlink;
  data_extlink_filename = filename;
  data_extlink_objname = objname;
}

void DiscreteFieldBlockComponent::setData(const H5::H5Location &loc,
                                          const string &name) {
  if (data_type != type_empty)
    setData();
  data_type = type_copy;
  data_copy_loc = loc.getId();
  data_copy_name = name;
}

void DiscreteFieldBlockComponent::setData(double origin,
                                          const vector<double> &delta) {
  if (data_type != type_empty)
    setData();
  data_type = type_range;
  data_range_origin = origin;
  data_range_delta = delta;
}

ostream &DiscreteFieldBlockComponent::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockComponent " << quote(name)
     << ": DiscreteFieldBlock " << quote(discretefieldblock.lock()->name)
     << " TensorComponent " << quote(tensorcomponent->name) << "\n";
  os << indent(level + 1) << "data: ";
  switch (data_type) {
  case type_empty:
    os << "empty\n";
    break;
  case type_dataset: {
    auto cls = data_datatype.getClass();
    auto clsname = H5::className(cls);
    auto typesize = data_datatype.getSize();
    assert(data_dataspace.isSimple());
    const int dim = data_dataspace.getSimpleExtentNdims();
    vector<hsize_t> size(dim);
    data_dataspace.getSimpleExtentDims(size.data());
    std::reverse(size.begin(), size.end());
    os << "dataset type=" << clsname << "(" << (8 * typesize)
       << " bit) shape=" << size << "\n";
    break;
  }
  case type_extlink:
    os << "external link to " << quote(data_extlink_filename) << ":"
       << quote(data_extlink_objname) << "\n";
    break;
  case type_copy:
    os << "copy of (?):" << quote(data_copy_name) << "\n";
    break;
  case type_range: {
    os << "range origin=" << data_range_origin << " delta=[";
    for (int d = 0; d < int(data_range_delta.size()); ++d) {
      if (d > 0)
        os << ",";
      os << data_range_delta.at(d);
    }
    os << "]\n";
    break;
  }
  default:
    assert(0);
  }
  return os;
}

void DiscreteFieldBlockComponent::write(const H5::CommonFG &loc,
                                        const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", discretefieldblock.lock()
                                         ->discretefield.lock()
                                         ->field.lock()
                                         ->project.lock()
                                         ->enumtype,
                      "DiscreteFieldBlockComponent");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefieldblock", parent, ".");
  H5::createHardLink(
      group, "tensorcomponent", parent,
      string("discretefield/field/tensortype/tensorcomponents/") +
          tensorcomponent->name);
  switch (data_type) {
  case type_empty: // do nothing
    break;
  case type_dataset: {
    auto proplist = H5::DSetCreatPropList();
    proplist.setFletcher32();
    assert(data_dataspace.isSimple());
    const int dim = data_dataspace.getSimpleExtentNdims();
    vector<hsize_t> size(dim);
    data_dataspace.getSimpleExtentDims(size.data());
    vector<hsize_t> chunksize(dim);
    const hsize_t linear_size = 16; // 16^3 * 8 B = 32 kB
    for (int d = 0; d < dim; ++d)
      chunksize.at(d) = std::min(linear_size, size.at(d));
    proplist.setChunk(dim, chunksize.data());
    proplist.setShuffle(); // Shuffling improves compression
    const int level = 1;   // Level 1 is fast, but still offers good compression
    proplist.setDeflate(level);
    data_dataset =
        group.createDataSet("data", data_datatype, data_dataspace, proplist);
    break;
  }
  case type_extlink:
    H5::createExternalLink(group, "data", data_extlink_filename,
                           data_extlink_objname);
    break;
  case type_copy: {
    auto ocpypl = H5::take_hid(H5Pcreate(H5P_OBJECT_COPY));
    assert(ocpypl.valid());
    herr_t herr = H5Pset_copy_object(ocpypl, H5O_COPY_WITHOUT_ATTR_FLAG);
    assert(!herr);
    auto lcpl = H5::take_hid(H5Pcreate(H5P_LINK_CREATE));
    assert(lcpl.valid());
    herr = H5Ocopy(data_copy_loc, data_copy_name.c_str(), group.getId(), "data",
                   ocpypl, lcpl);
    assert(!herr);
    break;
  }
  case type_range: {
    auto delta = data_range_delta;
    std::reverse(delta.begin(), delta.end());
    H5::createAttribute(group, "data_origin", data_range_origin);
    H5::createAttribute(group, "data_delta", delta);
    break;
  }
  default:
    assert(0);
  }
}

string DiscreteFieldBlockComponent::getPath() const {
  const auto &discretefield = discretefieldblock.lock()->discretefield;
  const auto &field = discretefield.lock()->field;
  ostringstream buf;
  buf << "fields/" << field.lock()->name << "/discretefields/"
      << discretefield.lock()->name << "/discretefieldblocks/"
      << discretefieldblock.lock()->name << "/discretefieldblockcomponents/"
      << name;
  return buf.str();
}
string DiscreteFieldBlockComponent::getName() const {
  assert(data_type == type_dataset || data_type == type_extlink ||
         data_type == type_copy);
  return "data";
}

template <typename T>
void DiscreteFieldBlockComponent::writeData(const vector<T> &data) const {
  assert(data_type == type_dataset);
  auto size = data_dataspace.getSimpleExtentNpoints();
  assert(ptrdiff_t(data.size()) == size);
  data_dataset.write(data.data(), H5::getType(data[0]));
  // TODO: Add function to add / update minimum and maximum; call it after
  // copying
  auto minmaxit = std::minmax_element(data.begin(), data.end());
  H5::createAttribute(data_dataset, "minimum", *minmaxit.first);
  H5::createAttribute(data_dataset, "maximum", *minmaxit.second);
}
template void
DiscreteFieldBlockComponent::writeData(const vector<int> &data) const;
template void
DiscreteFieldBlockComponent::writeData(const vector<double> &data) const;
}
