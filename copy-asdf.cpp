#include "SimulationIO.hpp"

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
  auto pis = make_shared<ifstream>(filename, ios::binary | ios::in);
  return readProjectASDF(pis);
}

void write(const shared_ptr<Project> &project, const string &filename) {
  ofstream os(filename, ios::binary | ios::trunc | ios::out);
  project->writeASDF(os);
}

int main(int argc, char **argv) {
  cout << "sio-copy: Copy the content of a SimulationIO file\n";
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
