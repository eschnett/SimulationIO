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
  if (m_datablock) {
    // Cannot combine DataBlocks
    assert(!discretefieldblockcomponent->datablock());
  } else {
    // Cannot copy DataSet (yet)
    assert(!discretefieldblockcomponent->dataset());
    m_datablock = discretefieldblockcomponent->datablock();
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
  //     "discretefield/field/tensortype/tensorcomponents/" +
  //         tensorcomponent->name());
  H5::createSoftLink(group, "tensorcomponent",
                     "../discretefield/field/tensortype/tensorcomponents/" +
                         tensorcomponent()->name());
  if (bool(datablock()))
    datablock()->write(group, dataname());
}

void DiscreteFieldBlockComponent::unsetDataBlock() { m_datablock = nullptr; }

shared_ptr<DataSet>
DiscreteFieldBlockComponent::createDataSet(const H5::DataType &type) {
  assert(!m_datablock);
  auto res = make_shared<DataSet>(
      discretefieldblock()->discretizationblock()->box(), type);
  m_datablock = res;
  return res;
}

shared_ptr<DataBufferEntry> DiscreteFieldBlockComponent::createDataBufferEntry(
    const H5::DataType &type, const shared_ptr<DataBuffer> &databuffer) {
  assert(!m_datablock);
  auto res = make_shared<DataBufferEntry>(
      discretefieldblock()->discretizationblock()->box(), type, databuffer);
  m_datablock = res;
  return res;
}

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
