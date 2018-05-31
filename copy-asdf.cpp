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

int main(int argc, char **argv) {
  cout << "sio-copy: Copy the content of a SimulationIO file\n";
  if (argc != 3) {
    cerr << "Wrong number of arguments\n"
         << "Syntax: " << argv[0] << " <input file> <output file>\n";
    exit(1);
  }
  string inputfilename = argv[1];
  string outputfilename = argv[2];
  assert(!inputfilename.empty());
  assert(!outputfilename.empty());

  // Read project
  ifstream is(inputfilename, ios::binary | ios::in);
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
  const auto &project = projects.begin()->second;

  // Copy project
  auto project2 = project;

  // Write project
  map<string, string> tags{
      {"sio", "tag:github.com/eschnett/SimulationIO/asdf-cxx/"}};
  map<string, function<void(ASDF::writer & w)>> funs{
      {"SimulationIO", [&](ASDF::writer &w) { w << *project; }}};
  const auto &doc2 = ASDF::asdf(move(tags), move(funs));
  ofstream os(outputfilename, ios::binary | ios::trunc | ios::out);
  doc2.write(os);
  os.close();

  cout << "Done.\n";
  return 0;
}
