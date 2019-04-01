#include "Project.hpp"

#include "Buffer.hpp"
#include "Configuration.hpp"
#include "CoordinateSystem.hpp"
#include "Field.hpp"
#include "Helpers.hpp"
#include "Manifold.hpp"
#include "Parameter.hpp"
#include "ParameterValue.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace SimulationIO {

using std::ifstream;
using std::ios;
using std::max;
using std::min;
using std::ofstream;
using std::ostringstream;
using std::string;
using std::vector;

shared_ptr<Project> createProject(const string &name) {
  auto project = Project::create(name);
  assert(project->invariant());
  return project;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Project> readProject(const H5::H5Location &loc,
                                const string &filename) {
  auto project = Project::create(loc, filename);
  assert(project->invariant());
  return project;
}

shared_ptr<Project> readProjectHDF5(const string &filename) {
  auto file = H5::H5File(filename, H5F_ACC_RDONLY);
  return readProject(file, filename);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Project> readProject(const shared_ptr<ASDF::reader_state> &rs,
                                const YAML::Node &node) {
  auto project = Project::create(rs, node);
  assert(project->invariant());
  return project;
}

map<string, shared_ptr<Project>>
readProjectsASDF(const shared_ptr<istream> &pis, const string &filename) {
  map<string, shared_ptr<Project>> projects;
  function<void(const shared_ptr<ASDF::reader_state> &rs, const string &name,
                const YAML::Node &node)>
      read_project{[&](const shared_ptr<ASDF::reader_state> &rs,
                       const string &name, const YAML::Node &node) {
        projects[name] = readProject(rs, node);
      }};
  map<string, function<void(const shared_ptr<ASDF::reader_state> &rs,
                            const string &name, const YAML::Node &node)>>
      readers{{"tag:github.com/eschnett/SimulationIO/asdf-cxx/Project-1.0.0",
               read_project}};
  auto doc = ASDF::asdf(pis, filename, readers);
  return projects;
}

map<string, shared_ptr<Project>> readProjectsASDF(const string &filename) {
  auto pis = make_shared<ifstream>(filename, ios::binary | ios::in);
  return readProjectsASDF(pis, filename);
}

shared_ptr<Project> readProjectASDF(const shared_ptr<istream> &pis,
                                    const string &filename) {
  auto projects = readProjectsASDF(pis, filename);
  assert(projects.size() == 1);
  return projects.begin()->second;
}

shared_ptr<Project> readProjectASDF(const string &filename) {
  auto pis = make_shared<ifstream>(filename, ios::binary | ios::in);
  return readProjectASDF(pis, filename);
}
#endif

bool Project::invariant() const { return Common::invariant(); }

#ifdef SIMULATIONIO_HAVE_HDF5
void Project::read(const H5::H5Location &loc, const string &filename) {
  auto group = loc.openGroup(".");
  createTypes(); // TODO: read from file instead to ensure integer constants are
                 // consistent
  assert(H5::readAttribute<string>(group, "type", enumtype) == "Project");
  H5::readAttribute(group, "name", m_name);
  H5::readGroup(group, "parameters",
                [&](const H5::Group &group, const string &name) {
                  readParameter(group, name);
                });
  H5::readGroup(group, "configurations",
                [&](const H5::Group &group, const string &name) {
                  readConfiguration(group, name);
                });
  H5::readGroup(group, "tensortypes",
                [&](const H5::Group &group, const string &name) {
                  readTensorType(group, name);
                });
  H5::readGroup(group, "manifolds",
                [&](const H5::Group &group, const string &name) {
                  readManifold(group, name);
                });
  H5::readGroup(group, "tangentspaces",
                [&](const H5::Group &group, const string &name) {
                  readTangentSpace(group, name);
                });
  H5::readGroup(group, "fields",
                [&](const H5::Group &group, const string &name) {
                  readField(group, name);
                });
  H5::readGroup(group, "coordinatesystems",
                [&](const H5::Group &group, const string &name) {
                  readCoordinateSystem(group, name);
                });
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Project::read(const shared_ptr<ASDF::reader_state> &rs,
                   const YAML::Node &node) {
  createTypes(); // TODO: read from file instead to ensure integer constants are
                 // consistent
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Project-1.0.0");
  m_name = node["name"].Scalar();
  for (const auto &kv : node["parameters"])
    readParameter(rs, kv.second);
  for (const auto &kv : node["configurations"])
    readConfiguration(rs, kv.second);
  for (const auto &kv : node["tensortypes"])
    readTensorType(rs, kv.second);
  for (const auto &kv : node["manifolds"])
    readManifold(rs, kv.second);
  for (const auto &kv : node["tangentspaces"])
    readTangentSpace(rs, kv.second);
  for (const auto &kv : node["fields"])
    readField(rs, kv.second);
  for (const auto &kv : node["coordinatesystems"])
    readCoordinateSystem(rs, kv.second);
}
#endif

void Project::merge(const shared_ptr<Project> &project) {
  // TODO: combine types
  for (const auto &iter : project->parameters()) {
    const auto &parameter = iter.second;
    if (!m_parameters.count(parameter->name()))
      createParameter(parameter->name());
    m_parameters.at(parameter->name())->merge(parameter);
  }
  for (const auto &iter : project->configurations()) {
    const auto &configuration = iter.second;
    if (!m_configurations.count(configuration->name()))
      createConfiguration(configuration->name());
    m_configurations.at(configuration->name())->merge(configuration);
  }
  for (const auto &iter : project->tensortypes()) {
    const auto &tensortype = iter.second;
    if (!m_tensortypes.count(tensortype->name()))
      createTensorType(tensortype->name(), tensortype->dimension(),
                       tensortype->rank());
    m_tensortypes.at(tensortype->name())->merge(tensortype);
  }
  for (const auto &iter : project->manifolds()) {
    const auto &manifold = iter.second;
    if (!m_manifolds.count(manifold->name()))
      createManifold(manifold->name(),
                     m_configurations.at(manifold->configuration()->name()),
                     manifold->dimension());
    m_manifolds.at(manifold->name())->merge(manifold);
  }
  for (const auto &iter : project->tangentspaces()) {
    const auto &tangentspace = iter.second;
    if (!m_tangentspaces.count(tangentspace->name()))
      createTangentSpace(
          tangentspace->name(),
          m_configurations.at(tangentspace->configuration()->name()),
          tangentspace->dimension());
    m_tangentspaces.at(tangentspace->name())->merge(tangentspace);
  }
  for (const auto &iter : project->fields()) {
    const auto &field = iter.second;
    if (!m_fields.count(field->name()))
      createField(field->name(),
                  m_configurations.at(field->configuration()->name()),
                  m_manifolds.at(field->manifold()->name()),
                  m_tangentspaces.at(field->tangentspace()->name()),
                  m_tensortypes.at(field->tensortype()->name()));
    m_fields.at(field->name())->merge(field);
  }
  for (const auto &iter : project->coordinatesystems()) {
    const auto &coordinatesystem = iter.second;
    if (!m_coordinatesystems.count(coordinatesystem->name()))
      createCoordinateSystem(
          coordinatesystem->name(),
          m_configurations.at(coordinatesystem->configuration()->name()),
          m_manifolds.at(coordinatesystem->manifold()->name()));
    m_coordinatesystems.at(coordinatesystem->name())->merge(coordinatesystem);
  }
}

void Project::createStandardTensorTypes() {
  {
    auto s0d = createTensorType("Scalar0D", 0, 0);
    s0d->createTensorComponent("scalar", 0, vector<int>{});
  }

  { auto v0d = createTensorType("Vector0D", 0, 1); }

  { auto t0d = createTensorType("Tensor0D", 0, 2); }

  { auto st0d = createTensorType("SymmetricTensor0D", 0, 2); }

  {
    auto s1d = createTensorType("Scalar1D", 1, 0);
    s1d->createTensorComponent("scalar", 0, vector<int>{});
  }

  {
    auto v1d = createTensorType("Vector1D", 1, 1);
    v1d->createTensorComponent("0", 0, {0});
  }

  {
    auto t1d = createTensorType("Tensor1D", 1, 2);
    t1d->createTensorComponent("00", 0, {0, 0});
  }

  {
    auto st1d = createTensorType("SymmetricTensor1D", 1, 2);
    st1d->createTensorComponent("00", 0, {0, 0});
  }

  {
    auto s2d = createTensorType("Scalar2D", 2, 0);
    s2d->createTensorComponent("scalar", 0, vector<int>{});
  }

  {
    auto v2d = createTensorType("Vector2D", 2, 1);
    v2d->createTensorComponent("0", 0, {0});
    v2d->createTensorComponent("1", 1, {1});
  }

  {
    auto t2d = createTensorType("Tensor2D", 2, 2);
    t2d->createTensorComponent("00", 0, {0, 0});
    t2d->createTensorComponent("01", 1, {0, 1});
    t2d->createTensorComponent("10", 2, {1, 0});
    t2d->createTensorComponent("11", 3, {1, 1});
  }

  {
    auto st2d = createTensorType("SymmetricTensor2D", 2, 2);
    st2d->createTensorComponent("00", 0, {0, 0});
    st2d->createTensorComponent("01", 1, {0, 1});
    st2d->createTensorComponent("11", 2, {1, 1});
  }

  {
    auto s3d = createTensorType("Scalar3D", 3, 0);
    s3d->createTensorComponent("scalar", 0, vector<int>{});
  }

  {
    auto v3d = createTensorType("Vector3D", 3, 1);
    v3d->createTensorComponent("0", 0, {0});
    v3d->createTensorComponent("1", 1, {1});
    v3d->createTensorComponent("2", 2, {2});
  }

  {
    auto t3d = createTensorType("Tensor3D", 3, 2);
    t3d->createTensorComponent("00", 0, {0, 0});
    t3d->createTensorComponent("01", 1, {0, 1});
    t3d->createTensorComponent("02", 2, {0, 2});
    t3d->createTensorComponent("10", 3, {1, 0});
    t3d->createTensorComponent("11", 4, {1, 1});
    t3d->createTensorComponent("12", 5, {1, 2});
    t3d->createTensorComponent("20", 6, {2, 0});
    t3d->createTensorComponent("21", 7, {2, 1});
    t3d->createTensorComponent("22", 8, {2, 2});
  }

  {
    auto st3d = createTensorType("SymmetricTensor3D", 3, 2);
    st3d->createTensorComponent("00", 0, {0, 0});
    st3d->createTensorComponent("01", 1, {0, 1});
    st3d->createTensorComponent("02", 2, {0, 2});
    st3d->createTensorComponent("11", 3, {1, 1});
    st3d->createTensorComponent("12", 4, {1, 2});
    st3d->createTensorComponent("22", 5, {2, 2});
  }

  {
    auto s4d = createTensorType("Scalar4D", 4, 0);
    s4d->createTensorComponent("scalar", 0, vector<int>{});
  }

  {
    auto v4d = createTensorType("Vector4D", 4, 1);
    v4d->createTensorComponent("0", 0, {0});
    v4d->createTensorComponent("1", 1, {1});
    v4d->createTensorComponent("2", 2, {2});
    v4d->createTensorComponent("3", 3, {3});
  }

  {
    auto t4d = createTensorType("Tensor4D", 4, 2);
    t4d->createTensorComponent("00", 0, {0, 0});
    t4d->createTensorComponent("01", 1, {0, 1});
    t4d->createTensorComponent("02", 2, {0, 2});
    t4d->createTensorComponent("03", 3, {0, 3});
    t4d->createTensorComponent("10", 4, {1, 0});
    t4d->createTensorComponent("11", 5, {1, 1});
    t4d->createTensorComponent("12", 6, {1, 2});
    t4d->createTensorComponent("13", 7, {1, 3});
    t4d->createTensorComponent("20", 8, {2, 0});
    t4d->createTensorComponent("21", 9, {2, 1});
    t4d->createTensorComponent("22", 10, {2, 2});
    t4d->createTensorComponent("23", 11, {2, 3});
    t4d->createTensorComponent("30", 12, {3, 0});
    t4d->createTensorComponent("31", 13, {3, 1});
    t4d->createTensorComponent("32", 14, {3, 2});
    t4d->createTensorComponent("33", 15, {3, 3});
  }

  {
    auto st4d = createTensorType("SymmetricTensor4D", 4, 2);
    st4d->createTensorComponent("00", 0, {0, 0});
    st4d->createTensorComponent("01", 1, {0, 1});
    st4d->createTensorComponent("02", 2, {0, 2});
    st4d->createTensorComponent("03", 3, {0, 3});
    st4d->createTensorComponent("11", 4, {1, 1});
    st4d->createTensorComponent("12", 5, {1, 2});
    st4d->createTensorComponent("13", 6, {1, 3});
    st4d->createTensorComponent("22", 7, {2, 2});
    st4d->createTensorComponent("23", 8, {2, 3});
    st4d->createTensorComponent("33", 9, {3, 3});
  }
}

ostream &Project::output(ostream &os, int level) const {
  os << indent(level) << "Project " << quote(name()) << "\n";
  for (const auto &par : parameters())
    par.second->output(os, level + 1);
  for (const auto &conf : configurations())
    conf.second->output(os, level + 1);
  for (const auto &tt : tensortypes())
    tt.second->output(os, level + 1);
  for (const auto &m : manifolds())
    m.second->output(os, level + 1);
  for (const auto &ts : tangentspaces())
    ts.second->output(os, level + 1);
  for (const auto &f : fields())
    f.second->output(os, level + 1);
  for (const auto &cs : coordinatesystems())
    cs.second->output(os, level + 1);
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Project::insertEnumField(const H5::EnumType &type, const string &name,
                              int value) {
  type.insert(name, &value);
}
#endif
void Project::createTypes() const {
#ifdef SIMULATIONIO_HAVE_HDF5
  enumtype = H5::EnumType(H5::getType(int{}));
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

  auto double_type = H5::getType(double{});
  typedef long long long_long;
  auto int_type = H5::getType(long_long{});

#if 0
  // A range is described by its minimum (inclusive), maximum (inclusive), and
  // count (non-negative). Here we use double precision for all three fields,
  // but other precisions are also possible. We use a floating point number to
  // describe the count for uniformity.
  rangetype = H5::CompType(sizeof(range_t));
  rangetype.insertMember("minimum", offsetof(range_t, minimum), double_type);
  rangetype.insertMember("maximum", offsetof(range_t, maximum), double_type);
  rangetype.insertMember("count", offsetof(range_t, count), double_type);
#endif

  // point, box, and region are defined in RegionCalculus.hpp
  pointtypes.clear();
  boxtypes.clear();
  regiontypes.clear();
  for (int d = 0; d <= 4; ++d) {
    // point_t
    auto pointtype = [&] {
      const hsize_t dim = max(1, d); // HDF5 requires at least 1 element
      const hsize_t dims[1] = {dim};
      // TODO: Handle d==0 correctly
      return H5::ArrayType(int_type, 1, dims);
    }();
    pointtypes.push_back(pointtype);
    // box_t
    auto boxtype = [&] {
      vector<size_t> offsets;
      size_t size = 0;
      if (d == 0) {
        offsets.push_back(size);
        auto inttype = H5::getType(int{});
        size += inttype.getSize();
        auto boxtype = H5::CompType(size);
        boxtype.insertMember("full", offsets.at(0), inttype);
        return boxtype;
      } else {
        offsets.push_back(size);
        size += pointtype.getSize();
        offsets.push_back(size);
        size += pointtype.getSize();
        auto boxtype = H5::CompType(size);
        boxtype.insertMember("lower", offsets.at(0), pointtype);
        boxtype.insertMember("upper", offsets.at(1), pointtype);
        return boxtype;
      }
    }();
    boxtypes.push_back(boxtype);
    // region_t
    auto regiontype = H5::VarLenType(&boxtype);
    regiontypes.push_back(regiontype);

    // TODO: Move these to Buffer.cpp
    // linearization_t
    auto linearizationtype = [&] {
      vector<size_t> offsets;
      size_t size = 0;
      offsets.push_back(size);
      size += boxtype.getSize();
      offsets.push_back(size);
      size += int_type.getSize();
      auto type = H5::CompType(size);
      type.insertMember("box", offsets.at(0), boxtype);
      type.insertMember("pos", offsets.at(1), int_type);
      return type;
    }();
    linearizationtypes.push_back(linearizationtype);
    // concatenation_t
    auto concatenationtype = [&] {
      auto type = H5::VarLenType(&linearizationtype);
      return type;
    }();
    concatenationtypes.push_back(concatenationtype);
  }
#endif
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Project::write(const H5::H5Location &loc,
                    const H5::H5Location &parent) const {
  assert(invariant());
  // auto group = loc.createGroup(name());
  auto group = loc.openGroup(".");
  createTypes();
  auto typegroup = group.createGroup("types");
  enumtype.commit(typegroup, "SimulationIO");
#if 0
  rangetype.commit(typegroup, "Range");
#endif
  for (int d = 0; d < int(pointtypes.size()); ++d)
    pointtypes.at(d).commit(typegroup, "Point[" + to_string(d) + "]");
  for (int d = 0; d < int(boxtypes.size()); ++d)
    boxtypes.at(d).commit(typegroup, "Box[" + to_string(d) + "]");
  for (int d = 0; d < int(regiontypes.size()); ++d)
    regiontypes.at(d).commit(typegroup, "Region[" + to_string(d) + "]");
  H5::createAttribute(group, "type", enumtype, "Project");
  H5::createAttribute(group, "name", name());
  // no link to parent
  H5::createGroup(group, "parameters", parameters());
  H5::createGroup(group, "configurations", configurations());
  H5::createGroup(group, "tensortypes", tensortypes());
  H5::createGroup(group, "manifolds", manifolds());
  H5::createGroup(group, "tangentspaces", tangentspaces());
  H5::createGroup(group, "fields", fields());
  H5::createGroup(group, "coordinatesystems", coordinatesystems());
}

void Project::writeHDF5(const string &filename) const {
  auto fapl = H5::FileAccPropList();
  // fapl.setFcloseDegree(H5F_CLOSE_STRONG);
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file =
      H5::H5File(filename, H5F_ACC_EXCL, H5::FileCreatPropList::DEFAULT, fapl);
  write(file);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Project::yaml_path() const { return {name()}; }

ASDF::writer &Project::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.group("parameters", parameters());
  aw.group("configurations", configurations());
  aw.group("tensortypes", tensortypes());
  aw.group("manifolds", manifolds());
  aw.group("tangentspaces", tangentspaces());
  aw.group("fields", fields());
  aw.group("coordinatesystems", coordinatesystems());
  return w;
}

void Project::writeASDF(ostream &file) const {
  map<string, string> tags{
      {"sio", "tag:github.com/eschnett/SimulationIO/asdf-cxx/"}};
  map<string, function<void(ASDF::writer & w)>> funs{
      {name(), [&](ASDF::writer &w) { w << *this; }}};
  const auto &doc = ASDF::asdf(move(tags), move(funs));
  doc.write(file);
}

void Project::writeASDF(const string &filename) const {
  ofstream os(filename, ios::binary | ios::trunc | ios::out);
  writeASDF(os);
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> Project::tiledb_path() const { return {m_tiledb_filename}; }

void Project::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_group("parameters", parameters());
  w.add_group("configurations", configurations());
  w.add_group("tensortypes", tensortypes());
  w.add_group("manifolds", manifolds());
  w.add_group("tangentspaces", tangentspaces());
  w.add_group("fields", fields());
  w.add_group("coordinatesystems", coordinatesystems());
}

void Project::writeTileDB(const string &filename) const {
  tiledb::Config config;
  config.set("vfs.num_threads", "1"); // TODO: let the caller choose this

  tiledb::Context ctx(config);
  m_tiledb_filename = filename;
  write(ctx, filename);
}
#endif

shared_ptr<Parameter> Project::createParameter(const string &name) {
  auto parameter = Parameter::create(name, shared_from_this());
  checked_emplace(m_parameters, parameter->name(), parameter, "Project",
                  "parameters");
  assert(parameter->invariant());
  return parameter;
}

shared_ptr<Parameter> Project::getParameter(const string &name) {
  auto loc = m_parameters.find(name);
  if (loc != m_parameters.end()) {
    const auto &parameter = loc->second;
    return parameter;
  }
  return createParameter(name);
}

shared_ptr<Parameter>
Project::copyParameter(const shared_ptr<Parameter> &parameter,
                       bool copy_children) {
  auto parameter2 = getParameter(parameter->name());
  if (copy_children) {
    for (const auto &parametervalue_kv : parameter->parametervalues()) {
      const auto &parametervalue = parametervalue_kv.second;
      auto parametervalue2 =
          parameter2->copyParameterValue(parametervalue, copy_children);
    }
  }
  return parameter2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Parameter> Project::readParameter(const H5::H5Location &loc,
                                             const string &entry) {
  auto parameter = Parameter::create(loc, entry, shared_from_this());
  checked_emplace(m_parameters, parameter->name(), parameter, "Project",
                  "parameters");
  assert(parameter->invariant());
  return parameter;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Parameter>
Project::readParameter(const shared_ptr<ASDF::reader_state> &rs,
                       const YAML::Node &node) {
  auto parameter = Parameter::create(rs, node, shared_from_this());
  checked_emplace(m_parameters, parameter->name(), parameter, "Project",
                  "parameters");
  assert(parameter->invariant());
  return parameter;
}

shared_ptr<Parameter>
Project::getParameter(const shared_ptr<ASDF::reader_state> &rs,
                      const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "parameters");
  const auto &parameter_name = path.at(2);
  return parameters().at(parameter_name);
}
#endif

shared_ptr<Configuration> Project::createConfiguration(const string &name) {
  auto configuration = Configuration::create(name, shared_from_this());
  checked_emplace(m_configurations, configuration->name(), configuration,
                  "Project", "configurations");
  assert(configuration->invariant());
  return configuration;
}

shared_ptr<Configuration> Project::getConfiguration(const string &name) {
  auto loc = m_configurations.find(name);
  if (loc != m_configurations.end()) {
    const auto &configuration = loc->second;
    return configuration;
  }
  return createConfiguration(name);
}

shared_ptr<Configuration>
Project::copyConfiguration(const shared_ptr<Configuration> &configuration,
                           bool copy_children) {
  auto configuration2 = getConfiguration(configuration->name());
  for (const auto &parametervalue_kv : configuration->parametervalues()) {
    const auto &parametervalue = parametervalue_kv.second;
    auto parameter2 = copyParameter(parametervalue->parameter());
    auto parametervalue2 = parameter2->copyParameterValue(parametervalue);
    if (!configuration2->parametervalues().count(parametervalue2->name()))
      configuration2->insertParameterValue(parametervalue2);
  }
  return configuration2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Configuration> Project::readConfiguration(const H5::H5Location &loc,
                                                     const string &entry) {
  auto configuration = Configuration::create(loc, entry, shared_from_this());
  checked_emplace(m_configurations, configuration->name(), configuration,
                  "Project", "configurations");
  assert(configuration->invariant());
  return configuration;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Configuration>
Project::readConfiguration(const shared_ptr<ASDF::reader_state> &rs,
                           const YAML::Node &node) {
  auto configuration = Configuration::create(rs, node, shared_from_this());
  checked_emplace(m_configurations, configuration->name(), configuration,
                  "Project", "configurations");
  assert(configuration->invariant());
  return configuration;
}

shared_ptr<Configuration>
Project::getConfiguration(const shared_ptr<ASDF::reader_state> &rs,
                          const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "configurations");
  const auto &configuration_name = path.at(2);
  return configurations().at(configuration_name);
}
#endif

shared_ptr<TensorType> Project::createTensorType(const string &name,
                                                 int dimension, int rank) {
  auto tensortype =
      TensorType::create(name, shared_from_this(), dimension, rank);
  checked_emplace(m_tensortypes, tensortype->name(), tensortype, "Project",
                  "tensortypes");
  assert(tensortype->invariant());
  return tensortype;
}

shared_ptr<TensorType> Project::getTensorType(const string &name, int dimension,
                                              int rank) {
  auto loc = m_tensortypes.find(name);
  if (loc != m_tensortypes.end()) {
    const auto &tensortype = loc->second;
    assert(tensortype->dimension() == dimension);
    assert(tensortype->rank() == rank);
    return tensortype;
  }
  return createTensorType(name, dimension, rank);
}

shared_ptr<TensorType>
Project::copyTensorType(const shared_ptr<TensorType> &tensortype,
                        bool copy_children) {
  auto tensortype2 = getTensorType(tensortype->name(), tensortype->dimension(),
                                   tensortype->rank());
  if (copy_children) {
    for (const auto &tensorcomponent_kv : tensortype->tensorcomponents()) {
      const auto &tensorcomponent = tensorcomponent_kv.second;
      auto tensorcomponent2 =
          tensortype2->copyTensorComponent(tensorcomponent, copy_children);
    }
  }
  return tensortype2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<TensorType> Project::readTensorType(const H5::H5Location &loc,
                                               const string &entry) {
  auto tensortype = TensorType::create(loc, entry, shared_from_this());
  checked_emplace(m_tensortypes, tensortype->name(), tensortype, "Project",
                  "tensortypes");
  assert(tensortype->invariant());
  return tensortype;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<TensorType>
Project::readTensorType(const shared_ptr<ASDF::reader_state> &rs,
                        const YAML::Node &node) {
  auto tensortype = TensorType::create(rs, node, shared_from_this());
  checked_emplace(m_tensortypes, tensortype->name(), tensortype, "Project",
                  "tensortypes");
  assert(tensortype->invariant());
  return tensortype;
}

shared_ptr<TensorType>
Project::getTensorType(const shared_ptr<ASDF::reader_state> &rs,
                       const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "tensortypes");
  const auto &tensortype_name = path.at(2);
  return tensortypes().at(tensortype_name);
}
#endif

shared_ptr<Manifold>
Project::createManifold(const string &name,
                        const shared_ptr<Configuration> &configuration,
                        int dimension) {
  assert(configuration->project().get() == this);
  auto manifold =
      Manifold::create(name, shared_from_this(), configuration, dimension);
  checked_emplace(m_manifolds, manifold->name(), manifold, "Project",
                  "manifolds");
  assert(manifold->invariant());
  return manifold;
}

shared_ptr<Manifold>
Project::getManifold(const string &name,
                     const shared_ptr<Configuration> &configuration,
                     int dimension) {
  auto loc = m_manifolds.find(name);
  if (loc != m_manifolds.end()) {
    const auto &manifold = loc->second;
    assert(manifold->configuration() == configuration);
    assert(manifold->dimension() == dimension);
    return manifold;
  }
  return createManifold(name, configuration, dimension);
}

shared_ptr<Manifold> Project::copyManifold(const shared_ptr<Manifold> &manifold,
                                           bool copy_children) {
  auto configuration2 = copyConfiguration(manifold->configuration());
  auto manifold2 =
      getManifold(manifold->name(), configuration2, manifold->dimension());
  if (copy_children) {
    for (const auto &discretization_kv : manifold->discretizations()) {
      const auto &discretization = discretization_kv.second;
      auto discretization2 =
          manifold2->copyDiscretization(discretization, copy_children);
    }
  }
  return manifold2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Manifold> Project::readManifold(const H5::H5Location &loc,
                                           const string &entry) {
  auto manifold = Manifold::create(loc, entry, shared_from_this());
  checked_emplace(m_manifolds, manifold->name(), manifold, "Project",
                  "manifolds");
  assert(manifold->invariant());
  return manifold;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Manifold>
Project::readManifold(const shared_ptr<ASDF::reader_state> &rs,
                      const YAML::Node &node) {
  auto manifold = Manifold::create(rs, node, shared_from_this());
  checked_emplace(m_manifolds, manifold->name(), manifold, "Project",
                  "manifolds");
  assert(manifold->invariant());
  return manifold;
}

shared_ptr<Manifold>
Project::getManifold(const shared_ptr<ASDF::reader_state> &rs,
                     const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "manifolds");
  const auto &manifold_name = path.at(2);
  return manifolds().at(manifold_name);
}
#endif

shared_ptr<TangentSpace>
Project::createTangentSpace(const string &name,
                            const shared_ptr<Configuration> &configuration,
                            int dimension) {
  assert(configuration->project().get() == this);
  auto tangentspace =
      TangentSpace::create(name, shared_from_this(), configuration, dimension);
  checked_emplace(m_tangentspaces, tangentspace->name(), tangentspace,
                  "Project", "tangentspaces");
  assert(tangentspace->invariant());
  return tangentspace;
}

shared_ptr<TangentSpace>
Project::getTangentSpace(const string &name,
                         const shared_ptr<Configuration> &configuration,
                         int dimension) {
  auto loc = m_tangentspaces.find(name);
  if (loc != m_tangentspaces.end()) {
    const auto &tangentspace = loc->second;
    assert(tangentspace->configuration() == configuration);
    assert(tangentspace->dimension() == dimension);
    return tangentspace;
  }
  return createTangentSpace(name, configuration, dimension);
}

shared_ptr<TangentSpace>
Project::copyTangentSpace(const shared_ptr<TangentSpace> &tangentspace,
                          bool copy_children) {
  auto configuration2 = copyConfiguration(tangentspace->configuration());
  auto tangentspace2 = getTangentSpace(tangentspace->name(), configuration2,
                                       tangentspace->dimension());
  if (copy_children) {
    for (const auto &basis_kv : tangentspace->bases()) {
      const auto &basis = basis_kv.second;
      auto basis2 = tangentspace2->copyBasis(basis, copy_children);
    }
  }
  return tangentspace2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<TangentSpace> Project::readTangentSpace(const H5::H5Location &loc,
                                                   const string &entry) {
  auto tangentspace = TangentSpace::create(loc, entry, shared_from_this());
  checked_emplace(m_tangentspaces, tangentspace->name(), tangentspace,
                  "Project", "tangentspaces");
  assert(tangentspace->invariant());
  return tangentspace;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<TangentSpace>
Project::readTangentSpace(const shared_ptr<ASDF::reader_state> &rs,
                          const YAML::Node &node) {
  auto tangentspace = TangentSpace::create(rs, node, shared_from_this());
  checked_emplace(m_tangentspaces, tangentspace->name(), tangentspace,
                  "Project", "tangentspaces");
  assert(tangentspace->invariant());
  return tangentspace;
}

shared_ptr<TangentSpace>
Project::getTangentSpace(const shared_ptr<ASDF::reader_state> &rs,
                         const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "tangentspaces");
  const auto &tangentspace_name = path.at(2);
  return tangentspaces().at(tangentspace_name);
}
#endif

shared_ptr<Field>
Project::createField(const string &name,
                     const shared_ptr<Configuration> &configuration,
                     const shared_ptr<Manifold> &manifold,
                     const shared_ptr<TangentSpace> &tangentspace,
                     const shared_ptr<TensorType> &tensortype) {
  assert(configuration->project().get() == this);
  assert(manifold->project().get() == this);
  assert(tangentspace->project().get() == this);
  assert(tensortype->project().get() == this);
  auto field = Field::create(name, shared_from_this(), configuration, manifold,
                             tangentspace, tensortype);
  checked_emplace(m_fields, field->name(), field, "Project", "fields");
  assert(field->invariant());
  return field;
}

shared_ptr<Field>
Project::getField(const string &name,
                  const shared_ptr<Configuration> &configuration,
                  const shared_ptr<Manifold> &manifold,
                  const shared_ptr<TangentSpace> &tangentspace,
                  const shared_ptr<TensorType> &tensortype) {
  auto loc = m_fields.find(name);
  if (loc != m_fields.end()) {
    const auto &field = loc->second;
    assert(field->configuration() == configuration);
    assert(field->manifold() == manifold);
    assert(field->tangentspace() == tangentspace);
    assert(field->tensortype() == tensortype);
    return field;
  }
  return createField(name, configuration, manifold, tangentspace, tensortype);
}

shared_ptr<Field> Project::copyField(const shared_ptr<Field> &field,
                                     bool copy_children) {
  auto configuration2 = copyConfiguration(field->configuration());
  auto manifold2 = copyManifold(field->manifold());
  auto tangentspace2 = copyTangentSpace(field->tangentspace());
  auto tensortype2 = copyTensorType(field->tensortype());
  auto field2 = getField(field->name(), configuration2, manifold2,
                         tangentspace2, tensortype2);
  if (copy_children) {
    for (const auto &discretefield_kv : field->discretefields()) {
      const auto &discretefield = discretefield_kv.second;
      auto discretefield2 =
          field2->copyDiscreteField(discretefield, copy_children);
    }
  }
  return field2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Field> Project::readField(const H5::H5Location &loc,
                                     const string &entry) {
  auto field = Field::create(loc, entry, shared_from_this());
  checked_emplace(m_fields, field->name(), field, "Project", "fields");
  assert(field->invariant());
  return field;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Field> Project::readField(const shared_ptr<ASDF::reader_state> &rs,
                                     const YAML::Node &node) {
  auto field = Field::create(rs, node, shared_from_this());
  checked_emplace(m_fields, field->name(), field, "Project", "fields");
  assert(field->invariant());
  return field;
}

shared_ptr<Field> Project::getField(const shared_ptr<ASDF::reader_state> &rs,
                                    const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == name());
  assert(path.at(1) == "fields");
  const auto &field_name = path.at(2);
  return fields().at(field_name);
}
#endif

shared_ptr<CoordinateSystem>
Project::createCoordinateSystem(const string &name,
                                const shared_ptr<Configuration> &configuration,
                                const shared_ptr<Manifold> &manifold) {
  auto coordinatesystem = CoordinateSystem::create(name, shared_from_this(),
                                                   configuration, manifold);
  checked_emplace(m_coordinatesystems, coordinatesystem->name(),
                  coordinatesystem, "Project", "coordinatesystems");
  assert(coordinatesystem->invariant());
  return coordinatesystem;
}

shared_ptr<CoordinateSystem>
Project::getCoordinateSystem(const string &name,
                             const shared_ptr<Configuration> &configuration,
                             const shared_ptr<Manifold> &manifold) {
  auto loc = m_coordinatesystems.find(name);
  if (loc != m_coordinatesystems.end()) {
    const auto &coordinatesystem = loc->second;
    assert(coordinatesystem->configuration() == configuration);
    assert(coordinatesystem->manifold() == manifold);
    return coordinatesystem;
  }
  return createCoordinateSystem(name, configuration, manifold);
}

shared_ptr<CoordinateSystem> Project::copyCoordinateSystem(
    const shared_ptr<CoordinateSystem> &coordinatesystem, bool copy_children) {
  auto configuration2 = copyConfiguration(coordinatesystem->configuration());
  auto manifold2 = copyManifold(coordinatesystem->manifold());
  auto coordinatesystem2 =
      getCoordinateSystem(coordinatesystem->name(), configuration2, manifold2);
  if (copy_children) {
    for (const auto &coordinatefield_kv :
         coordinatesystem->coordinatefields()) {
      const auto &coordinatefield = coordinatefield_kv.second;
      auto coordinatefield2 = coordinatesystem2->copyCoordinateField(
          coordinatefield, copy_children);
    }
  }
  return coordinatesystem2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<CoordinateSystem>
Project::readCoordinateSystem(const H5::H5Location &loc, const string &entry) {
  auto coordinatesystem =
      CoordinateSystem::create(loc, entry, shared_from_this());
  checked_emplace(m_coordinatesystems, coordinatesystem->name(),
                  coordinatesystem, "Project", "coordinatesystems");
  assert(coordinatesystem->invariant());
  return coordinatesystem;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<CoordinateSystem>
Project::readCoordinateSystem(const shared_ptr<ASDF::reader_state> &rs,
                              const YAML::Node &node) {
  auto coordinatesystem =
      CoordinateSystem::create(rs, node, shared_from_this());
  checked_emplace(m_coordinatesystems, coordinatesystem->name(),
                  coordinatesystem, "Project", "coordinatesystems");
  assert(coordinatesystem->invariant());
  return coordinatesystem;
}
#endif

} // namespace SimulationIO
