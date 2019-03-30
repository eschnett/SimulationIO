#include "SimulationIO.hpp"

#include <iostream>
#include <sstream>

using namespace SimulationIO;
using namespace std;

// Global configuration options:

// Whether to copy or create external links
const bool create_extlink = false;

bool endswith(const string &str, const string &suffix) {
  return str.length() >= suffix.length() and
         str.substr(str.length() - suffix.length()) == suffix;
}

enum format_t { format_asdf, format_hdf5 };

format_t classify_filename(const string &filename) {
  if (endswith(filename, "5"))
    return format_hdf5;
  if (endswith(filename, ".asdf"))
    return format_asdf;
  cerr << "Cannot determine file format from file name \"" << filename
       << "\"\n";
  exit(1);
}

shared_ptr<Project> read(const string &filename) {
  switch (classify_filename(filename)) {
  case format_asdf: {
    return readProjectASDF(filename);
    break;
  }
  case format_hdf5: {
    // return readProjectHDF5(filename);
    try {
      auto file = H5::H5File(filename, H5F_ACC_RDONLY);
      return readProject(file);
    } catch (const H5::FileIException &error) {
      cerr << "Could not read file " << quote(filename) << "\n";
      exit(1);
    }
    break;
  }
  }
  assert(0);
}

shared_ptr<ParameterValue>
getParameterValue(const shared_ptr<Configuration> &configuration,
                  const string &name) {
  for (const auto &name_parametervalue : configuration->parametervalues()) {
    const auto &parametervalue = name_parametervalue.second;
    auto parameter = parametervalue->parameter();
    if (parameter->name() == name)
      return parametervalue;
  }
  return nullptr;
}

int main(int argc, char **argv) {
  cout << "sio-convert-to-carpet: Convert to Carpet format\n";

  if (argc != 3) {
    cerr << "Synopsis:\n"
         << argv[0] << " <input file name> <output file name>\n";
    return 1;
  }

  // Read SimulationIO file
  auto filename = argv[1];
  auto project = read(filename);

  // Create Carpet output file
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file =
      H5::H5File(argv[2], H5F_ACC_EXCL, H5::FileCreatPropList::DEFAULT, fapl);

  for (const auto &name_field : project->fields()) {
    const auto &field = name_field.second;
    cout << "Reading field \"" << field->name() << "\"...\n";
    for (const auto &name_discretefield : field->discretefields()) {
      const auto &discretefield = name_discretefield.second;
      cout << "  Reading discrete field \"" << discretefield->name()
           << "\"...\n";
      // TODO: Instead of discrete field blocks, map discretization
      // blocks to components
      int component = 0;
      for (const auto &name_discretefieldblock :
           discretefield->discretefieldblocks()) {
        const auto &discretefieldblock = name_discretefieldblock.second;
        cout << "    Reading discrete field block \""
             << discretefieldblock->name() << "\"...\n";
        for (const auto &name_discretefieldblockcomponent :
             discretefieldblock->discretefieldblockcomponents()) {
          const auto &discretefieldblockcomponent =
              name_discretefieldblockcomponent.second;
          cout << "      Reading discrete field block component \""
               << discretefieldblockcomponent->name() << "\"...\n";
          // Analyse dataset
          // Determine Carpet dataset name
          ostringstream buf;
          buf << field->name();
          auto configuration = discretefield->configuration();
          // Determine tensor type
          auto tensortype = field->tensortype();
          auto tensorcomponent = discretefieldblockcomponent->tensorcomponent();
          if (tensortype->rank() > 0) {
            buf << ".";
            for (int r = 0; r < tensortype->rank(); ++r) {
              int index = tensorcomponent->indexvalues().at(r);
              switch (index) {
              case 0:
                buf << "x";
                break;
              case 1:
                buf << "y";
                break;
              case 2:
                buf << "z";
                break;
              default:
                buf << "[" << index << "]";
                break;
              }
            }
          }
          // Look up iteration number, if any
          auto iteration = getParameterValue(configuration, "iteration");
          if (iteration) {
            assert(iteration->value_type == ParameterValue::type_int);
            buf << " it=" << iteration->value_int;
          }
          // Look up time level, if any
          auto timelevel = getParameterValue(configuration, "timelevel");
          if (timelevel) {
            assert(timelevel->value_type == ParameterValue::type_int);
            buf << " tl=" << timelevel->value_int;
          }
          // TODO: Determine map index from discretization
          // Calculate refinement level
          auto discretization = discretefield->discretization();
          int reflevel = 0;
          while (true) {
            auto subdiscretization_iter =
                discretization->parent_discretizations().begin();
            if (subdiscretization_iter ==
                discretization->parent_discretizations().end())
              break;
            auto subdiscretization = subdiscretization_iter->second.lock();
            discretization = subdiscretization->parent_discretization();
            ++reflevel;
          }
          buf << " rl=" << reflevel;
          // Handle component
          buf << " c=" << component;
          // Finalize Carpet dataset name
          string name = buf.str();
          // Write dataset
          cout << "        Writing dataset \"" << name << "...\n";
          bool did_process = false;
          const auto datarange = discretefieldblockcomponent->datarange();
          if (datarange) {
            cout << "           (ignoring data range)\n";
            did_process = true;
          }
#ifdef SIMULATIONIO_HAVE_HDF5
          const auto copyobj = discretefieldblockcomponent->copyobj();
          if (copyobj) {
            // Input is HDF5
            if (!create_extlink) {
              // Copy dataset
              auto ocpypl = H5::take_hid(H5Pcreate(H5P_OBJECT_COPY));
              assert(ocpypl.valid());
              herr_t herr =
                  H5Pset_copy_object(ocpypl, H5O_COPY_WITHOUT_ATTR_FLAG);
              assert(!herr);
              auto lcpl = H5::take_hid(H5Pcreate(H5P_LINK_CREATE));
              assert(lcpl.valid());
              herr = H5Ocopy(copyobj->group().getId(), copyobj->name().c_str(),
                             file.getId(), name.c_str(), ocpypl, lcpl);
              assert(!herr);
            } else {
              // Create external link to dataset
              herr_t herr = createExternalLink(
                  file, name, discretefieldblockcomponent->getPath(),
                  discretefieldblockcomponent->getName());
              assert(!herr);
            }
            // TODO: Add attributes
            did_process = true;
          }
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
          const auto asdfref = discretefieldblockcomponent->asdfref();
          if (asdfref) {
            // Input is ASDF
            // TODO: Fill this in later
            assert(0);
            did_process = true;
          }
#endif
          if (!did_process) {
            cerr << "Cannot read discrete field block component \""
                 << discretefieldblockcomponent->name() << "\"\n";
            exit(1);
          }
        }
        ++component;
      }
    }
  }

  file.close();

  cout << "Done.\n";
  return 0;
}
