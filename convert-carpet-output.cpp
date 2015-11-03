#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;
using namespace std;

const char *const dirnames[] = {"x", "y", "z"};

string get_basename(string filename) {
  auto dotpos = filename.rfind('.');
  if (dotpos != string::npos)
    filename = filename.substr(0, dotpos);
  auto slashpos = filename.rfind('/');
  if (slashpos != string::npos)
    filename = filename.substr(slashpos + 1);
  return filename;
}

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
      if (outputfilename.empty()) {
        outputfilename = argvi;
      } else {
        inputfilenames.push_back(argvi);
      }
    }
  }
  if (inputfilenames.empty()) {
    have_error = true;
  }
  if (have_error) {
    cerr << "Synposis:\n" << argv[0]
         << " [--copy|--extlink] <output file name> {<input file name>}\n";
    return 1;
  }
  if (action == action_unset) {
    action = action_copy;
  }

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
  map<int, vector<shared_ptr<Discretization>>>
      discretizations;                   // [mapindex][reflevel]
  map<int, vector<double>> root_ioffset; // [mapindex]
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
          while (!fieldname.empty() && *fieldname.rbegin() >= 'x' &&
                 *fieldname.rbegin() <= 'z') {
            tensorindices.push_back(*fieldname.rbegin() - 'x');
            fieldname = fieldname.substr(0, fieldname.length() - 1);
          }
          while (!fieldname.empty() && *fieldname.rbegin() == ':') {
            fieldname = fieldname.substr(0, fieldname.length() - 1);
          }
          assert(!fieldname.empty());
          reverse(tensorindices.begin(), tensorindices.end());
          const int tensorrank = tensorindices.size();
          string tensortypename;
          if (tensorrank == 0) {
            tensortypename = "Scalar3D";
          } else if (tensorrank == 1) {
            tensortypename = "Vector3D";
          } else if (tensorrank == 2) {
            tensortypename = "SymmetricTensor3D";
          } else {
            assert(0);
          }
          // Determine iteration, time level, map index, refinement level
          int iteration = 0, timelevel = 0, mapindex = 0, refinementlevel = 0,
              component = 0;
          bool is_multiblock = false, is_amr = false, is_parallel = false;
          const int width_it = 10, width_tl = 1, width_m = 3, width_rl = 2,
                    width_c = 8;
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
            else
              assert(0);
#warning "TODO: determine component, is_parallel"
            istringstream buf(val);
            assert(!buf.eof());
            buf >> *variable;
            assert(buf.eof());
          }

          assert(H5::readAttribute<int>(dataset, "group_timelevel") ==
                 timelevel);
          assert(H5::readAttribute<int>(dataset, "level") == refinementlevel);
#warning "TODO: check attribute component?"
          auto ioffsetnum = H5::readAttribute<vector<int>>(dataset, "ioffset");
          auto ioffsetdenom =
              H5::readAttribute<vector<int>>(dataset, "ioffsetdenom");
          assert(int(ioffsetnum.size()) == manifold->dimension);
          assert(int(ioffsetdenom.size()) == manifold->dimension);
          vector<double> ioffset(manifold->dimension);
          for (int d = 0; d < int(ioffset.size()); ++d)
            ioffset.at(d) =
                double(ioffsetnum.at(d)) / double(ioffsetdenom.at(d));
          if (refinementlevel == 0)
            if (!root_ioffset.count(mapindex))
              root_ioffset[mapindex] = ioffset;

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
          string value_iteration_name;
          {
            ostringstream buf;
            buf << parameter_iteration->name << "." << setfill('0')
                << setw(width_it) << iteration;
            value_iteration_name = buf.str();
          }
          string value_timelevel_name;
          {
            ostringstream buf;
            buf << parameter_timelevel->name << "." << setfill('0')
                << setw(width_tl) << timelevel;
            value_timelevel_name = buf.str();
          }
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
          if (!discretizations.count(mapindex)) {
            discretizations[mapindex];
          }
          while (int(discretizations.at(mapindex).size()) <= refinementlevel) {
            int rl = discretizations.at(mapindex).size();
            string discretizationname;
            {
              ostringstream buf;
              buf << "grid";
              if (is_multiblock)
                buf << "-map." << setfill('0') << setw(width_m) << mapindex;
              if (is_amr)
                buf << "-level." << setfill('0') << setw(width_rl) << rl;
              discretizationname = buf.str();
            }
            auto discretization = manifold->createDiscretization(
                discretizationname, configuration);
            discretizations.at(mapindex).push_back(discretization);
            if (rl > 0) {
              string subdiscretizationname;
              {
                ostringstream buf;
                buf << "level." << setfill('0') << setw(width_rl) << rl;
                subdiscretizationname = buf.str();
              }
              vector<double> offset(manifold->dimension);
              for (int d = 0; d < manifold->dimension; ++d)
                offset.at(d) = ioffset.at(d) - root_ioffset.at(mapindex).at(d);
              vector<double> factor(manifold->dimension, 2);
              auto subdiscretization = manifold->createSubDiscretization(
                  subdiscretizationname,
                  discretizations.at(mapindex).at(rl - 1),
                  discretizations.at(mapindex).at(rl), factor, offset);
              (void)subdiscretization;
            }
          }
          const auto &discretization =
              discretizations.at(mapindex).at(refinementlevel);

          // Get discretization block
          string blockname;
          {
            ostringstream buf;
            buf << configuration->name << "-c." << setfill('0') << setw(width_c)
                << component;
            blockname = buf.str();
          }
          if (!discretization->discretizationblocks.count(blockname)) {
            auto discretizationblock =
                discretization->createDiscretizationBlock(blockname);
            vector<hssize_t> offset(dim);
            H5::readAttribute(dataset, "iorigin", offset);
            discretizationblock->setOffset(offset);
          }
          auto discretizationblock =
              discretization->discretizationblocks.at(blockname);
          // Get field
          if (!project->fields.count(fieldname)) {
            auto field =
                project->createField(fieldname, global_configuration, manifold,
                                     tangentspace, tensortype);
          }
          auto field = project->fields.at(fieldname);

          // Get coordinates
          if (field->name == "GRID") {
            string coordinatesystemname;
            {
              ostringstream buf;
              buf << field->name << "-" << configuration->name;
              coordinatesystemname = buf.str();
            }
            if (!project->coordinatesystems.count(coordinatesystemname)) {
              project->createCoordinateSystem(coordinatesystemname,
                                              configuration, manifold);
            }
            auto coordinatesystem =
                project->coordinatesystems.at(coordinatesystemname);
            // TODO: Handle scalar coordinate fields
            assert(tensortype->rank == 1);
            int direction = tensorcomponent->indexvalues.at(0);
            string coordinatefieldname;
            {
              ostringstream buf;
              buf << coordinatesystem->name << "-" << direction;
              coordinatefieldname = buf.str();
            }
            if (!coordinatesystem->directions.count(direction)) {
              coordinatesystem->createCoordinateField(coordinatefieldname,
                                                      direction, field);
            }
          }

          // Get discrete field
          string discretefieldname;
          {
            ostringstream buf;
            buf << fieldname << "-" << configurationname;
            discretefieldname = buf.str();
          }
          if (!field->discretefields.count(discretefieldname)) {
            field->createDiscreteField(discretefieldname, configuration,
                                       discretization, basis);
          }
          auto discretefield = field->discretefields.at(discretefieldname);
          // Get discrete field block
          if (!discretefield->discretefieldblocks.count(
                  discretizationblock->name)) {
            discretefield->createDiscreteFieldBlock(discretizationblock->name,
                                                    discretizationblock);
          }
          auto discretefieldblock =
              discretefield->discretefieldblocks.at(discretizationblock->name);
          // Get discrete field block data
          if (!discretefieldblock->discretefieldblockcomponents.count(
                  tensorcomponent->name)) {
            discretefieldblock->createDiscreteFieldBlockComponent(
                tensorcomponent->name, tensorcomponent);
          }
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
  }

  // Write file
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file = H5::H5File(outputfilename, H5F_ACC_TRUNC,
                         H5::FileCreatPropList::DEFAULT, fapl);
  project->write(file);

  return 0;
}
