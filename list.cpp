#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;

using std::cerr;
using std::cout;
using std::ostringstream;
using std::string;
using std::vector;

const char *const dirnames[] = {"x", "y", "z"};

int main(int argc, char **argv) {

  if (argc != 2) {
    cerr << "Synopsis:\n" << argv[0] << " <filename>\n";
    return 1;
  }

  auto filename = argv[1];
  try {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    map<string, Project *> projects;
    H5::readGroup(file, ".", [&](const string &name, const H5::Group &group) {
      auto project = createProject(group, name);
      cout << *project;
    }, projects);
  } catch (H5::FileIException error) {
    cerr << "Could not open file \"" << filename << "\" for reading.\n";
    return 2;
  }

  return 0;
}
