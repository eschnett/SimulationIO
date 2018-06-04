#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include "asdf.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
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
  ofstream os(filename, ios::binary | ios::trunc | ios::out);
  project->writeASDF(os);
}

int main(int argc, char **argv) {
  cout << "sio-convert-hdf5-to-asdf: Convert an SimulationIO file from HDF5 to "
          "ASDF\n";
  if (argc != 3) {
    cerr << "Wrong number of arguments\n"
         << "Syntax: " << argv[0] << " <input file> <output file>\n";
    exit(1);
  }

  auto project = read(argv[1]);
  write(project, argv[2]);

  cout << "Done.\n";
  return 0;
}
