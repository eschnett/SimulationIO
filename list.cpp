#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace SimulationIO;

using std::cerr;
using std::cout;
using std::string;

const int efc_size = 10; // external link file cache size

int main(int argc, char **argv) {

  if (argc < 2) {
    cerr << "Synopsis:\n" << argv[0] << " {<file name>}+\n";
    return 1;
  }

  for (int argi = 1; argi < argc; ++argi) {
    auto filename = argv[argi];
    try {
      auto fapl = H5::FileAccPropList();
      herr_t herr = H5Pset_elink_file_cache_size(fapl.getId(), efc_size);
      assert(!herr);
      // fapl: setSieveBufSize
      // fapl: setMetaBlockSize
      // fapl: setAlignment
      // fapl: setCache
      // fcpl: setSymk
      auto file = H5::H5File(filename, H5F_ACC_RDONLY,
                             H5::FileCreatPropList::DEFAULT, fapl);
      auto project = readProject(file);
      cout << *project;
    } catch (const H5::FileIException &error) {
      cerr << "Could not open file " << quote(filename) << " for reading.\n";
      return 2;
    }
  }

  return 0;
}
