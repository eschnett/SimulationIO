#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using namespace SimulationIO;
using namespace std;

shared_ptr<Project> read(const string &filename) {
  try {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    return readProject(file);
  } catch (const H5::FileIException &error) {
    cerr << "Could not read file " << quote(filename) << "\n";
    exit(2);
  }
}

void write(const shared_ptr<Project> &project, const string &filename) {
  try {
    auto fapl = H5::FileAccPropList();
    fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    auto file = H5::H5File(filename, H5F_ACC_TRUNC,
                           H5::FileCreatPropList::DEFAULT, fapl);
    project->write(file);
  } catch (const H5::FileIException &error) {
    cerr << "Could not write file " << quote(filename) << "\n";
    exit(2);
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "Synopsis:\n" << argv[0] << " {<src-filename>} {<dst-filename>}\n";
    exit(1);
  }

  auto project = read(argv[1]);
  write(project, argv[2]);

  return 0;
}
