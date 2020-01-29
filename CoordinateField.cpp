#include "CoordinateField.hpp"

#include "CoordinateSystem.hpp"
#include "Field.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool CoordinateField::invariant() const {
  return Common::invariant() && bool(coordinatesystem()) &&
         coordinatesystem()->coordinatefields().count(name()) &&
         coordinatesystem()->coordinatefields().at(name()).get() == this &&
         direction() >= 0 &&
         direction() < coordinatesystem()->manifold()->dimension() &&
         coordinatesystem()->directions().count(direction()) &&
         coordinatesystem()->directions().at(direction()).get() == this &&
         bool(field()) && field()->coordinatefields().nobacklink();
  // TODO: Ensure that the field lives on the same manifold
  // TODO: Ensure that all fields of this coordinate system are distinct
  // TODO: Ensure the field is a scalar
}

#ifdef SIMULATIONIO_HAVE_HDF5
void CoordinateField::read(
    const H5::H5Location &loc, const string &entry,
    const shared_ptr<CoordinateSystem> &coordinatesystem) {
  m_coordinatesystem = coordinatesystem;
  auto group = loc.openGroup(entry);
  assert(
      H5::readAttribute<string>(
          group, "type", coordinatesystem->manifold()->project()->enumtype) ==
      "CoordinateField");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "coordinatesystem", "name") ==
         coordinatesystem->name());
  H5::readAttribute(group, "direction", m_direction);
  m_field = coordinatesystem->manifold()->project()->fields().at(
      H5::readGroupAttribute<string>(group, "field", "name"));
  m_field->noinsert(shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void CoordinateField::read(
    const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
    const shared_ptr<CoordinateSystem> &coordinatesystem) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/CoordinateField-1.0.0");
  m_name = node["name"].Scalar();
  m_coordinatesystem = coordinatesystem;
  m_direction = node["direction"].as<int>();
  m_field =
      coordinatesystem->manifold()->project()->getField(rs, node["field"]);
  m_field->noinsert(shared_from_this());
}
#endif

void CoordinateField::merge(
    const shared_ptr<CoordinateField> &coordinatefield) {
  assert(coordinatesystem()->name() ==
         coordinatefield->coordinatesystem()->name());
  assert(m_direction == coordinatefield->direction());
  assert(m_field->name() == coordinatefield->field()->name());
}

ostream &CoordinateField::output(ostream &os, int level) const {
  os << indent(level) << "CoordinateField " << quote(name())
     << ": CoordinateSystem " << quote(coordinatesystem()->name())
     << " direction=" << direction() << " Field " << quote(field()->name())
     << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void CoordinateField::write(const H5::H5Location &loc,
                            const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      coordinatesystem()->manifold()->project()->enumtype,
                      "CoordinateField");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "coordinatesystem", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "coordinatesystem", "..");
  H5::createAttribute(group, "direction", direction());
  // H5::createHardLink(group, "field", parent,
  //                    "project/fields/" + field->name);
  H5::createSoftLink(group, "field", "../project/fields/" + field()->name());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> CoordinateField::yaml_path() const {
  return concat(coordinatesystem()->yaml_path(), {"coordinatefields", name()});
}

ASDF::writer &CoordinateField::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.value("direction", direction());
  aw.alias("field", *field());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
string CoordinateField::silo_path() const {
  return coordinatesystem()->silo_path() + "coordinatefields/" +
         legalize_silo_name(name()) + "/";
}

void CoordinateField::write(DBfile *const file, const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "name", name());
  write_attribute(file, loc, "direction", direction());
  write_symlink(file, loc, "field", field()->silo_path());
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> CoordinateField::tiledb_path() const {
  return concat(coordinatesystem()->tiledb_path(),
                {"coordinatefields", name()});
}

void CoordinateField::write(const tiledb::Context &ctx,
                            const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_attribute("direction", direction());
  w.add_symlink(concat(tiledb_path(), {"field"}), field()->tiledb_path());
}
#endif

} // namespace SimulationIO
