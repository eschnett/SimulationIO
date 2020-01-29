#include "SimulationIO.hpp"

#include <iostream>

using namespace SimulationIO;
using namespace std;

int main(int argc, char **argv) {
  cout << "sio-list-silo: List content of Silo files\n";

  if (argc <= 1) {
    cerr << "Synopsis:\n" << argv[0] << " <file name>*\n";
    return 1;
  }

  for (int argi = 1; argi < argc; ++argi) {
    auto project = readProjectSilo(argv[argi]);
    cout << *project;
  }

  return 0;
}
