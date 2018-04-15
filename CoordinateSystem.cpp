#include "CoordinateSystem.hpp"

#include "CoordinateField.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void CoordinateSystem::read(const H5::H5Location &loc, const string &entry,
                            const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "CoordinateSystem");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  m_manifold = project->manifolds().at(
      H5::readGroupAttribute<string>(group, "manifold", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "project/coordinatesystems/" + name(), "name") == name());
  H5::readGroup(group, "coordinatefields",
                [&](const H5::Group &group, const string &name) {
                  readCoordinateField(group, name);
                });
  // TODO: check group directions
  m_configuration->insert(name(), shared_from_this());
  m_manifold->insert(name(), shared_from_this());
}

void CoordinateSystem::merge(
    const shared_ptr<CoordinateSystem> &coordinatesystem) {
  assert(project()->name() == coordinatesystem->project()->name());
  assert(m_configuration->name() == coordinatesystem->configuration()->name());
  assert(m_manifold->name() == coordinatesystem->manifold()->name());
  for (const auto &iter : coordinatesystem->coordinatefields()) {
    const auto &coordinatefield = iter.second;
    if (!m_coordinatefields.count(coordinatefield->name()))
      createCoordinateField(
          coordinatefield->name(), coordinatefield->direction(),
          project()->fields().at(coordinatefield->field()->name()));
    m_coordinatefields.at(coordinatefield->name())->merge(coordinatefield);
  }
  for (const auto &iter : coordinatesystem->directions()) {
    auto direction = iter.first;
    assert(m_directions.at(direction)->name() ==
           coordinatesystem->directions().at(direction)->name());
  }
}

ostream &CoordinateSystem::output(ostream &os, int level) const {
  os << indent(level) << "CoordinateSystem " << quote(name())
     << ": Configuration " << quote(configuration()->name()) << " Manifold "
     << quote(manifold()->name()) << "\n";
  for (const auto &cf : directions())
    cf.second->output(os, level + 1);
  return os;
}

void CoordinateSystem::write(const H5::H5Location &loc,
                             const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "CoordinateSystem");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "configurations/" + configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../configurations/" + configuration()->name());
  H5::createHardLink(group,
                     "project/configurations/" + configuration()->name() +
                         "/coordinatesystems",
                     name(), group, ".");
  // H5::createHardLink(group, "manifold", parent,
  //                    "manifolds/" + manifold->name());
  H5::createSoftLink(group, "manifold", "../manifolds/" + manifold()->name());
  H5::createHardLink(
      group, "project/manifolds/" + manifold()->name() + "/coordinatesystems",
      name(), group, ".");
  H5::createGroup(group, "coordinatefields", coordinatefields());
  // TODO: output directions
}

shared_ptr<CoordinateField>
CoordinateSystem::createCoordinateField(const string &name, int direction,
                                        const shared_ptr<Field> &field) {
  assert(field->project().get() == project().get());
  auto coordinatefield =
      CoordinateField::create(name, shared_from_this(), direction, field);
  checked_emplace(m_coordinatefields, coordinatefield->name(), coordinatefield,
                  "CoordinateSystem", "coordinatefields");
  checked_emplace(m_directions, coordinatefield->direction(), coordinatefield,
                  "CoordinateSystem", "directions");
  assert(coordinatefield->invariant());
  return coordinatefield;
}

shared_ptr<CoordinateField>
CoordinateSystem::getCoordinateField(const string &name, int direction,
                                     const shared_ptr<Field> &field) {
  auto loc = m_coordinatefields.find(name);
  if (loc != m_coordinatefields.end()) {
    const auto &coordinatefield = loc->second;
    assert(coordinatefield->direction() == direction);
    assert(coordinatefield->field() == field);
    return coordinatefield;
  }
  return createCoordinateField(name, direction, field);
}

shared_ptr<CoordinateField> CoordinateSystem::copyCoordinateField(
    const shared_ptr<CoordinateField> &coordinatefield, bool copy_children) {
  auto field2 = project()->copyField(coordinatefield->field(), copy_children);
  auto coordinatefield2 = getCoordinateField(
      coordinatefield->name(), coordinatefield->direction(), field2);
  return coordinatefield2;
}

shared_ptr<CoordinateField>
CoordinateSystem::readCoordinateField(const H5::H5Location &loc,
                                      const string &entry) {
  auto coordinatefield =
      CoordinateField::create(loc, entry, shared_from_this());
  checked_emplace(m_coordinatefields, coordinatefield->name(), coordinatefield,
                  "CoordinateSystem", "coordinatefields");
  checked_emplace(m_directions, coordinatefield->direction(), coordinatefield,
                  "CoordinateSystem", "directions");
  assert(coordinatefield->invariant());
  return coordinatefield;
}
} // namespace SimulationIO
