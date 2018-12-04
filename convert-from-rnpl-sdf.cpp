#include "SimulationIO.hpp"

#include "RegionCalculus.hpp"

#include "H5Helpers.hpp"

#include <librnpl.h>
#include <sdf_priv.h>

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
using namespace Output;
using namespace std;

const char *const dirnames[] = {"x", "y", "z"};

// Output field widths for various quantities
const int width_it = 10;

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
  string outputfilename;
  vector<string> inputfilenames;

  for (int argi = 1; argi < argc; ++argi) {
    auto argvi = string(argv[argi]);
    if (argvi.empty()) {
      have_error = true;
      break;
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
         << argv[0] << " <output file name> {<input file name>}*\n";
    return 1;
  }

  const string basename = get_basename(outputfilename);
  assert(!basename.empty());

  // Project
  const string projectname = basename;
  auto project = createProject(projectname);
  // Parameters
  auto parameter_iteration = project->createParameter("iteration");
  // Configuration
  auto global_configuration = project->createConfiguration("global");
  // TensorTypes
  project->createStandardTensorTypes();
  // Manifold and TangentSpace, for arbitrary dimension
  shared_ptr<Manifold> manifold;
  shared_ptr<TangentSpace> tangentspace;
  // Discretization for Manifold
  map<string, shared_ptr<Discretization>> discretizations; // [configuration]
  // Basis for TangentSpace
  shared_ptr<Basis> basis;

  gft_set_multi();

  for (const auto &inputfilename : inputfilenames) {
    cout << "Reading file " << inputfilename << "...\n";

    gft_sdf_file_data *inputfile = gft_open_sdf_stream(inputfilename.c_str());
    assert(inputfile);

    int iteration = 0;
    double time;
    int version, dimension, data_size, coord_size;
    char *dataset_name, *coord_names, *tag;
    int *sdf_shape;
    double *sdf_bbox, *coord_pointers, *data_pointer;
    while (low_read_sdf_stream(1, inputfile->fp, &time, &version, &dimension,
                               &data_size, &coord_size, &dataset_name,
                               &coord_names, &tag, &sdf_shape, &sdf_bbox,
                               &coord_pointers, &data_pointer)) {

      const string fieldname(dataset_name);
      cout << "  opening dataset #" << iteration << " \"" << fieldname
           << "\"...\n";

      // The tensor type is not stored, so we assume a scalar
      const int tensorrank = 0;
      const vector<int> tensorindices{};
      const string tensortypename =
          vector<string>{"Scalar0D", "Scalar1D", "Scalar2D", "Scalar3D"}.at(
              dimension);

      // Output information
      cout << "    field name: " << fieldname << "\n"
           << "    dimension:  " << dimension << "\n"
           << "    iteration:  " << iteration << "\n"
           << "    time:       " << time << "\n"
           << "\n";

      // Get configuration
      string value_iteration_name = [&] {
        ostringstream buf;
        buf << parameter_iteration->name() << "." << setfill('0')
            << setw(width_it) << iteration;
        return buf.str();
      }();
      auto configurationname = value_iteration_name;
      if (!project->configurations().count(configurationname)) {
        auto configuration = project->createConfiguration(configurationname);
        if (!parameter_iteration->parametervalues().count(
                value_iteration_name)) {
          auto value_iteration =
              parameter_iteration->createParameterValue(value_iteration_name);
          value_iteration->setValue(iteration);
        }
        auto value_iteration =
            parameter_iteration->parametervalues().at(value_iteration_name);
        configuration->insertParameterValue(value_iteration);
      }
      auto configuration = project->configurations().at(configurationname);

      // Get tensor type
      auto tensortype = project->tensortypes().at(tensortypename);
      assert(tensortype->rank() == tensorrank);
      shared_ptr<TensorComponent> tensorcomponent;
      for (const auto &tc : tensortype->tensorcomponents()) {
        if (tc.second->indexvalues() == tensorindices) {
          tensorcomponent = tc.second;
          break;
        }
      }
      assert(tensorcomponent);

      // Get manifold
      if (!manifold)
        manifold =
            project->createManifold("domain", global_configuration, dimension);
      assert(manifold->dimension() == dimension);

      // Get tangentspace
      if (!tangentspace) {
        tangentspace = project->createTangentSpace(
            "tangentspace", global_configuration, dimension);
        basis = tangentspace->createBasis("Cartesian", global_configuration);
        for (int d = 0; d < dimension; ++d)
          basis->createBasisVector(dirnames[d], d);
      }
      assert(tangentspace->dimension() == dimension);

      // Get discretization
      auto discretizationname = configuration->name();
      if (!discretizations.count(configuration->name()))
        discretizations[configuration->name()] =
            manifold->createDiscretization(discretizationname, configuration);
      auto discretization = discretizations.at(configuration->name());

      // Get discretization block
      string blockname = "grid";
      if (!discretization->discretizationblocks().count(blockname)) {
        auto discretizationblock =
            discretization->createDiscretizationBlock(blockname);
        vector<int> offset(dimension), shape(dimension);
        for (int d = 0; d < dimension; ++d) {
          offset[d] = 0;
          shape[d] = sdf_shape[d];
        }
        discretizationblock->setBox(
            box_t(point_t(offset), point_t(offset) + point_t(shape)));
      }
      auto discretizationblock =
          discretization->discretizationblocks().at(blockname);

      // Get field
      if (!project->fields().count(fieldname))
        project->createField(fieldname, global_configuration, manifold,
                             tangentspace, tensortype);
      auto field = project->fields().at(fieldname);

      // Get discrete field
      string discretefieldname = [&] {
        ostringstream buf;
        buf << fieldname << "-" << discretization->name();
        return buf.str();
      }();

      auto discretefield = field->createDiscreteField(
          discretefieldname, configuration, discretization, basis);
      auto discretefieldblock = discretefield->createDiscreteFieldBlock(
          discretizationblock->name(), discretizationblock);
      auto discretefieldblockcomponent =
          discretefieldblock->createDiscreteFieldBlockComponent(
              tensorcomponent->name(), tensorcomponent);

      auto dataset = discretefieldblockcomponent->createDataSet<double>();
      dataset->attachData(data_pointer, discretizationblock->getBox());

      ++iteration;
    } // while low_read_sdf_stream
  }   // for inputfilename

  // Write file
  auto file =
      H5::H5File(outputfilename, H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT,
                 H5::FileAccPropList::DEFAULT);
  project->write(file);

  return 0;
}
