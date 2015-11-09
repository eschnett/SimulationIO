#include "Project.hpp"

#include "Configuration.hpp"
#include "CoordinateSystem.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "Parameter.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace SimulationIO {

using std::max;
using std::min;
using std::ostringstream;
using std::string;
using std::vector;

shared_ptr<Project> createProject(const string &name) {
  auto project = Project::create(name);
  assert(project->invariant());
  return project;
}
shared_ptr<Project> createProject(const H5::CommonFG &loc) {
  auto project = Project::create(loc);
  assert(project->invariant());
  return project;
}

void Project::read(const H5::CommonFG &loc) {
  auto group = loc.openGroup(".");
  createTypes(); // TODO: read from file
  assert(H5::readAttribute<string>(group, "type", enumtype) == "Project");
  H5::readAttribute(group, "name", name);
  H5::readGroup(group, "parameters",
                [&](const H5::Group &group, const string &name) {
                  createParameter(group, name);
                });
  H5::readGroup(group, "configurations",
                [&](const H5::Group &group, const string &name) {
                  createConfiguration(group, name);
                });
  H5::readGroup(group, "tensortypes",
                [&](const H5::Group &group, const string &name) {
                  createTensorType(group, name);
                });
  H5::readGroup(group, "manifolds",
                [&](const H5::Group &group, const string &name) {
                  createManifold(group, name);
                });
  H5::readGroup(group, "tangentspaces",
                [&](const H5::Group &group, const string &name) {
                  createTangentSpace(group, name);
                });
  H5::readGroup(group, "fields",
                [&](const H5::Group &group, const string &name) {
                  createField(group, name);
                });
  H5::readGroup(group, "coordinatesystems",
                [&](const H5::Group &group, const string &name) {
                  createCoordinateSystem(group, name);
                });
}

void Project::createStandardTensorTypes() {
  auto s3d = createTensorType("Scalar3D", 3, 0);
  s3d->createTensorComponent("scalar", 0, vector<int>{});

  auto v3d = createTensorType("Vector3D", 3, 1);
  v3d->createTensorComponent("0", 0, {0});
  v3d->createTensorComponent("1", 1, {1});
  v3d->createTensorComponent("2", 2, {2});

  auto st3d = createTensorType("SymmetricTensor3D", 3, 2);
  st3d->createTensorComponent("00", 0, {0, 0});
  st3d->createTensorComponent("01", 1, {0, 1});
  st3d->createTensorComponent("02", 2, {0, 2});
  st3d->createTensorComponent("11", 3, {1, 1});
  st3d->createTensorComponent("12", 4, {1, 2});
  st3d->createTensorComponent("22", 5, {2, 2});
}

ostream &Project::output(ostream &os, int level) const {
  os << indent(level) << "Project " << quote(name) << "\n";
  for (const auto &par : parameters)
    par.second->output(os, level + 1);
  for (const auto &conf : configurations)
    conf.second->output(os, level + 1);
  for (const auto &tt : tensortypes)
    tt.second->output(os, level + 1);
  for (const auto &m : manifolds)
    m.second->output(os, level + 1);
  for (const auto &ts : tangentspaces)
    ts.second->output(os, level + 1);
  for (const auto &f : fields)
    f.second->output(os, level + 1);
  for (const auto &cs : coordinatesystems)
    cs.second->output(os, level + 1);
  return os;
}

void Project::insertEnumField(const H5::EnumType &type, const string &name,
                              int value) {
  type.insert(name, &value);
}
void Project::createTypes() const {
  enumtype = H5::EnumType(H5::getType(int()));
  insertEnumField(enumtype, "Basis", type_Basis);
  insertEnumField(enumtype, "BasisVector", type_BasisVector);
  insertEnumField(enumtype, "Configuration", type_Configuration);
  insertEnumField(enumtype, "CoordinateField", type_CoordinateField);
  insertEnumField(enumtype, "CoordinateSystem", type_CoordinateSystem);
  insertEnumField(enumtype, "DiscreteField", type_DiscreteField);
  insertEnumField(enumtype, "DiscreteFieldBlock", type_DiscreteFieldBlock);
  insertEnumField(enumtype, "DiscreteFieldBlockComponent",
                  type_DiscreteFieldBlockComponent);
  insertEnumField(enumtype, "Discretization", type_Discretization);
  insertEnumField(enumtype, "DiscretizationBlock", type_DiscretizationBlock);
  insertEnumField(enumtype, "Field", type_Field);
  insertEnumField(enumtype, "Manifold", type_Manifold);
  insertEnumField(enumtype, "Parameter", type_Parameter);
  insertEnumField(enumtype, "ParameterValue", type_ParameterValue);
  insertEnumField(enumtype, "Project", type_Project);
  insertEnumField(enumtype, "SubDiscretization", type_SubDiscretization);
  insertEnumField(enumtype, "TangentSpace", type_TangentSpace);
  insertEnumField(enumtype, "TensorComponent", type_TensorComponent);
  insertEnumField(enumtype, "TensorType", type_TensorType);

  // A range is described by its minimum (inclusive), maximum (inclusive), and
  // count (non-negative). Here we use double precision for all three fields,
  // but other precisions are also possible. We use a floating point number to
  // describe the count for uniformity.
  {
    vector<size_t> offsets;
    size_t size = 0;
    offsets.push_back(size);
    size += H5::getType(double()).getSize();
    offsets.push_back(size);
    size += H5::getType(double()).getSize();
    offsets.push_back(size);
    size += H5::getType(double()).getSize();
    rangetype = H5::CompType(size);
    rangetype.insertMember("minimum", offsets.at(0), H5::getType(double()));
    rangetype.insertMember("maximum", offsets.at(1), H5::getType(double()));
    rangetype.insertMember("count", offsets.at(2), H5::getType(double()));
  }

  // point, box, and region are defined in RegionCalculus.hpp
  pointtypes.clear();
  boxtypes.clear();
  regiontypes.clear();
  for (int d = 0; d <= 4; ++d) {
    // point_t
    const hsize_t dim = max(1, d); // HDF5 requires at least 1 element
    const hsize_t dims[1] = {dim};
    // TODO: Handle d==0 correctly
    auto pointtype = H5::ArrayType(H5::getType(hssize_t()), 1, dims);
    pointtypes.push_back(pointtype);
    // box_t
    vector<size_t> offsets;
    size_t size = 0;
    offsets.push_back(size);
    size += pointtype.getSize();
    offsets.push_back(size);
    size += pointtype.getSize();
    auto boxtype = H5::CompType(size);
    boxtype.insertMember("lower", offsets.at(0), pointtype);
    boxtype.insertMember("upper", offsets.at(1), pointtype);
    boxtypes.push_back(boxtype);
    // region_t
    auto regiontype = H5::VarLenType(&boxtype);
    regiontypes.push_back(regiontype);
  }
}

namespace {
string itos(int d) {
  ostringstream buf;
  buf << d;
  return buf.str();
}
}

void Project::write(const H5::CommonFG &loc,
                    const H5::H5Location &parent) const {
  assert(invariant());
  // auto group = loc.createGroup(name);
  auto group = loc.openGroup(".");
  createTypes();
  auto typegroup = group.createGroup("types");
  enumtype.commit(typegroup, "SimulationIO");
  rangetype.commit(typegroup, "Range");
  for (int d = 0; d < int(pointtypes.size()); ++d)
    pointtypes.at(d).commit(typegroup, string("Point[") + itos(d) + "]");
  for (int d = 0; d < int(boxtypes.size()); ++d)
    boxtypes.at(d).commit(typegroup, string("Box[") + itos(d) + "]");
  for (int d = 0; d < int(regiontypes.size()); ++d)
    regiontypes.at(d).commit(typegroup, string("Region[") + itos(d) + "]");
  H5::createAttribute(group, "type", enumtype, "Project");
  H5::createAttribute(group, "name", name);
  // no link to parent
  H5::createGroup(group, "parameters", parameters);
  H5::createGroup(group, "configurations", configurations);
  H5::createGroup(group, "tensortypes", tensortypes);
  H5::createGroup(group, "manifolds", manifolds);
  H5::createGroup(group, "tangentspaces", tangentspaces);
  H5::createGroup(group, "fields", fields);
  H5::createGroup(group, "coordinatesystems", coordinatesystems);
}

shared_ptr<Parameter> Project::createParameter(const string &name) {
  auto parameter = Parameter::create(name, shared_from_this());
  checked_emplace(parameters, parameter->name, parameter);
  assert(parameter->invariant());
  return parameter;
}

shared_ptr<Parameter> Project::createParameter(const H5::CommonFG &loc,
                                               const string &entry) {
  auto parameter = Parameter::create(loc, entry, shared_from_this());
  checked_emplace(parameters, parameter->name, parameter);
  assert(parameter->invariant());
  return parameter;
}

shared_ptr<Configuration> Project::createConfiguration(const string &name) {
  auto configuration = Configuration::create(name, shared_from_this());
  checked_emplace(configurations, configuration->name, configuration);
  assert(configuration->invariant());
  return configuration;
}

shared_ptr<Configuration> Project::createConfiguration(const H5::CommonFG &loc,
                                                       const string &entry) {
  auto configuration = Configuration::create(loc, entry, shared_from_this());
  checked_emplace(configurations, configuration->name, configuration);
  assert(configuration->invariant());
  return configuration;
}

shared_ptr<TensorType> Project::createTensorType(const string &name,
                                                 int dimension, int rank) {
  auto tensortype =
      TensorType::create(name, shared_from_this(), dimension, rank);
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

shared_ptr<TensorType> Project::createTensorType(const H5::CommonFG &loc,
                                                 const string &entry) {
  auto tensortype = TensorType::create(loc, entry, shared_from_this());
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

shared_ptr<Manifold>
Project::createManifold(const string &name,
                        const shared_ptr<Configuration> &configuration,
                        int dimension) {
  auto manifold =
      Manifold::create(name, shared_from_this(), configuration, dimension);
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

shared_ptr<Manifold> Project::createManifold(const H5::CommonFG &loc,
                                             const string &entry) {
  auto manifold = Manifold::create(loc, entry, shared_from_this());
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

shared_ptr<TangentSpace>
Project::createTangentSpace(const string &name,
                            const shared_ptr<Configuration> &configuration,
                            int dimension) {
  auto tangentspace =
      TangentSpace::create(name, shared_from_this(), configuration, dimension);
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

shared_ptr<TangentSpace> Project::createTangentSpace(const H5::CommonFG &loc,
                                                     const string &entry) {
  auto tangentspace = TangentSpace::create(loc, entry, shared_from_this());
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

shared_ptr<Field>
Project::createField(const string &name,
                     const shared_ptr<Configuration> &configuration,
                     const shared_ptr<Manifold> &manifold,
                     const shared_ptr<TangentSpace> &tangentspace,
                     const shared_ptr<TensorType> &tensortype) {
  auto field = Field::create(name, shared_from_this(), configuration, manifold,
                             tangentspace, tensortype);
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

shared_ptr<Field> Project::createField(const H5::CommonFG &loc,
                                       const string &entry) {
  auto field = Field::create(loc, entry, shared_from_this());
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

shared_ptr<CoordinateSystem>
Project::createCoordinateSystem(const string &name,
                                const shared_ptr<Configuration> &configuration,
                                const shared_ptr<Manifold> &manifold) {
  auto coordinatesystem = CoordinateSystem::create(name, shared_from_this(),
                                                   configuration, manifold);
  checked_emplace(coordinatesystems, coordinatesystem->name, coordinatesystem);
  assert(coordinatesystem->invariant());
  return coordinatesystem;
}

shared_ptr<CoordinateSystem>
Project::createCoordinateSystem(const H5::CommonFG &loc, const string &entry) {
  auto coordinatesystem =
      CoordinateSystem::create(loc, entry, shared_from_this());
  checked_emplace(coordinatesystems, coordinatesystem->name, coordinatesystem);
  assert(coordinatesystem->invariant());
  return coordinatesystem;
}
}
