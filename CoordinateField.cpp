#include "CoordinateField.hpp"

#include "CoordinateSystem.hpp"
#include "Field.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void CoordinateField::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<CoordinateSystem> &coordinatesystem) {
  this->coordinatesystem = coordinatesystem;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type",
             coordinatesystem->manifold->project.lock()->enumtype) ==
         "CoordinateField");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "coordinatesystem", "name") ==
         coordinatesystem->name);
  H5::readAttribute(group, "direction", direction);
  field = coordinatesystem->manifold->project.lock()->fields.at(
      H5::readGroupAttribute<string>(group, "field", "name"));
  field->noinsert(shared_from_this());
}

ostream &CoordinateField::output(ostream &os, int level) const {
  os << indent(level) << "CoordinateField " << quote(name)
     << ": CoordinateSystem " << quote(coordinatesystem.lock()->name)
     << " direction=" << direction << " Field " << quote(field->name) << "\n";
  return os;
}

void CoordinateField::write(const H5::CommonFG &loc,
                            const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(
      group, "type",
      coordinatesystem.lock()->manifold->project.lock()->enumtype,
      "CoordinateField");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "coordinatesystem", parent, ".");
  H5::createAttribute(group, "direction", direction);
  H5::createHardLink(group, "field", parent,
                     string("project/fields/") + field->name);
}
}
