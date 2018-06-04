#include "SimulationIO.hpp"

#include "asdf.hpp"

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using namespace SimulationIO;
using namespace std;

shared_ptr<Project> read(const string &filename) {
  ifstream is(filename, ios::binary | ios::in);
  return readProjectASDF(is);
}

int main(int argc, char **argv) {
  cout << "sio-list-asdf: List content of ASDF files\n";

  if (argc < 1) {
    cerr << "Synopsis:\n" << argv[0] << " <file name>*\n";
    return 1;
  }

  for (int argi = 1; argi < argc; ++argi) {
    auto project = read(argv[argi]);
    cout << *project;
  }

  return 0;
}
