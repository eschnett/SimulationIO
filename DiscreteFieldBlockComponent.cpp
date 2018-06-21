#include "DiscreteFieldBlockComponent.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>
#include <sstream>

namespace SimulationIO {

using std::ostringstream;

bool DiscreteFieldBlockComponent::invariant() const {
  bool inv =
      Common::invariant() && bool(discretefieldblock()) &&
      discretefieldblock()->discretefieldblockcomponents().count(name()) &&
      discretefieldblock()->discretefieldblockcomponents().at(name()).get() ==
          this &&
      bool(tensorcomponent()) &&
      tensorcomponent()->discretefieldblockcomponents().nobacklink() &&
      discretefieldblock()->discretefield()->field()->tensortype().get() ==
          tensorcomponent()->tensortype().get();
  // Ensure all discrete field block data have different tensor components
  for (const auto &dfbd : discretefieldblock()->discretefieldblockcomponents())
    if (dfbd.second.get() != this)
      inv &= dfbd.second->tensorcomponent().get() != tensorcomponent().get();
  // Ensure mapping from storage_indices is correct
  inv &= discretefieldblock()
             ->storage_indices()
             .at(tensorcomponent()->storage_index())
             .get() == this;
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteFieldBlockComponent::read(
    const H5::H5Location &loc, const string &entry,
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DiscreteFieldBlockComponent::read(
    const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  assert(node.Tag() == "tag:github.com/eschnett/SimulationIO/asdf-cxx/"
                       "DiscreteFieldBlockComponent-1.0.0");
  m_name = node["name"].Scalar();
  m_discretefieldblock = discretefieldblock;
  m_tensorcomponent = discretefieldblock->discretefield()
                          ->field()
                          ->tensortype()
                          ->getTensorComponent(rs, node["tensorcomponent"]);
  if (node[dataname()].IsDefined())
    m_datablock = DataBlock::read_asdf(
        rs, node[dataname()], discretefieldblock->discretizationblock()->box());
  m_tensorcomponent->noinsert(shared_from_this());
}
#endif

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
#ifdef SIMULATIONIO_HAVE_HDF5
    // Cannot copy DataSet (yet)
    assert(!discretefieldblockcomponent->dataset());
#endif
#warning "TODO: Handle ASDF types"
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

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteFieldBlockComponent::write(const H5::H5Location &loc,
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> DiscreteFieldBlockComponent::yaml_path() const {
  return concat(discretefieldblock()->yaml_path(),
                {"discretefieldblockcomponents", name()});
}

ASDF::writer &DiscreteFieldBlockComponent::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("tensorcomponent", *tensorcomponent());
  if (bool(datablock()))
    datablock()->write(w, dataname());
  return w;
}
#endif

void DiscreteFieldBlockComponent::unsetDataBlock() { m_datablock = nullptr; }

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

shared_ptr<DataRange>
DiscreteFieldBlockComponent::createDataRange(double origin,
                                             const vector<double> &delta) {
  assert(!m_datablock);
  auto res = make_shared<DataRange>(
      discretefieldblock()->discretizationblock()->box(), origin, delta);
  m_datablock = res;
  return res;
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<ASDFData> DiscreteFieldBlockComponent::createASDFData(
    const shared_ptr<ASDF::ndarray> &ndarray) {
  assert(!m_datablock);
  auto res = make_shared<ASDFData>(
      discretefieldblock()->discretizationblock()->box(), ndarray);
  m_datablock = res;
  return res;
}

shared_ptr<ASDFData> DiscreteFieldBlockComponent::createASDFData(
    const ASDF::memoized<ASDF::block_t> &mdata,
    const shared_ptr<ASDF::datatype_t> &datatype) {
  assert(!m_datablock);
  auto res = make_shared<ASDFData>(
      discretefieldblock()->discretizationblock()->box(), mdata, datatype);
  m_datablock = res;
  return res;
}

shared_ptr<ASDFData> DiscreteFieldBlockComponent::createASDFData(
    const void *data, size_t npoints, const box_t &memlayout,
    const shared_ptr<ASDF::datatype_t> &datatype) {
  assert(!m_datablock);
  auto res =
      make_shared<ASDFData>(discretefieldblock()->discretizationblock()->box(),
                            data, npoints, memlayout, datatype);
  m_datablock = res;
  return res;
}

shared_ptr<ASDFRef>
DiscreteFieldBlockComponent::createASDFRef(const string &filename,
                                           const vector<string> &path) {
  assert(!m_datablock);
  auto res =
      make_shared<ASDFRef>(discretefieldblock()->discretizationblock()->box(),
                           make_shared<ASDF::reference>(filename, path));
  m_datablock = res;
  return res;
}
#endif

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

} // namespace SimulationIO
