#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <iostream>
#include <string>

using namespace SimulationIO;

using std::cerr;
using std::cout;
using std::string;

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

  if (argc < 3) {
    cerr << "Synopsis:\n"
         << argv[0] << " <output file name> {<input file name>}+\n";
    return 1;
  }

  string outputfilename = argv[1];
  string basename = get_basename(outputfilename);
  assert(!basename.empty());

  // Create project
  shared_ptr<Project> outputproject;

  // Read projects
  for (int argi = 2; argi < argc; ++argi) {
    auto filename = argv[argi];
    try {
      auto file = H5::H5File(filename, H5F_ACC_RDONLY);
      auto project = readProject(file);
      if (!outputproject)
        outputproject = project;
      else
        outputproject->merge(project);
    } catch (const H5::FileIException &error) {
      cerr << "Could not open file " << quote(filename) << " for reading.\n";
      return 2;
    }
  }

  // Write file
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file = H5::H5File(outputfilename, H5F_ACC_TRUNC,
                         H5::FileCreatPropList::DEFAULT, fapl);
  outputproject->write(file);

  return 0;
}
