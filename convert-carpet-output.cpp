#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;
using namespace std;

const char *const dirnames[] = {"x", "y", "z"};

int main(int argc, char **argv) {

  if (argc < 3) {
    cerr << "Synposis:\n" << argv[0]
         << " <output file name> {<input file name>}+\n";
    return 1;
  }

  const string outputfilename = argv[1];
  string basename = outputfilename;
  {
    auto dotpos = basename.rfind('.');
    if (dotpos != string::npos)
      basename = basename.substr(0, dotpos);
  }
  {
    auto slashpos = basename.rfind('/');
    if (slashpos != string::npos)
      basename = basename.substr(slashpos + 1);
  }
  assert(!basename.empty());

  // Project
  const string projectname = basename;
  auto project = createProject(projectname);
  // Parameters
  auto parameter_iteration = project->createParameter("iteration");
  auto parameter_timelevel = project->createParameter("timelevel");
  // Configuration
  auto configuration = project->createConfiguration("global");
  // TensorTypes
  project->createStandardTensortypes();
  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", dim);
  auto tangentspace = project->createTangentSpace("space", dim);
  // Discretization for Manifold
  auto discretization = manifold->createDiscretization("grid");
  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian");
  for (int d = 0; d < dim; ++d) {
    basis->createBasisVector(dirnames[d], d);
  }

  for (int argi = 2; argi < argc; ++argi) {
    auto inputfilename = argv[argi];
    cout << "Reading file " << inputfilename << "...\n";

    auto file = H5::H5File(inputfilename, H5F_ACC_RDONLY);
    hsize_t idx = 0;
    H5::iterateElems(
        file, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
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
          string configurationname;
          {
            ostringstream buf;
            buf << "it." << iteration << "-tl." << timelevel;
            configurationname = buf.str();
          }
          if (!project->configurations.count(configurationname)) {
            auto configuration =
                project->createConfiguration(configurationname);
            auto value_iteration = configuration->createParameterValue(
                parameter_iteration->name, parameter_iteration);
            value_iteration->setValue(iteration);
            auto value_timelevel = configuration->createParameterValue(
                parameter_timelevel->name, parameter_timelevel);
            value_timelevel->setValue(timelevel);
          }
          auto configuration = project->configurations.at(configurationname);

          // Get tensor type
          auto tensortype = project->tensortypes.at(tensortypename);
          assert(tensortype->rank == tensorrank);
          TensorComponent *tensorcomponent = 0;
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
            auto field = project->createField(fieldname, manifold, tangentspace,
                                              tensortype);
            field->createDiscreteField(fieldname, discretization, basis);
          }
          auto field = project->fields.at(fieldname);
          auto discretefield = field->discretefields.at(fieldname);
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
          discretefieldblockdata->setExternalLink(inputfilename, name);
          // Done
          return 0;
        });
  }

  // Write file
  auto file = H5::H5File(outputfilename, H5F_ACC_TRUNC);
  project->write(file);

  return 0;
}
