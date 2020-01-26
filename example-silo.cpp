#include <silo.h>

#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char **argv) {
  cout << "example-silo: Create a Silo file\n";

  // DB_NOCLOBBER does not work
  DBfile *const file = DBCreate("example.silo", DB_CLOBBER, DB_LOCAL,
                                "Example Silo file for SimulationIO", DB_HDF5);

  int ierr = DBMkDir(file, "directory");
  assert(!ierr);

  const vector<double> coordx{0, 1}, coordy{2, 3, 4}, coordz{5, 6, 7, 8};
  const vector<int> dims{2, 3, 4};
  const vector<const double *> coords{coordx.data(), coordy.data(),
                                      coordz.data()};
  ierr = DBPutQuadmesh(file, "directory/mesh", nullptr, coords.data(),
                       dims.data(), 3, DB_DOUBLE, DB_COLLINEAR, nullptr);
  assert(!ierr);

  const vector<double> values(2 * 3 * 4, 0);
  ierr = DBPutQuadvar1(file, "directory/scalar", "directory/mesh",
                       values.data(), dims.data(), 3, nullptr, 0, DB_DOUBLE,
                       DB_NODECENT, nullptr);
  assert(!ierr);

  // write options

  // Can also use DBPutQuadvar for edge- or face-centred data
  const vector<double> valuesxx(2 * 3 * 4, 1);
  const vector<double> valuesxy(2 * 3 * 4, 2);
  const vector<double> valuesxz(2 * 3 * 4, 3);
  const vector<double> valuesyy(2 * 3 * 4, 4);
  const vector<double> valuesyz(2 * 3 * 4, 5);
  const vector<double> valueszz(2 * 3 * 4, 6);
  const vector<const char *> varnames{"varxx", "varxy", "varxz",
                                      "varyy", "varyz", "varzz"};
  const vector<const double *> vars{valuesxx.data(), valuesxy.data(),
                                    valuesxz.data(), valuesyy.data(),
                                    valuesyz.data(), valueszz.data()};
  ierr = DBPutQuadvar(file, "directory/symmtensor", "directory/mesh", 6,
                      varnames.data(), vars.data(), dims.data(), 3, nullptr, 0,
                      DB_DOUBLE, DB_NODECENT, nullptr);
  assert(!ierr);

  // DBPutZonelist2
  // DBPutFacelist (?)

  // DBPutMultimesh
  // DBMakeMrgtree
  // DBPutMultivar

  DBobject *const obj = DBMakeObject("directory/object", DB_USERDEF, 0);
  assert(obj);
  ierr = DBAddStrComponent(obj, "name", "epsdiss");
  assert(!ierr);
  ierr = DBAddDblComponent(obj, "value", 0.32);
  assert(!ierr);
  ierr = DBWriteObject(file, obj, 1);
  assert(!ierr);

  ierr = DBClose(file);
  assert(!ierr);

  cout << "Done.\n";
  return 0;
}
