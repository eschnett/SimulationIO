#include "Manifold.hpp"

#include "CoordinateSystem.hpp"
#include "Discretization.hpp"
#include "Field.hpp"
#include "SubDiscretization.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <set>

namespace SimulationIO {

using std::equal;
using std::set;

bool Manifold::invariant() const {
  bool inv = Common::invariant() && bool(project()) &&
             project()->manifolds().count(name()) &&
             project()->manifolds().at(name()).get() == this &&
             bool(configuration()) &&
             configuration()->manifolds().count(name()) &&
             configuration()->manifolds().at(name()).lock().get() == this &&
             dimension() >= 0;
  for (const auto &d : discretizations())
    inv &= !d.first.empty() && bool(d.second);
  return inv;
}

void Manifold::read(const H5::H5Location &loc, const string &entry,
                    const shared_ptr<Project> &project) {
  this->m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Manifold");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/manifolds/" + name(), "name") == name());
  H5::readAttribute(group, "dimension", m_dimension);
  H5::readGroup(group, "discretizations",
                [&](const H5::Group &group, const string &name) {
                  readDiscretization(group, name);
                });
  H5::readGroup(group, "subdiscretizations",
                [&](const H5::Group &group, const string &name) {
                  readSubDiscretization(group, name);
                });
  // Cannot check "fields", "coordinatesystems" since they have not been read
  // yet
  // assert(H5::checkGroupNames(group, "fields", fields));
  // assert(H5::checkGroupNames(group, "coordinatesystems", fields));
  m_configuration->insert(name(), shared_from_this());
}

void Manifold::merge(const shared_ptr<Manifold> &manifold) {
  assert(project()->name() == manifold->project()->name());
  assert(m_configuration->name() == manifold->configuration()->name());
  assert(m_dimension == manifold->dimension());
  for (const auto &iter : manifold->discretizations()) {
    const auto &discretization = iter.second;
    if (!m_discretizations.count(discretization->name()))
      createDiscretization(discretization->name(),
                           project()->configurations().at(
                               discretization->configuration()->name()));
    m_discretizations.at(discretization->name())->merge(discretization);
  }
  for (const auto &iter : manifold->subdiscretizations()) {
    const auto &subdiscretization = iter.second;
    if (!m_subdiscretizations.count(subdiscretization->name()))
      createSubDiscretization(
          subdiscretization->name(),
          discretizations().at(
              subdiscretization->parent_discretization()->name()),
          discretizations().at(
              subdiscretization->child_discretization()->name()),
          subdiscretization->factor(), subdiscretization->offset());
    m_subdiscretizations.at(subdiscretization->name())
        ->merge(subdiscretization);
  }
}

ostream &Manifold::output(ostream &os, int level) const {
  os << indent(level) << "Manifold " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " dim=" << dimension() << "\n";
  for (const auto &d : discretizations())
    d.second->output(os, level + 1);
  for (const auto &sd : subdiscretizations())
    sd.second->output(os, level + 1);
  for (const auto &f : fields())
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name())
       << "\n";
  for (const auto &cs : coordinatesystems())
    os << indent(level + 1) << "CoordinateSystem "
       << quote(cs.second.lock()->name()) << "\n";
  return os;
}

void Manifold::write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Manifold");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "configurations/" + configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../configurations/" + configuration()->name());
  H5::createHardLink(
      group, "project/configurations/" + configuration()->name() + "/manifolds",
      name(), group, ".");
  H5::createAttribute(group, "dimension", dimension());
  H5::createGroup(group, "discretizations", discretizations());
  H5::createGroup(group, "subdiscretizations", subdiscretizations());
  group.createGroup("fields");
  group.createGroup("coordinatesystems");
}

shared_ptr<Discretization>
Manifold::createDiscretization(const string &name,
                               const shared_ptr<Configuration> &configuration) {
  assert(configuration->project().get() == project().get());
  auto discretization =
      Discretization::create(name, shared_from_this(), configuration);
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}

shared_ptr<Discretization>
Manifold::getDiscretization(const string &name,
                            const shared_ptr<Configuration> &configuration) {
  auto loc = m_discretizations.find(name);
  if (loc != m_discretizations.end()) {
    const auto &discretization = loc->second;
    assert(discretization->configuration() == configuration);
    return discretization;
  }
  return createDiscretization(name, configuration);
}

shared_ptr<Discretization>
Manifold::copyDiscretization(const shared_ptr<Discretization> &discretization,
                             bool copy_children) {
  auto configuration2 =
      project()->copyConfiguration(discretization->configuration());
  auto discretization2 =
      getDiscretization(discretization->name(), configuration2);
  if (copy_children) {
    for (const auto &discretizationblock_kv :
         discretization->discretizationblocks()) {
      const auto &discretizationblock = discretizationblock_kv.second;
      auto discretizationblock2 = discretization2->copyDiscretizationBlock(
          discretizationblock, copy_children);
    }
  }
  return discretization2;
}

shared_ptr<Discretization>
Manifold::readDiscretization(const H5::H5Location &loc, const string &entry) {
  auto discretization = Discretization::create(loc, entry, shared_from_this());
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}

shared_ptr<SubDiscretization> Manifold::createSubDiscretization(
    const string &name, const shared_ptr<Discretization> &parent_discretization,
    const shared_ptr<Discretization> &child_discretization,
    const vector<double> &factor, const vector<double> &offset) {
  assert(parent_discretization->manifold().get() == this);
  assert(child_discretization->manifold().get() == this);
  auto subdiscretization =
      SubDiscretization::create(name, shared_from_this(), parent_discretization,
                                child_discretization, factor, offset);
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}

shared_ptr<SubDiscretization> Manifold::getSubDiscretization(
    const string &name, const shared_ptr<Discretization> &parent_discretization,
    const shared_ptr<Discretization> &child_discretization,
    const vector<double> &factor, const vector<double> &offset) {
  auto loc = m_subdiscretizations.find(name);
  if (loc != m_subdiscretizations.end()) {
    const auto &subdiscretization = loc->second;
    assert(subdiscretization->parent_discretization() == parent_discretization);
    assert(subdiscretization->child_discretization() == child_discretization);
    assert(subdiscretization->factor() == factor);
    assert(subdiscretization->offset() == offset);
    return subdiscretization;
  }
  return createSubDiscretization(name, parent_discretization,
                                 child_discretization, factor, offset);
}

shared_ptr<SubDiscretization> Manifold::copySubDiscretization(
    const shared_ptr<SubDiscretization> &subdiscretization,
    bool copy_children) {
  auto parent_discretization2 =
      copyDiscretization(subdiscretization->parent_discretization());
  auto child_discretization2 =
      copyDiscretization(subdiscretization->child_discretization());
  auto subdiscretization2 = getSubDiscretization(
      subdiscretization->name(), parent_discretization2, child_discretization2,
      subdiscretization->factor(), subdiscretization->offset());
  return subdiscretization2;
}

shared_ptr<SubDiscretization>
Manifold::readSubDiscretization(const H5::H5Location &loc,
                                const string &entry) {
  auto subdiscretization =
      SubDiscretization::create(loc, entry, shared_from_this());
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}
} // namespace SimulationIO
