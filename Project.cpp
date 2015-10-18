#include "Project.hpp"

#include "Configuration.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "Parameter.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Project *createProject(const string &name) {
  auto project = new Project(name);
  assert(project->invariant());
  return project;
}
Project *createProject(const H5::CommonFG &loc, const string &entry) {
  auto project = new Project(loc, entry);
  assert(project->invariant());
  return project;
}

Project::Project(const H5::CommonFG &loc, const string &entry) {
  auto group = loc.openGroup(".");
  createTypes(); // TODO: read from file
  string type;
  H5::readAttribute(group, "type", enumtype, type);
  assert(type == "Project");
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
  // TODO
  // H5::readGroup(group, "coordinatesystems",
  //                [&](const H5::Group &group, const string &name) {
  //                  createCoordinateSystem(name, group);
  //                },
  //                coordinatesystems);
}

void Project::createStandardTensortypes() {
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
  os << indent(level) << "Project \"" << name << "\"\n";
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
  // TODO
  // for (const auto &cs : coordinatesystems)
  //   cs.second->output(os, level + 1);
  return os;
}

void Project::insertEnumField(const H5::EnumType &type, const string &name,
                              int value) {
  type.insert(name, &value);
}
void Project::createTypes() const {
  enumtype = H5::EnumType(H5::getType(0));
  insertEnumField(enumtype, "Basis", type_Basis);
  insertEnumField(enumtype, "BasisVector", type_BasisVector);
  insertEnumField(enumtype, "Configuration", type_Configuration);
  insertEnumField(enumtype, "DiscreteField", type_DiscreteField);
  insertEnumField(enumtype, "DiscreteFieldBlock", type_DiscreteFieldBlock);
  insertEnumField(enumtype, "DiscreteFieldBlockData",
                  type_DiscreteFieldBlockData);
  insertEnumField(enumtype, "Discretization", type_Discretization);
  insertEnumField(enumtype, "DiscretizationBlock", type_DiscretizationBlock);
  insertEnumField(enumtype, "Field", type_Field);
  insertEnumField(enumtype, "Manifold", type_Manifold);
  insertEnumField(enumtype, "Parameter", type_Parameter);
  insertEnumField(enumtype, "ParameterValue", type_ParameterValue);
  insertEnumField(enumtype, "Project", type_Project);
  insertEnumField(enumtype, "TangentSpace", type_TangentSpace);
  insertEnumField(enumtype, "TensorComponent", type_TensorComponent);
  insertEnumField(enumtype, "TensorType", type_TensorType);
}

void Project::write(const H5::CommonFG &loc,
                    const H5::H5Location &parent) const {
  // auto group = loc.createGroup(name);
  auto group = loc.openGroup(".");
  createTypes();
  H5::Group typegroup;
  typegroup = group.createGroup("types");
  enumtype.commit(typegroup, "SimulationIO");
  H5::createAttribute(group, "type", enumtype, "Project");
  H5::createAttribute(group, "name", name);
  // no link to parent
  H5::createGroup(group, "parameters", parameters);
  H5::createGroup(group, "configurations", configurations);
  H5::createGroup(group, "tensortypes", tensortypes);
  H5::createGroup(group, "manifolds", manifolds);
  H5::createGroup(group, "tangentspaces", tangentspaces);
  H5::createGroup(group, "fields", fields);
  // TODO
  // H5::createGroup(group, "coordiantesystems", coordinatesystems);
  enumtype = H5::EnumType();
}

Parameter *Project::createParameter(const string &name) {
  auto parameter = new Parameter(name, this);
  checked_emplace(parameters, parameter->name, parameter);
  assert(parameter->invariant());
  return parameter;
}

Parameter *Project::createParameter(const H5::CommonFG &loc,
                                    const string &entry) {
  auto parameter = new Parameter(loc, entry, this);
  checked_emplace(parameters, parameter->name, parameter);
  assert(parameter->invariant());
  return parameter;
}

Configuration *Project::createConfiguration(const string &name) {
  auto configuration = new Configuration(name, this);
  checked_emplace(configurations, configuration->name, configuration);
  assert(configuration->invariant());
  return configuration;
}

Configuration *Project::createConfiguration(const H5::CommonFG &loc,
                                            const string &entry) {
  auto configuration = new Configuration(loc, entry, this);
  checked_emplace(configurations, configuration->name, configuration);
  assert(configuration->invariant());
  return configuration;
}

TensorType *Project::createTensorType(const string &name, int dimension,
                                      int rank) {
  auto tensortype = new TensorType(name, this, dimension, rank);
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

TensorType *Project::createTensorType(const H5::CommonFG &loc,
                                      const string &entry) {
  auto tensortype = new TensorType(loc, entry, this);
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

Manifold *Project::createManifold(const string &name, int dimension) {
  auto manifold = new Manifold(name, this, dimension);
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

Manifold *Project::createManifold(const H5::CommonFG &loc,
                                  const string &entry) {
  auto manifold = new Manifold(loc, entry, this);
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

TangentSpace *Project::createTangentSpace(const string &name, int dimension) {
  auto tangentspace = new TangentSpace(name, this, dimension);
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

TangentSpace *Project::createTangentSpace(const H5::CommonFG &loc,
                                          const string &entry) {
  auto tangentspace = new TangentSpace(loc, entry, this);
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

Field *Project::createField(const string &name, Manifold *manifold,
                            TangentSpace *tangentspace,
                            TensorType *tensortype) {
  auto field = new Field(name, this, manifold, tangentspace, tensortype);
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

Field *Project::createField(const H5::CommonFG &loc, const string &entry) {
  auto field = new Field(loc, entry, this);
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

// TODO
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   Manifold *manifold) {
//   auto coordinatesystem = new CoordinateSystem(name, this, manifold);
//   checked_emplace(coordinatesystems,coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
//
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   const H5::CommonFG &loc)
//                                                   {
//   auto coordinatesystem = new CoordinateSystem(name, this, loc);
//   checked_emplace(coordinatesystems, coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
}
