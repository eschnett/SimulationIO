#include <silo.h>

#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  cout << "example-silo: Create a Silo file\n";
  DBFile *file = DBCreate("example.silo", DB_NOCLOBBER, DB_LOCAL,
                          "Example Silo file for SimulationIO", DB_HDF5);
  const int ierr = DBClose(file);
  assert(!ierr);
  file = nullptr;
  cout << "Done.\n";
  return 0;
}
