#include "SimulationIO.hpp"

#include "H5Helpers.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using namespace SimulationIO;
using namespace std;

bool endswith(const string &str, const string &suffix) {
  return str.length() >= suffix.length() and
         str.substr(str.length() - suffix.length()) == suffix;
}

enum format_t {
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  format_asdf,
#endif
#ifdef SIMULATIONIO_HAVE_HDF5
  format_hdf5,
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  format_tiledb,
#endif
};

format_t classify_filename(const string &filename) {
  if (endswith(filename, "5"))
    return format_hdf5;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  if (endswith(filename, ".asdf"))
    return format_asdf;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  if (endswith(filename, ".tdb"))
    return format_tiledb;
#endif
  cerr << "Cannot determine file format from file name \"" << filename
       << "\"\n";
  exit(1);
}

shared_ptr<Project> read(const string &filename) {
  switch (classify_filename(filename)) {
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  case format_asdf:
    return readProjectASDF(filename);
    break;
#endif
#ifdef SIMULATIONIO_HAVE_HDF5
  case format_hdf5:
    // return readProjectHDF5(filename);
    try {
      auto file = H5::H5File(filename, H5F_ACC_RDONLY);
      return readProject(file);
    } catch (const H5::FileIException &error) {
      cerr << "Could not read file " << quote(filename) << "\n";
      exit(1);
    }
    break;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  case format_tiledb:
    cerr << "Reading TileDB files is not yet implemented\n";
    exit(1);
    break;
#endif
  }
  assert(0);
}

void write(const shared_ptr<Project> &project, const string &filename) {
  switch (classify_filename(filename)) {
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  case format_asdf: {
    project->writeASDF(filename);
    break;
  }
#endif
#ifdef SIMULATIONIO_HAVE_HDF5
  case format_hdf5: {
    project->writeHDF5(filename);
    break;
  }
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  case format_tiledb: {
    project->writeTileDB(filename);
    break;
  }
#endif
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
