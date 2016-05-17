#include "SimulationIO.hpp"

#include "RegionCalculus.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;
using namespace std;

typedef RegionCalculus::dpoint<hssize_t> dpoint;
typedef RegionCalculus::dbox<hssize_t> dbox;
typedef RegionCalculus::dregion<hssize_t> dregion;

const char *const dirnames[] = {"x", "y", "z"};

// Output field widths for various quantities
const int width_it = 10, width_tl = 1, width_m = 3, width_rl = 2, width_c = 8;

bool startswith(const string &str, const string &prefix) {
  return str.substr(0, prefix.length()) == prefix;
}

string get_basename(string filename) {
  auto dotpos = filename.rfind('.');
  if (dotpos != string::npos)
    filename = filename.substr(0, dotpos);
  auto slashpos = filename.rfind('/');
  if (slashpos != string::npos)
    filename = filename.substr(slashpos + 1);
  return filename;
}

class expect {
  string str;

public:
  expect(const string &str) : str(str) {}
  expect(char ch) : str(1, ch) {}
  istream &input(istream &is) const {
    assert(bool(is));
    for (auto ch : str) {
      if (is.eof())
        throw "Unexpected end of input";
      char got = is.get();
      assert(got == ch);
      if (got != ch)
        throw "Unexpectec input";
      assert(bool(is));
    }
    return is;
  }
  friend istream &operator>>(istream &is, const expect &self) {
    return self.input(is);
  }
};

class skipuntil {
  char ch;

public:
  skipuntil(char ch) : ch(ch) {}
  istream &input(istream &is) const {
    assert(is.good());
    while (!is.eof() && is.peek() != ch) {
      is.get();
      assert(bool(is));
    }
    assert(is.good());
    is.get();
    assert(bool(is));
    return is;
  }
  friend istream &operator>>(istream &is, const skipuntil &self) {
    return self.input(is);
  }
};

template <typename T, int D> struct vect {
  array<T, D> elts;
  istream &input(istream &is) {
    is >> expect('[');
    for (int d = 0; d < D; ++d) {
      if (d > 0)
        is >> expect(',');
      is >> elts[d];
    }
    is >> expect(']');
    return is;
  }
  ostream &output(ostream &os) const {
    os << "[";
    for (int d = 0; d < D; ++d) {
      if (d > 0)
        os << ",";
      os << elts[d];
    }
    os << "]";
    return os;
  }
  friend istream &operator>>(istream &is, vect<T, D> &v) { return v.input(is); }
  friend ostream &operator<<(ostream &os, const vect<T, D> &v) {
    return v.output(os);
  }
};

template <typename T, int D> struct bbox {
  vect<T, D> lower, upper, stride;
  istream &input(istream &is) {
    vect<T, D> tmplo, tmphi, tmpsh;
    long long tmpsz;
    is >> expect('(') >> lower >> expect(':') >> upper >> expect(':') >>
        stride >> expect('/') >> tmplo >> expect(':') >> tmphi >> expect('/') >>
        tmpsh >> expect('/') >> tmpsz >> expect(')');
    return is;
  }
  ostream &output(ostream &os) const {
    return os << "(" << lower << ":" << upper << ":" << stride << ")";
  }
  friend istream &operator>>(istream &is, bbox<T, D> &b) { return b.input(is); }
  friend ostream &operator<<(ostream &os, const bbox<T, D> &b) {
    return b.output(os);
  }
};

template <typename T, int D> struct bboxset {
  vector<bbox<T, D>> elts;
  istream &input(istream &is) {
    elts.clear();
    is >> expect("bboxset<") >> skipuntil('{');
    if (is.peek() == '(') {
      char got;
      do {
        bbox<T, D> elt;
        is >> elt;
        elts.push_back(elt);
        got = is.get();
      } while (got == ',');
      assert(got == '}');
    }
    is >> skipuntil(')');
    return is;
  }
  ostream &output(ostream &os) const {
    os << "bboxset{";
    for (size_t i = 0; i < elts.size(); ++i) {
      if (i > 0)
        os << ",";
      os << elts[i];
    }
    os << "}";
    return os;
  }
  friend istream &operator>>(istream &is, bboxset<T, D> &bs) {
    return bs.input(is);
  }
  friend ostream &operator<<(ostream &os, const bboxset<T, D> &bs) {
    return bs.output(os);
  }
};

typedef vect<hssize_t, 3> ivect;
typedef bbox<hssize_t, 3> ibbox;
typedef bboxset<hssize_t, 3> ibboxset;

int main(int argc, char **argv) {

  bool have_error = false;
  enum { action_unset, action_copy, action_extlink } action = action_unset;
  string outputfilename;
  vector<string> inputfilenames;

  for (int argi = 1; argi < argc; ++argi) {
    auto argvi = string(argv[argi]);
    if (argvi.empty()) {
      have_error = true;
      break;
    } else if (argvi[0] == '-') {
      if (argvi == "--copy") {
        if (action != action_unset) {
          have_error = true;
          break;
        }
        action = action_copy;
      } else if (argvi == "--extlink") {
        if (action != action_unset) {
          have_error = true;
          break;
        }
        action = action_extlink;
      } else {
        have_error = true;
        break;
      }
    } else {
      if (outputfilename.empty())
        outputfilename = argvi;
      else
        inputfilenames.push_back(argvi);
    }
  }
  if (inputfilenames.empty()) {
    have_error = true;
  }
  if (have_error) {
    cerr << "Synposis:\n"
         << argv[0]
         << " [--copy|--extlink] <output file name> {<input file name>}\n";
    return 1;
  }
  if (action == action_unset)
    action = action_copy;

  const string basename = get_basename(outputfilename);
  assert(!basename.empty());

  // Project
  const string projectname = basename;
  auto project = createProject(projectname);
  // Parameters
  auto parameter_iteration = project->createParameter("iteration");
  auto parameter_timelevel = project->createParameter("timelevel");
  // Configuration
  auto global_configuration = project->createConfiguration("global");
  // TensorTypes
  project->createStandardTensorTypes();
  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", global_configuration, dim);
  auto tangentspace =
      project->createTangentSpace("space", global_configuration, dim);
  // Discretization for Manifold
  map<string, map<int, vector<shared_ptr<Discretization>>>>
      discretizations; // [configuration][mapindex][reflevel]
  map<string, map<int, map<int, vector<double>>>> ideltas,
      ioffsets; // [configuration][mapindex][reflevel]
  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian", global_configuration);
  for (int d = 0; d < dim; ++d) {
    basis->createBasisVector(dirnames[d], d);
  }

  for (const auto &inputfilename : inputfilenames) {
    cout << "Reading file " << inputfilename << "...\n";

    auto inputfile = H5::H5File(inputfilename, H5F_ACC_RDONLY);
    hsize_t idx = 0;
    H5::iterateElems(
        inputfile, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
        [&](const H5::Group &group, const std::string &name,
            const H5L_info_t *info) {
          if (name == "Parameters and Global Attributes") {
            cout << "  skipping " << name << "\n";
            return 0;
          }
          cout << "  opening dataset " << name << "...\n";
          auto dataset = group.openDataSet(name);
          // Parse dataset name
          istringstream namestream(name);
          vector<string> tokens;
          copy(istream_iterator<string>(namestream), istream_iterator<string>(),
               back_inserter(tokens));
          // const string varname = tokens.at(0);
          const string varname = H5::readAttribute<string>(dataset, "name");
          tokens.erase(tokens.begin());
          // Determine field name and tensor type
          string fieldname(varname);
          vector<int> tensorindices;
          if (startswith(fieldname, "GRID:")) {
            // Special case for coordinates: do nothing, treat them as
            // scalars
          } else if (startswith(fieldname, "RADHYDRO2::ustate")) {
            // Special case, treat these variables as scalars
            // TODO: Handle this in a clean way
          } else {
            // There are three different conventions to represent
            // tensors in Cactus:
            // 1. Suffix x, y, z
            // 2. Suffix [0], [1], [2]
            // 3. Suffix 1, 2, 3
            ptrdiff_t pos = fieldname.length() - 1;
            if (pos >= 0) {
              if (fieldname[pos] >= 'x' && fieldname[pos] <= 'z') {
                while (pos >= 0 && fieldname[pos] >= 'x' &&
                       fieldname[pos] <= 'z') {
                  int ti = fieldname[pos] - 'x';
                  assert(ti >= 0 && ti < dim);
                  tensorindices.push_back(ti);
                  --pos;
                }
              } else if (fieldname[pos] >= '1' && fieldname[pos] <= '9') {
                while (pos >= 0 && fieldname[pos] >= '1' &&
                       fieldname[pos] <= '9') {
                  int ti = fieldname[pos] - '1';
                  assert(ti >= 0 && ti < dim);
                  tensorindices.push_back(ti);
                  --pos;
                }
              } else if (fieldname[pos] == ']') {
                --pos;
                assert(pos >= 0);
                assert(fieldname[pos] >= '0' && fieldname[pos] <= '9');
                int ti = fieldname[pos] - '0';
                assert(ti >= 0 && ti < dim);
                tensorindices.push_back(ti);
                --pos;
                assert(pos >= 0);
                assert(fieldname[pos] == '[');
                --pos;
              }
            }
            while (pos >= 0 && fieldname[pos] == ':')
              --pos;
            assert(pos >= 0);
            fieldname = fieldname.substr(0, pos + 1);
          }
          reverse(tensorindices.begin(), tensorindices.end());
          const int tensorrank = tensorindices.size();
          string tensortypename;
          if (tensorrank == 0)
            tensortypename = "Scalar3D";
          else if (tensorrank == 1)
            tensortypename = "Vector3D";
          else if (tensorrank == 2)
            tensortypename = "SymmetricTensor3D";
          else
            assert(0);
          // Determine iteration, time level, map index, refinement level
          int iteration = 0, timelevel = 0, mapindex = 0, refinementlevel = 0,
              component = 0;
          bool is_multiblock = false, is_amr = false;
          for (const auto &token : tokens) {
            auto eqpos = token.find('=');
            string id = token.substr(0, eqpos);
            string val = token.substr(eqpos + 1);
            int *variable = 0;
            if (id == "it")
              variable = &iteration;
            else if (id == "tl")
              variable = &timelevel;
            else if (id == "m")
              variable = &mapindex, is_multiblock = true;
            else if (id == "rl")
              variable = &refinementlevel, is_amr = true;
            else if (id == "c")
              variable = &component;
            else
              assert(0);
            istringstream buf(val);
            assert(!buf.eof());
            buf >> *variable;
            assert(buf.eof());
          }

          // metadata
          assert(H5::readAttribute<int>(dataset, "group_timelevel") ==
                 timelevel);
          assert(H5::readAttribute<int>(dataset, "level") == refinementlevel);
          // local coordinates
          auto origin = H5::readAttribute<vector<double>>(dataset, "origin");
          assert(int(origin.size()) == manifold->dimension);
          auto delta = H5::readAttribute<vector<double>>(dataset, "delta");
          assert(int(delta.size()) == manifold->dimension);
          // subdiscretizations
          vector<double> idelta(manifold->dimension);
          for (int d = 0; d < int(idelta.size()); ++d)
            idelta.at(d) = double(delta.at(d));
          vector<double> ioffset(manifold->dimension);
          auto ioffsetnum =
              H5::readAttribute<vector<hssize_t>>(dataset, "ioffset");
          auto ioffsetdenom =
              H5::readAttribute<vector<hssize_t>>(dataset, "ioffsetdenom");
          assert(int(ioffsetnum.size()) == manifold->dimension);
          assert(int(ioffsetdenom.size()) == manifold->dimension);
          for (int d = 0; d < int(ioffset.size()); ++d)
            ioffset.at(d) =
                double(ioffsetnum.at(d)) / double(ioffsetdenom.at(d));
          // active region
          dregion active;
          if (dataset.attrExists("active")) {
            string active_str = H5::readAttribute<string>(dataset, "active");
            istringstream ibuf(active_str);
            ibboxset active_bs;
            ibuf >> active_bs;
            vector<dbox> dbs;
            for (const ibbox &b : active_bs.elts) {
              dpoint lo(b.lower.elts);
              dpoint hi(b.upper.elts);
              const dpoint str(b.stride.elts);
              hi += str;
              const dpoint poffsetnum(ioffsetnum);
              const dpoint poffsetdenom(ioffsetdenom);
              assert(all(!(str % poffsetdenom)));
              lo -= str * poffsetnum / poffsetdenom;
              hi -= str * poffsetnum / poffsetdenom;
              assert(all(!(lo % str) && !(hi % str)));
              const dbox db(lo / str, hi / str);
              dbs.push_back(db);
            }
            active = dregion(dbs);
          }

          // Output information
          cout << "    field name: " << fieldname << "\n"
               << "    tensor rank: " << tensorrank << "\n"
               << "    tensor type: " << tensortypename << "\n"
               << "    tensor indices: " << tensorindices << "\n"
               << "    iteration: " << iteration << "\n"
               << "    time level: " << timelevel << "\n"
               << "    map: " << mapindex << "\n"
               << "    refinement level: " << refinementlevel << "\n"
               << "    component: " << component << "\n";

          // Get configuration
          string value_iteration_name = [&] {
            ostringstream buf;
            buf << parameter_iteration->name << "." << setfill('0')
                << setw(width_it) << iteration;
            return buf.str();
          }();
          string value_timelevel_name = [&] {
            ostringstream buf;
            buf << parameter_timelevel->name << "." << setfill('0')
                << setw(width_tl) << timelevel;
            return buf.str();
          }();
          auto configurationname =
              value_iteration_name + "-" + value_timelevel_name;
          if (!project->configurations.count(configurationname)) {
            auto configuration =
                project->createConfiguration(configurationname);
            if (!parameter_iteration->parametervalues.count(
                    value_iteration_name)) {
              auto value_iteration = parameter_iteration->createParameterValue(
                  value_iteration_name);
              value_iteration->setValue(iteration);
            }
            auto value_iteration =
                parameter_iteration->parametervalues.at(value_iteration_name);
            configuration->insertParameterValue(value_iteration);
            if (!parameter_timelevel->parametervalues.count(
                    value_timelevel_name)) {
              auto value_timelevel = parameter_timelevel->createParameterValue(
                  value_timelevel_name);
              value_timelevel->setValue(timelevel);
            }
            auto value_timelevel =
                parameter_timelevel->parametervalues.at(value_timelevel_name);
            configuration->insertParameterValue(value_timelevel);
          }
          auto configuration = project->configurations.at(configurationname);

          // Get tensor type
          auto tensortype = project->tensortypes.at(tensortypename);
          assert(tensortype->rank == tensorrank);
          shared_ptr<TensorComponent> tensorcomponent;
          for (const auto &tc : tensortype->tensorcomponents) {
            if (tc.second->indexvalues == tensorindices) {
              tensorcomponent = tc.second;
              break;
            }
          }
          assert(tensorcomponent);

          // Get discretization
          ideltas[configuration->name][mapindex][refinementlevel] = idelta;
          ioffsets[configuration->name][mapindex][refinementlevel] = ioffset;
          if (!discretizations.count(configuration->name))
            discretizations[configuration->name];
          if (!discretizations.at(configuration->name).count(mapindex))
            discretizations.at(configuration->name)[mapindex];
          while (int(discretizations.at(configuration->name)
                         .at(mapindex)
                         .size()) <= refinementlevel) {
            const int rl =
                discretizations.at(configuration->name).at(mapindex).size();
            string discretizationname = [&] {
              ostringstream buf;
              buf << configuration->name;
              if (is_multiblock)
                buf << "-map." << setfill('0') << setw(width_m) << mapindex;
              if (is_amr)
                buf << "-level." << setfill('0') << setw(width_rl) << rl;
              return buf.str();
            }();
            auto discretization = manifold->createDiscretization(
                discretizationname, configuration);
            discretizations.at(configuration->name)
                .at(mapindex)
                .push_back(discretization);
          }
          auto discretization = discretizations.at(configuration->name)
                                    .at(mapindex)
                                    .at(refinementlevel);

          // Get discretization block
          string blockname = [&] {
            ostringstream buf;
            buf << "c." << setfill('0') << setw(width_c) << component;
            return buf.str();
          }();
          if (!discretization->discretizationblocks.count(blockname)) {
            auto discretizationblock =
                discretization->createDiscretizationBlock(blockname);
            vector<hssize_t> offset(dim), shape(dim);
            H5::readAttribute(dataset, "iorigin", offset);
            auto dataspace = dataset.getSpace();
            int rank = dataspace.getSimpleExtentNdims();
            assert(rank == dim);
            dataspace.getSimpleExtentDims((hsize_t *)(shape.data()));
            std::reverse(shape.begin(), shape.end());
            discretizationblock->setRegion(
                box_t(offset, point_t(offset) + shape));
            if (active.valid()) {
              discretizationblock->setActive(active);
            }
          }
          auto discretizationblock =
              discretization->discretizationblocks.at(blockname);

          // Get local coordinates
          {
            string coordinatesystemname = [&] {
              ostringstream buf;
              buf << "cctkGH.space";
              if (is_multiblock)
                buf << "-map." << setfill('0') << setw(width_m) << mapindex;
              return buf.str();
            }();
            auto tangentspacename = coordinatesystemname;
            auto basisname = tangentspacename;
            if (!project->coordinatesystems.count(coordinatesystemname)) {
              auto coordinatesystem = project->createCoordinateSystem(
                  coordinatesystemname, global_configuration, manifold);
              auto tangentspace = project->createTangentSpace(
                  tangentspacename, global_configuration, manifold->dimension);
              tangentspace->createBasis(coordinatesystemname,
                                        global_configuration);
            }
            auto coordinatesystem =
                project->coordinatesystems.at(coordinatesystemname);
            auto tangentspace = project->tangentspaces.at(tangentspacename);
            auto basis = tangentspace->bases.at(basisname);
            auto tensortype = project->tensortypes.at("Scalar3D");
            assert(tensortype->tensorcomponents.size() == 1);
            auto tensorcomponent = tensortype->tensorcomponents.begin()->second;
            for (int direction = 0; direction < tangentspace->dimension;
                 ++direction) {
              string fieldname = [&] {
                ostringstream buf;
                buf << coordinatesystemname << "[" << direction << "]";
                return buf.str();
              }();
              auto coordinatefieldname = fieldname;
              auto discretefieldname = fieldname;
              if (!project->fields.count(fieldname)) {
                auto field = project->createField(
                    coordinatefieldname, global_configuration, manifold,
                    tangentspace, tensortype);
                coordinatesystem->createCoordinateField(fieldname, direction,
                                                        field);
                field->createDiscreteField(discretefieldname,
                                           global_configuration, discretization,
                                           basis);
              }
              auto field = project->fields.at(fieldname);
              auto discretefield = field->discretefields.at(discretefieldname);

              auto discretefieldblockname = discretizationblock->name;
              auto discretefieldblockcomponentname = tensorcomponent->name;
              if (!discretefield->discretefieldblocks.count(
                      discretefieldblockname)) {
                auto discretefieldblock =
                    discretefield->createDiscreteFieldBlock(
                        discretizationblock->name, discretizationblock);
                auto discretefieldblockcomponent =
                    discretefieldblock->createDiscreteFieldBlockComponent(
                        discretefieldblockcomponentname, tensorcomponent);

                vector<hssize_t> count = discretizationblock->region.shape();
                double data_origin = origin.at(direction);
                vector<double> data_delta(manifold->dimension, 0.0);
                data_delta.at(direction) = delta.at(direction);
                discretefieldblockcomponent->setData(data_origin, data_delta);
              }
            }
          }

          // Get field
          if (!project->fields.count(fieldname))
            project->createField(fieldname, global_configuration, manifold,
                                 tangentspace, tensortype);
          auto field = project->fields.at(fieldname);

          // Get global coordinates
          if (field->name == "GRID") {
            string coordinatesystemname = [&] {
              ostringstream buf;
              buf << field->name << "-" << configuration->name;
              return buf.str();
            }();
            if (!project->coordinatesystems.count(coordinatesystemname))
              project->createCoordinateSystem(coordinatesystemname,
                                              configuration, manifold);
            auto coordinatesystem =
                project->coordinatesystems.at(coordinatesystemname);
            assert(tensortype->rank == 1);
            int direction = tensorcomponent->indexvalues.at(0);
            string coordinatefieldname = [&] {
              ostringstream buf;
              buf << coordinatesystem->name << "-" << direction;
              return buf.str();
            }();
            if (!coordinatesystem->directions.count(direction))
              coordinatesystem->createCoordinateField(coordinatefieldname,
                                                      direction, field);
          } else if (field->name == "GRID::x" || field->name == "GRID::y" ||
                     field->name == "GRID::z") {
            string coordinatesystemname = [&] {
              ostringstream buf;
              buf << "GRID-" << configuration->name;
              return buf.str();
            }();
            if (!project->coordinatesystems.count(coordinatesystemname))
              project->createCoordinateSystem(coordinatesystemname,
                                              configuration, manifold);
            auto coordinatesystem =
                project->coordinatesystems.at(coordinatesystemname);
            assert(tensortype->rank == 0);
            int direction = -1;
            if (field->name == "GRID::x")
              direction = 0;
            else if (field->name == "GRID::y")
              direction = 1;
            else if (field->name == "GRID::z")
              direction = 2;
            else
              assert(0);
            string coordinatefieldname = [&] {
              ostringstream buf;
              buf << coordinatesystem->name << "-" << *field->name.rbegin();
              return buf.str();
            }();
            if (!coordinatesystem->directions.count(direction))
              coordinatesystem->createCoordinateField(coordinatefieldname,
                                                      direction, field);
          }

          // Get discrete field
          string discretefieldname = [&] {
            ostringstream buf;
            buf << fieldname << "-" << discretization->name;
            return buf.str();
          }();
          if (!field->discretefields.count(discretefieldname))
            field->createDiscreteField(discretefieldname, configuration,
                                       discretization, basis);
          auto discretefield = field->discretefields.at(discretefieldname);
          // Get discrete field block
          if (!discretefield->discretefieldblocks.count(
                  discretizationblock->name))
            discretefield->createDiscreteFieldBlock(discretizationblock->name,
                                                    discretizationblock);
          auto discretefieldblock =
              discretefield->discretefieldblocks.at(discretizationblock->name);
          // Get discrete field block data
          if (!discretefieldblock->discretefieldblockcomponents.count(
                  tensorcomponent->name))
            discretefieldblock->createDiscreteFieldBlockComponent(
                tensorcomponent->name, tensorcomponent);
          auto discretefieldblockcomponent =
              discretefieldblock->discretefieldblockcomponents.at(
                  tensorcomponent->name);
          switch (action) {
          case action_copy:
            discretefieldblockcomponent->setData(inputfile, name);
            break;
          case action_extlink:
            discretefieldblockcomponent->setData(inputfilename, name);
            break;
          default:
            assert(0);
          }
          // Done
          return 0;
        });

    // Create subdiscretizations
    // We need to add these late since Cactus stores the refinement level
    // offsets with respect to a (virtual) global grid, whereas we need the
    // relative offsets between two refinement levels. We can only calculate
    // these after both levels have been handled.
    for (auto &i0 : ioffsets) {
      auto &configurationname = i0.first;
      auto &ioffsets1 = i0.second;
      auto ideltas1 = ideltas.at(configurationname);
      for (auto &i1 : ioffsets1) {
        const auto &mapindex = i1.first;
        auto &ioffsets2 = i1.second;
        auto &ideltas2 = ideltas1.at(mapindex);

        // Invent missing information
        int minrefinementlevel = numeric_limits<int>::max();
        for (const auto &i2 : ioffsets2) {
          const auto &refinementlevel = i2.first;
          minrefinementlevel = min(minrefinementlevel, refinementlevel);
        }
        if (minrefinementlevel != numeric_limits<int>::max()) {
          auto minioffset = ioffsets2.at(minrefinementlevel);
          auto minidelta = ideltas2.at(minrefinementlevel);
          for (int refinementlevel = 1; refinementlevel < minrefinementlevel;
               ++refinementlevel) {
            assert(!ioffsets2.count(refinementlevel));
            assert(!ideltas2.count(refinementlevel));
            ioffsets2[refinementlevel] = minioffset; // arbitrary choice
            ideltas2[refinementlevel] = minidelta;   // arbitrary choice
          }
        }

        // Create subdiscretizations
        for (const auto &i2 : ioffsets2) {
          const auto &refinementlevel = i2.first;
          const auto &ioffset = i2.second;
          const auto &idelta = ideltas2.at(refinementlevel);
          // Skip the coarsest grid that exists at this iteration
          if (ioffsets2.count(refinementlevel - 1)) {
            string subdiscretizationname = [&] {
              ostringstream buf;
              buf << configurationname << "-map." << setfill('0')
                  << setw(width_m) << mapindex << "-level." << setfill('0')
                  << setw(width_rl) << refinementlevel;
              return buf.str();
            }();
            if (!manifold->subdiscretizations.count(subdiscretizationname)) {
              // origin0 = origin + delta0 * offset0
              // origin1 = origin + delta1 * offset1
              // x0 = origin0 + i0 * delta0
              // x1 = origin1 + i1 * delta1
              // x0 = x1
              // (i1 + offset1) * delta1 = (i0 + offset0) * delta0
              // i1 = (offset0 + i0) * delta0 / delta1 - offset1
              // i1 = offset0 * delta0 / delta1 - offset1 + i0 * delta0 / delta1
              const auto &coarse_idelta = ideltas2.at(refinementlevel - 1);
              const auto &coarse_ioffset = ioffsets2.at(refinementlevel - 1);
              vector<double> factor(idelta.size());
              for (int d = 0; d < int(factor.size()); ++d)
                factor.at(d) = coarse_idelta.at(d) / idelta.at(d);
              vector<double> offset(ioffset.size());
              for (int d = 0; d < int(offset.size()); ++d)
                offset.at(d) =
                    ioffset.at(d) - factor.at(d) * coarse_ioffset.at(d);
              manifold->createSubDiscretization(
                  subdiscretizationname, discretizations.at(configurationname)
                                             .at(mapindex)
                                             .at(refinementlevel - 1),
                  discretizations.at(configurationname)
                      .at(mapindex)
                      .at(refinementlevel),
                  factor, offset);
            }
          }
        }
      }
    }
  }

  // Write file
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file = H5::H5File(outputfilename, H5F_ACC_TRUNC,
                         H5::FileCreatPropList::DEFAULT, fapl);
  project->write(file);

  return 0;
}
