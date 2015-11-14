#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <iostream>
#include <string>

using namespace SimulationIO;

using std::cerr;
using std::cout;
using std::string;

int main(int argc, char **argv) {

  if (argc < 2) {
    cerr << "Synopsis:\n" << argv[0] << " {<filename>}+\n";
    return 1;
  }

  for (int argi = 1; argi < argc; ++argi) {
    auto filename = argv[argi];
    try {
      auto file = H5::H5File(filename, H5F_ACC_RDONLY);
      auto project = readProject(file);
      cout << *project;
    } catch (H5::FileIException error) {
      cerr << "Could not open file " << quote(filename) << " for reading.\n";
      return 2;
    }
  }

  return 0;
}
