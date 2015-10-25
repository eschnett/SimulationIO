#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <cassert>
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
  auto discretization =
      manifold->createDiscretization("grid", global_configuration);
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
          const string varname = tokens.at(0);
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
          int iteration = 0, timelevel = 0, mapindex = 0, refinementlevel = 0;
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
              variable = &mapindex;
            else if (id == "rl")
              variable = &refinementlevel;
            else
              assert(0);
            istringstream buf(val);
            assert(!buf.eof());
            buf >> *variable;
            assert(buf.eof());
          }
          // Output information
          cout << "    field name: " << fieldname << "\n";
          cout << "    tensor rank: " << tensorrank << "\n";
          cout << "    tensor type: " << tensortypename << "\n";
          cout << "    tensor indices: [";
          for (int d = 0; d < int(tensorindices.size()); ++d) {
            if (d > 0)
              cout << ",";
            cout << tensorindices[d];
          }
          cout << "]\n";
          cout << "    iteration: " << iteration << "\n";
          cout << "    time level: " << timelevel << "\n";
          cout << "    map: " << mapindex << "\n";
          cout << "    refinement level: " << refinementlevel << "\n";

          // Get configuration
          string value_iteration_name;
          {
            ostringstream buf;
            buf << parameter_iteration->name << "." << iteration;
            value_iteration_name = buf.str();
          }
          string value_timelevel_name;
          {
            ostringstream buf;
            buf << parameter_timelevel->name << "." << timelevel;
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

          // Get discretization block
          string blockname;
          {
            ostringstream buf;
            buf << configuration->name << "-m." << mapindex << "-rl."
                << refinementlevel;
            blockname = buf.str();
          }
          if (!discretization->discretizationblocks.count(blockname)) {
            discretization->createDiscretizationBlock(blockname);
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
          if (!discretefieldblock->discretefieldblockdata.count(
                  tensorcomponent->name)) {
            discretefieldblock->createDiscreteFieldBlockData(
                tensorcomponent->name, tensorcomponent);
          }
          auto discretefieldblockdata =
              discretefieldblock->discretefieldblockdata.at(
                  tensorcomponent->name);
          switch (action) {
          case action_copy:
            discretefieldblockdata->setData(inputfile, name);
            break;
          case action_extlink:
            discretefieldblockdata->setData(inputfilename, name);
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
