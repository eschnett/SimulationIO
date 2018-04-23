#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>

using namespace SimulationIO;

using namespace std;

const string projectname = "qc0-mclachlan_ih_erik";

// Indentation level
int level = 0;
struct increase_indentation {
  int old_level;
  increase_indentation() : old_level(level) { ++level; }
  ~increase_indentation() {
    --level;
    assert(level == old_level);
  }
};

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

  // Default: copy all fields
  string fieldnameregex = ".*";
  string outputfilename;
  vector<string> inputfilenames;
  bool have_error = false;
  for (int argi = 1; argi < argc; ++argi) {
    string arg = argv[argi];
    if (arg.find("--regex=") == 0) {
      fieldnameregex = arg.substr(string("--regex=").length());
    } else if (arg.find("-") == 0) {
      have_error = true;
    } else if (outputfilename.length() == 0) {
      outputfilename = arg;
    } else {
      inputfilenames.push_back(arg);
    }
  }
  if (outputfilename.length() == 0)
    have_error = true;
  if (inputfilenames.size() == 0)
    have_error = true;

  if (have_error) {
    cerr << "Synopsis:\n"
         << argv[0]
         << " [--regex=<extended regex>] <output file name> "
            "{<input file name>}+\n";
    exit(1);
  }

  // Create project
  string basename = get_basename(outputfilename);
  assert(!basename.empty());
  cout << indent(level) << "Creating project " << quote(projectname) << "\n";
  auto project2 = createProject(projectname);

  const int nfiles = inputfilenames.size();
  cout << indent(level) << "Reading " << nfiles << " files\n";
  int ifile = 0;
  for (const auto &inputfilename : inputfilenames) {
    ++ifile;
    cout << indent(level) << "Reading file " << quote(inputfilename) << " ("
         << ifile << "/" << nfiles << ")\n";
    increase_indentation l;
    auto file = [&]() {
      try {
        return H5::H5File(inputfilename, H5F_ACC_RDONLY);
      } catch (const H5::FileIException &error) {
        cerr << "Could not open file " << quote(inputfilename)
             << " for reading.\n";
        exit(2);
      }
    }();
    auto project = readProject(file);

    // Copy requested fields
    auto r = regex(fieldnameregex, regex_constants::nosubs |
                                       regex_constants::optimize |
                                       regex_constants::extended);
    for (const auto &field_kv : project->fields()) {
      const auto &field = field_kv.second;
      if (regex_match(field->name(), r)) {
        cout << indent(level) << "Field " << quote(field->name()) << "\n";
        increase_indentation l;
        auto field2 = project2->copyField(field, true);
      }
    }

    // Copy coordinate systems
    for (const auto &coordinatesystem_kv : project->coordinatesystems()) {
      const auto &coordinatesystem = coordinatesystem_kv.second;
      cout << indent(level) << "CoordinateSystem "
           << quote(coordinatesystem->name()) << "\n";
      increase_indentation l;
      auto coordinatesystem2 =
          project2->copyCoordinateSystem(coordinatesystem, true);
    }
  }

  // Write file
  cout << "Writing file " << quote(outputfilename) << "\n";
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file = H5::H5File(outputfilename, H5F_ACC_TRUNC,
                         H5::FileCreatPropList::DEFAULT, fapl);
  project2->write(file);

  return 0;
}
