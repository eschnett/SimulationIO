#include <silo.h>

#include <cassert>
#include <iostream>
using namespace std;

int main(int argc, char **argv) {
  cout << "example-silo: Create a Silo file\n";
  DBfile *file = DBCreate("example.silo", DB_NOCLOBBER, DB_LOCAL,
                          "Example Silo file for SimulationIO", DB_HDF5);
  const int ierr = DBClose(file);
  assert(!ierr);
  file = nullptr;
  cout << "Done.\n";
  return 0;
}
