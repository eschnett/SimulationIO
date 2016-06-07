#include "DiscreteFieldBlockComponent.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <sstream>

namespace SimulationIO {

using std::ostringstream;

void DiscreteFieldBlockComponent::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  m_discretefieldblock = discretefieldblock;
  auto group = loc.openGroup(entry);
  assert(
      H5::readAttribute<string>(
          group, "type",
          discretefieldblock->discretefield()->field()->project()->enumtype) ==
      "DiscreteFieldBlockComponent");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "discretefieldblock", "name") ==
         discretefieldblock->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_tensorcomponent =
      discretefieldblock->discretefield()
          ->field()
          ->tensortype()
          ->tensorcomponents()
          .at(H5::readGroupAttribute<string>(group, "tensorcomponent", "name"));
  m_datablock = DataBlock::read(
      group, dataname(), discretefieldblock->discretizationblock()->box());
  m_tensorcomponent->noinsert(shared_from_this());
}

void DiscreteFieldBlockComponent::merge(
    const shared_ptr<DiscreteFieldBlockComponent>
        &discretefieldblockcomponent) {
  assert(discretefieldblock()->name() ==
         discretefieldblockcomponent->discretefieldblock()->name());
  assert(m_tensorcomponent->name() ==
         discretefieldblockcomponent->tensorcomponent()->name());
  if (data_type == type_empty) {
    data_type = discretefieldblockcomponent->data_type;
    switch (data_type) {
    case type_empty:
      break;
    case type_dataset:
      // Don't know how to copy datasets
      assert(0);
      break;
    case type_extlink:
      data_extlink_filename =
          discretefieldblockcomponent->data_extlink_filename;
      data_extlink_objname = discretefieldblockcomponent->data_extlink_objname;
      break;
    case type_copy:
      data_copy_loc = discretefieldblockcomponent->data_copy_loc;
      data_copy_name = discretefieldblockcomponent->data_copy_name;
      break;
    case type_range:
      data_range_origin = discretefieldblockcomponent->data_range_origin;
      data_range_delta = discretefieldblockcomponent->data_range_delta;
      break;
    default:
      assert(0);
    }
  }
  assert(data_type == discretefieldblockcomponent->data_type);
  switch (data_type) {
  case type_empty:
    break;
  case type_dataset:
    // Don't know how to compare datasets
    assert(0);
    break;
  case type_extlink:
    assert(data_extlink_filename ==
           discretefieldblockcomponent->data_extlink_filename);
    assert(data_extlink_objname ==
           discretefieldblockcomponent->data_extlink_objname);
    break;
  case type_copy:
    assert(data_copy_loc == discretefieldblockcomponent->data_copy_loc);
    assert(data_copy_name == discretefieldblockcomponent->data_copy_name);
    break;
  case type_range:
    assert(data_range_origin == discretefieldblockcomponent->data_range_origin);
    assert(data_range_delta == discretefieldblockcomponent->data_range_delta);
    break;
  default:
    assert(0);
  }
}

ostream &DiscreteFieldBlockComponent::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockComponent " << quote(name())
     << ": DiscreteFieldBlock " << quote(discretefieldblock()->name())
     << " TensorComponent " << quote(tensorcomponent()->name()) << "\n";
  if (bool(datablock()))
    os << indent(level + 1) << *datablock() << "\n";
  return os;
}

void DiscreteFieldBlockComponent::write(const H5::CommonFG &loc,
                                        const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(
      group, "type",
      discretefieldblock()->discretefield()->field()->project()->enumtype,
      "DiscreteFieldBlockComponent");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "discretefieldblock", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "discretefieldblock", "..");
  // H5::createHardLink(
  //     group, "tensorcomponent", parent,
  //     string("discretefield/field/tensortype/tensorcomponents/") +
  //         tensorcomponent->name());
  H5::createSoftLink(
      group, "tensorcomponent",
      string("../discretefield/field/tensortype/tensorcomponents/") +
          tensorcomponent()->name());
  if (bool(datablock()))
    datablock()->write(group, dataname());
}

void DiscreteFieldBlockComponent::unsetDataBlock() { m_datablock = nullptr; }

shared_ptr<DataRange>
DiscreteFieldBlockComponent::createDataRange(double origin,
                                             const vector<double> &delta) {
  assert(!m_datablock);
  auto res = make_shared<DataRange>(
      discretefieldblock()->discretizationblock()->box(), origin, delta);
  m_datablock = res;
  return res;
}
shared_ptr<CopyObj>
DiscreteFieldBlockComponent::createCopyObj(const H5::Group &group,
                                           const string &name) {
  assert(!m_datablock);
  auto res = make_shared<CopyObj>(
      discretefieldblock()->discretizationblock()->box(), group, name);
  m_datablock = res;
  return res;
}
shared_ptr<CopyObj>
DiscreteFieldBlockComponent::createCopyObj(const H5::H5File &file,
                                           const string &name) {
  assert(!m_datablock);
  auto res = make_shared<CopyObj>(
      discretefieldblock()->discretizationblock()->box(), file, name);
  m_datablock = res;
  return res;
}
shared_ptr<ExtLink>
DiscreteFieldBlockComponent::createExtLink(const string &filename,
                                           const string &objname) {
  assert(!m_datablock);
  auto res = make_shared<ExtLink>(
      discretefieldblock()->discretizationblock()->box(), filename, objname);
  m_datablock = res;
  return res;
}

string DiscreteFieldBlockComponent::getPath() const {
  auto discretefield = discretefieldblock()->discretefield();
  auto field = discretefield->field();
  ostringstream buf;
  buf << "fields/" << field->name() << "/discretefields/"
      << discretefield->name() << "/discretefieldblocks/"
      << discretefieldblock()->name() << "/discretefieldblockcomponents/"
      << name();
  return buf.str();
}
string DiscreteFieldBlockComponent::getName() const { return dataname(); }
}
