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

  // Project
  auto project = createProject("simulation");

  // TensorTypes
  project->createStandardTensortypes();

  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", dim);
  auto tangentspace = project->createTangentSpace("space", dim);

  // Discretization for Manifold
  auto discretization = manifold->createDiscretization("uniform");

  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian");
  for (int d = 0; d < dim; ++d) {
    basis->createBasisVector(dirnames[d], d);
  }

  for (int argi = 1; argi < argc; ++argi) {
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
          assert(tokens.size() == 4);
          const string varname = tokens.at(0);
          const string itstring = tokens.at(1);
          const string tlstring = tokens.at(2);
          const string rlstring = tokens.at(3);
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
          // Determine iteration, time level, refinement level
          assert(itstring.substr(0, 3) == "it=");
          istringstream itstream(itstring.substr(4));
          int iteration;
          itstream >> iteration;
          assert(tlstring.substr(0, 3) == "tl=");
          istringstream tlstream(tlstring.substr(4));
          int timelevel;
          itstream >> timelevel;
          assert(rlstring.substr(0, 3) == "rl=");
          istringstream rltstream(rlstring.substr(4));
          int refinementlevel;
          itstream >> refinementlevel;
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
          cout << "    refinement level: " << refinementlevel << "\n";
          // Get tensor type
          auto tensortype = project->tensortypes.at(tensortypename);
          assert(tensortype->rank == tensorrank);
          TensorComponent *tensorcomponent = 0;
          for (map<string, TensorComponent *>::const_iterator
                   i = tensortype->tensorcomponents.begin(),
                   e = tensortype->tensorcomponents.end();
               i != e; ++i) {
            const auto &tc = i->second;
            if (tc->indexvalues == tensorindices) {
              tensorcomponent = tc;
              break;
            }
          }
          assert(tensorcomponent);
          // Get discretization block
          if (!discretization->discretizationblocks.count("uniform")) {
            discretization->createDiscretizationBlock("uniform");
          }
          auto discretizationblock =
              discretization->discretizationblocks.at("uniform");
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

  // output
  cout << *project;

  // Write file
  auto filename = "simulation.h5";
  auto file = H5::H5File(filename, H5F_ACC_TRUNC);
  project->write(file);

  return 0;
}
