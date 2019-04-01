#include "BasisVector.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool BasisVector::invariant() const {
  return Common::invariant() && bool(basis()) &&
         basis()->basisvectors().count(name()) &&
         basis()->basisvectors().at(name()).get() == this && direction() >= 0 &&
         direction() < basis()->tangentspace()->dimension() &&
         basis()->directions().count(direction()) &&
         basis()->directions().at(direction()).get() == this;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void BasisVector::read(const H5::H5Location &loc, const string &entry,
                       const shared_ptr<Basis> &basis) {
  m_basis = basis;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", basis->tangentspace()->project()->enumtype) ==
         "BasisVector");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "basis", "name") ==
         basis->name());
  H5::readAttribute(group, "direction", m_direction);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void BasisVector::read(const shared_ptr<ASDF::reader_state> &rs,
                       const YAML::Node &node, const shared_ptr<Basis> &basis) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/BasisVector-1.0.0");
  m_name = node["name"].Scalar();
  m_basis = basis;
  m_direction = node["direction"].as<int>();
}
#endif

void BasisVector::merge(const shared_ptr<BasisVector> &basisvector) {
  assert(basis()->name() == basisvector->basis()->name());
  assert(m_direction == basisvector->direction());
}

ostream &BasisVector::output(ostream &os, int level) const {
  os << indent(level) << "BasisVector " << quote(name()) << ": Basis "
     << quote(basis()->name()) << " direction=" << direction() << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void BasisVector::write(const H5::H5Location &loc,
                        const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      basis()->tangentspace()->project()->enumtype,
                      "BasisVector");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "basis", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "basis", "..");
  H5::createAttribute(group, "direction", direction());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> BasisVector::yaml_path() const {
  return concat(basis()->yaml_path(), {"basisvectors", name()});
}

ASDF::writer &BasisVector::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.value("direction", direction());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> BasisVector::tiledb_path() const {
  return concat(basis()->tiledb_path(), {"basisvectors", name()});
}

void BasisVector::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_attribute("direction", direction());
}
#endif

} // namespace SimulationIO
