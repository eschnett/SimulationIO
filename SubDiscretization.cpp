#include "SubDiscretization.hpp"

#include "Discretization.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>

namespace SimulationIO {

bool SubDiscretization::invariant() const {
  bool inv = Common::invariant() && bool(manifold()) &&
             manifold()->subdiscretizations().count(name()) &&
             manifold()->subdiscretizations().at(name()).get() == this &&
             bool(parent_discretization()) &&
             parent_discretization()->child_discretizations().count(name()) &&
             parent_discretization()
                     ->child_discretizations()
                     .at(name())
                     .lock()
                     .get() == this &&
             parent_discretization()->manifold().get() == manifold().get() &&
             bool(child_discretization()) &&
             child_discretization()->parent_discretizations().count(name()) &&
             child_discretization()
                     ->parent_discretizations()
                     .at(name())
                     .lock()
                     .get() == this &&
             child_discretization().get() != parent_discretization().get() &&
             child_discretization()->manifold().get() == manifold().get() &&
             child_discretization()->configuration().get() ==
                 parent_discretization()->configuration().get() &&
             int(factor().size()) == manifold()->dimension() &&
             int(offset().size()) == manifold()->dimension();
  for (int d = 0; d < int(factor().size()); ++d)
    inv &= std::isfinite(factor().at(d)) && factor().at(d) != 0.0;
  for (int d = 0; d < int(offset().size()); ++d)
    inv &= std::isfinite(offset().at(d));
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void SubDiscretization::read(const H5::H5Location &loc, const string &entry,
                             const shared_ptr<Manifold> &manifold) {
  m_manifold = manifold;
  auto group = loc.openGroup(entry);
  assert(
      H5::readAttribute<string>(group, "type", manifold->project()->enumtype) ==
      "SubDiscretization");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name());
  m_parent_discretization = manifold->discretizations().at(
      H5::readGroupAttribute<string>(group, "parent_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "parent_discretization/child_discretizations/" + name(),
             "name") == name());
  m_child_discretization = manifold->discretizations().at(
      H5::readGroupAttribute<string>(group, "child_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "child_discretization/parent_discretizations/" + name(),
             "name") == name());
  H5::readAttribute(group, "factor", m_factor);
  std::reverse(m_factor.begin(), m_factor.end());
  H5::readAttribute(group, "offset", m_offset);
  std::reverse(m_offset.begin(), m_offset.end());
  m_parent_discretization->insertChild(name(), shared_from_this());
  m_child_discretization->insertParent(name(), shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void SubDiscretization::read(const shared_ptr<ASDF::reader_state> &rs,
                             const YAML::Node &node,
                             const shared_ptr<Manifold> &manifold) {
  assert(
      node.Tag() ==
      "tag:github.com/eschnett/SimulationIO/asdf-cxx/SubDiscretization-1.0.0");
  m_name = node["name"].Scalar();
  m_manifold = manifold;
  m_parent_discretization =
      manifold->getDiscretization(rs, node["parent_discretization"]);
  m_child_discretization =
      manifold->getDiscretization(rs, node["child_discretization"]);
  m_factor = node["factor"].as<vector<double>>();
  m_offset = node["offset"].as<vector<double>>();
  m_parent_discretization->insertChild(name(), shared_from_this());
  m_child_discretization->insertParent(name(), shared_from_this());
}
#endif

void SubDiscretization::merge(
    const shared_ptr<SubDiscretization> &subdiscretization) {
  assert(manifold()->name() == subdiscretization->manifold()->name());
  assert(m_parent_discretization->name() ==
         subdiscretization->parent_discretization()->name());
  assert(m_child_discretization->name() ==
         subdiscretization->child_discretization()->name());
  assert(m_factor == subdiscretization->factor());
  assert(m_offset == subdiscretization->offset());
}

ostream &SubDiscretization::output(ostream &os, int level) const {
  using namespace Output;
  os << indent(level) << "SubDiscretization " << quote(name()) << ": Manifold "
     << quote(manifold()->name()) << " parent Discretization "
     << quote(parent_discretization()->name()) << " child Discretization "
     << quote(child_discretization()->name()) << " factor=" << factor()
     << " offset=" << offset() << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void SubDiscretization::write(const H5::H5Location &loc,
                              const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", manifold()->project()->enumtype,
                      "SubDiscretization");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "manifold", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "manifold", "..");
  // H5::createHardLink(group, "parent_discretization", parent,
  //                    "discretizations/" +
  //                    parent_discretization->name());
  H5::createSoftLink(group, "parent_discretization",
                     "../discretizations/" + parent_discretization()->name());
  H5::createHardLink(group,
                     "manifold/discretizations/" +
                         parent_discretization()->name() +
                         "/child_discretizations",
                     name(), group, ".");
  // H5::createHardLink(group, "child_discretization", parent,
  //                    "discretizations/" +
  //                    child_discretization->name());
  H5::createSoftLink(group, "child_discretization",
                     "../discretizations/" + child_discretization()->name());
  H5::createHardLink(group,
                     "manifold/discretizations/" +
                         child_discretization()->name() +
                         "/parent_discretizations",
                     name(), group, ".");
  auto tmp_factor = factor();
  std::reverse(tmp_factor.begin(), tmp_factor.end());
  H5::createAttribute(group, "factor", tmp_factor);
  auto tmp_offset = offset();
  std::reverse(tmp_offset.begin(), tmp_offset.end());
  H5::createAttribute(group, "offset", tmp_offset);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> SubDiscretization::yaml_path() const {
  return concat(manifold()->yaml_path(), {"subdiscretizations", name()});
}

ASDF::writer &SubDiscretization::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("parent_discretization", *parent_discretization());
  aw.alias("child_discretization", *child_discretization());
  aw.value("factor", factor());
  aw.value("offset", offset());
  return w;
}
#endif

} // namespace SimulationIO
