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
  map<string, shared_ptr<Project>> projects;
  function<void(const ASDF::reader_state &rs, const string &name,
                const YAML::Node &node)>
      read_project{[&](const ASDF::reader_state &rs, const string &name,
                       const YAML::Node &node) {
        projects[name] = readProject(rs, node);
      }};
  map<string, function<void(const ASDF::reader_state &rs, const string &name,
                            const YAML::Node &node)>>
      readers{{"tag:github.com/eschnett/SimulationIO/asdf-cxx/Project-1.0.0",
               read_project}};
  auto doc = ASDF::asdf(is, readers);
  is.close();
  assert(projects.size() > 0);
  return projects.begin()->second;
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
