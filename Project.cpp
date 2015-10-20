#include "Project.hpp"

#include "Configuration.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "Parameter.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

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
  // TODO
  // H5::readGroup(group, "coordinatesystems",
  //                [&](const H5::Group &group, const string &name) {
  //                  createCoordinateSystem(name, group);
  //                },
  //                coordinatesystems);
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
  assert(invariant());
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

shared_ptr<Manifold> Project::createManifold(const string &name,
                                             int dimension) {
  auto manifold = Manifold::create(name, shared_from_this(), dimension);
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

shared_ptr<TangentSpace> Project::createTangentSpace(const string &name,
                                                     int dimension) {
  auto tangentspace = TangentSpace::create(name, shared_from_this(), dimension);
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
Project::createField(const string &name, const shared_ptr<Manifold> &manifold,
                     const shared_ptr<TangentSpace> &tangentspace,
                     const shared_ptr<TensorType> &tensortype) {
  auto field = Field::create(name, shared_from_this(), manifold, tangentspace,
                             tensortype);
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

// TODO
// shared_ptr<CoordinateSystem>Project::createCoordinateSystem(const string
// &name,
//                                                   const
//                                                   shared_ptr<Manifold>&manifold)
//                                                   {
//   auto coordinatesystem = new CoordinateSystem(name, shared_from_this(),
//   manifold);
//   checked_emplace(coordinatesystems,coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
//
// shared_ptr<CoordinateSystem>Project::createCoordinateSystem(const string
// &name,
//                                                   const H5::CommonFG &loc)
//                                                   {
//   auto coordinatesystem = new CoordinateSystem(name, shared_from_this(),
//   loc);
//   checked_emplace(coordinatesystems, coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
}
