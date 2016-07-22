#include "Field.hpp"

#include "DiscreteField.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Field::read(const H5::CommonFG &loc, const string &entry,
                 const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Field");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_manifold = project->manifolds().at(
      H5::readGroupAttribute<string>(group, "manifold", "name"));
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("configuration/fields/") + name(), "name") ==
         name());
  assert(H5::readGroupAttribute<string>(
             group, string("manifold/fields/") + name(), "name") == name());
  m_tangentspace = project->tangentspaces().at(
      H5::readGroupAttribute<string>(group, "tangentspace", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("tangentspace/fields/") + name(), "name") == name());
  m_tensortype = project->tensortypes().at(
      H5::readGroupAttribute<string>(group, "tensortype", "name"));
  H5::readGroup(group, "discretefields",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteField(group, name);
                });
  m_configuration->insert(name(), shared_from_this());
  m_manifold->insert(name(), shared_from_this());
  m_tangentspace->insert(name(), shared_from_this());
  m_tensortype->noinsert(shared_from_this());
}

void Field::merge(const shared_ptr<Field> &field) {
  assert(project()->name() == field->project()->name());
  assert(m_configuration->name() == field->configuration()->name());
  assert(m_manifold->name() == field->manifold()->name());
  assert(m_tangentspace->name() == field->tangentspace()->name());
  assert(m_tensortype->name() == field->tensortype()->name());
  for (const auto &iter : field->discretefields()) {
    const auto &discretefield = iter.second;
    if (!m_discretefields.count(discretefield->name()))
      createDiscreteField(
          discretefield->name(), project()->configurations().at(
                                     discretefield->configuration()->name()),
          manifold()->discretizations().at(
              discretefield->discretization()->name()),
          tangentspace()->bases().at(discretefield->basis()->name()));
    m_discretefields.at(discretefield->name())->merge(discretefield);
  }
}

ostream &Field::output(ostream &os, int level) const {
  os << indent(level) << "Field " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " Manifold "
     << quote(manifold()->name()) << " TangentSpace "
     << quote(tangentspace()->name()) << " TensorType "
     << quote(tensortype()->name()) << "\n";
  for (const auto &df : discretefields())
    df.second->output(os, level + 1);
  return os;
}

void Field::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Field");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    string("configurations/") + configuration->name());
  H5::createSoftLink(group, "configuration",
                     string("../configurations/") + configuration()->name());
  H5::createHardLink(group, string("project/configurations/") +
                                configuration()->name() + "/fields",
                     name(), group, ".");
  // H5::createHardLink(group, "manifold", parent,
  //                    string("manifolds/") + manifold->name());
  H5::createSoftLink(group, "manifold",
                     string("../manifolds/") + manifold()->name());
  H5::createHardLink(group, string("project/manifolds/") + manifold()->name() +
                                "/fields",
                     name(), group, ".");
  // H5::createHardLink(group, "tangentspace", parent,
  //                    string("tangentspaces/") + tangentspace->name());
  H5::createSoftLink(group, "tangentspace",
                     string("../tangentspaces/") + tangentspace()->name());
  H5::createHardLink(group, string("project/tangentspaces/") +
                                tangentspace()->name() + "/fields",
                     name(), group, ".");
  // H5::createHardLink(group, "tensortype", parent,
  //                    string("tensortypes/") + tensortype->name());
  H5::createSoftLink(group, "tensortype",
                     string("../tensortypes/") + tensortype()->name());
  H5::createGroup(group, "discretefields", discretefields());
}

shared_ptr<DiscreteField>
Field::createDiscreteField(const string &name,
                           const shared_ptr<Configuration> &configuration,
                           const shared_ptr<Discretization> &discretization,
                           const shared_ptr<Basis> &basis) {
  auto discretefield = DiscreteField::create(
      name, shared_from_this(), configuration, discretization, basis);
  checked_emplace(m_discretefields, discretefield->name(), discretefield,
                  "Field", "discretefields");
  assert(discretefield->invariant());
  return discretefield;
}
shared_ptr<DiscreteField> Field::readDiscreteField(const H5::CommonFG &loc,
                                                   const string &entry) {
  auto discretefield = DiscreteField::create(loc, entry, shared_from_this());
  checked_emplace(m_discretefields, discretefield->name(), discretefield,
                  "Field", "discretefields");
  assert(discretefield->invariant());
  return discretefield;
}
}
