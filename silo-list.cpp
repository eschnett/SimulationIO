#include "SiloHelpers.hpp"

#include <silo.h>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

string bool_string(const bool b) { return b ? "true" : "false"; }

string quote_string(const string &str) {
  ostringstream buf;
  buf << '"';
  for (const unsigned char ch : str) {
    if (!isprint(ch)) {
      buf << '\\';
      switch (ch) {
      case '\a':
        buf << 'a';
        break;
      case '\b':
        buf << 'b';
        break;
      case '\e':
        buf << 'e';
        break;
      case '\f':
        buf << 'f';
        break;
      case '\n':
        buf << 'n';
        break;
      case '\r':
        buf << 'r';
        break;
      case '\t':
        buf << 't';
        break;
      case '\v':
        buf << 'v';
        break;
      default:
        buf << 'x' << hex << setw(2) << setfill('0') << int(ch);
        break;
      }
    } else {
      if (ch == '\\' || ch == '"')
        buf << '\\';
      buf << ch;
    }
  }
  buf << '"';
  return buf.str();
}

////////////////////////////////////////////////////////////////////////////////

string centering_string(const int centering) {
  switch (centering) {
  case DB_NODECENT:
    return "node";
  case DB_ZONECENT:
    return "zone";
  case DB_FACECENT:
    return "face";
  case DB_EDGECENT:
    return "edge";
  default:
    assert(0);
  }
}

string coordsys_string(const int coordsys) {
  switch (coordsys) {
  case DB_CARTESIAN:
    return "cartesian";
  case DB_CYLINDRICAL:
    return "cylindrical";
  case DB_SPHERICAL:
    return "spherical";
  case DB_NUMERICAL:
    return "numerical";
  case DB_OTHER:
    return "other";
  default:
    assert(0);
  }
}

string coordtype_string(const int coordtype) {
  switch (coordtype) {
  case DB_COLLINEAR:
    return "collinear";
  case DB_NONCOLLINEAR:
    return "non-collinear";
  default:
    assert(0);
  }
}

// string major_order_string(const int major_order) {
//   switch (major_order) {
//   case 0:
//     return "column-major (Fortran)";
//   case 1:
//     return "row-major (C)";
//   default:
//     assert(0);
//   }
// }

string tensorrank_string(const int tensortype) {
  switch (tensortype) {
  case DB_VARTYPE_SCALAR:
    return "scalar";
  case DB_VARTYPE_VECTOR:
    return "vector";
  case DB_VARTYPE_TENSOR:
    return "tensor";
  case DB_VARTYPE_SYMTENSOR:
    return "symtensor";
  default:
    assert(0);
  }
}

int vartype(const char *) { return DB_CHAR; }
int vartype(const short *) { return DB_SHORT; }
int vartype(const int *) { return DB_INT; }
int vartype(const long *) { return DB_LONG; }
int vartype(const long long *) { return DB_LONG_LONG; }
int vartype(const float *) { return DB_FLOAT; }
int vartype(const double *) { return DB_DOUBLE; }

string vartype_string(const int type) {
  switch (type) {
  case DB_CHAR:
    return "char";
  case DB_SHORT:
    return "short";
  case DB_INT:
    return "int";
  case DB_LONG:
    return "long";
  case DB_LONG_LONG:
    return "long long";
  case DB_FLOAT:
    return "float";
  case DB_DOUBLE:
    return "double";
  default:
    return "<unknown variable type>";
  }
}

////////////////////////////////////////////////////////////////////////////////

string read_char_var(const Silo<DBfile> &file, const string &varname) {
  int ierr;
  const int type = DBGetVarType(file.get(), varname.c_str());
  assert(type == DB_CHAR);
  const int length = DBGetVarLength(file.get(), varname.c_str());
  assert(length >= 0);
  string data(length, '\0');
  ierr =
      DBReadVar(file.get(), varname.c_str(), const_cast<char *>(data.data()));
  assert(!ierr);
  // assert(*data.rbegin() == '\0');
  // data.pop_back();
  return data;
}

template <typename T>
vector<T> read_var(const Silo<DBfile> &file, const string &varname) {
  int ierr;
  const int type = DBGetVarType(file.get(), varname.c_str());
  assert(type == vartype(static_cast<const T *>(nullptr)));
  const int length = DBGetVarLength(file.get(), varname.c_str());
  assert(length >= 0);
  vector<T> data(length);
  ierr = DBReadVar(file.get(), varname.c_str(), data.data());
  assert(!ierr);
  return data;
}

////////////////////////////////////////////////////////////////////////////////

void list_multimesh(const Silo<DBfile> &file, const string &multimeshname,
                    const string &indent) {
  const Silo<const DBmultimesh> multimesh =
      MakeSilo(DBGetMultimesh(file.get(), multimeshname.c_str()));
  assert(multimesh);
  cout << indent << quote_string(multimeshname) << ":\n";
  assert(multimesh->ngroups == 0);
  // meshids
  cout << indent << "  meshes:\n";
  for (int i = 0; i < multimesh->nblocks; ++i)
    cout << indent << "    - " << quote_string(multimesh->meshnames[i]) << "\n";
  assert(!multimesh->meshtypes);
  assert(!multimesh->dirids);
  // blockorigin
  // grouporigin
  // extentssize
  // extents
  assert(!multimesh->zonecounts);
  assert(!multimesh->has_external_zones);
  cout << indent << "  hide_from_gui: " << bool_string(multimesh->guihide)
       << "\n";
  assert(multimesh->lgroupings == 0);
  assert(!multimesh->groupings);
  assert(!multimesh->groupnames);
  assert(!multimesh->mrgtree_name);
  // tv_connectivity
  // disjoint_mode
  // topo_dim
  assert(!multimesh->file_ns);
  assert(!multimesh->block_ns);
  // block_type
  assert(!multimesh->empty_list);
  assert(multimesh->empty_cnt == 0);
  // repr_block_idx
  assert(!multimesh->alt_nodenum_vars);
  assert(!multimesh->alt_zonenum_vars);
  // meshnames_alloc
}

////////////////////////////////////////////////////////////////////////////////

void list_multivar(const Silo<DBfile> &file, const string &multivarname,
                   const string &indent) {
  const Silo<const DBmultivar> multivar =
      MakeSilo(DBGetMultivar(file.get(), multivarname.c_str()));
  assert(multivar);
  cout << indent << quote_string(multivarname) << ":\n";
  assert(multivar->ngroups == 0);
  cout << indent << "  variables:\n";
  for (int i = 0; i < multivar->nvars; ++i)
    cout << indent << "    - " << quote_string(multivar->varnames[i]) << "\n";
  assert(!multivar->vartypes);
  // blockorigin
  // grouporigin
  // extentssize
  // extents
  cout << indent << "  hide_from_gui: " << bool_string(multivar->guihide)
       << "\n";
  assert(!multivar->region_pnames);
  cout << indent << "  multimesh: " << quote_string(multivar->mmesh_name)
       << "\n";
  cout << indent
       << "  tensor_rank: " << tensorrank_string(multivar->tensor_rank) << "\n";
  // conserved
  // extensive
  assert(!multivar->file_ns);
  assert(!multivar->block_ns);
  // block_type
  assert(!multivar->empty_list);
  assert(multivar->empty_cnt == 0);
  // repr_block_idx
  // missing_value
  // varnames_alloc
}

////////////////////////////////////////////////////////////////////////////////

void list_quadmesh(const Silo<DBfile> &file, const string &quadmeshname,
                   const string &indent) {
  const Silo<const DBquadmesh> quadmesh =
      MakeSilo(DBGetQuadmesh(file.get(), quadmeshname.c_str()));
  assert(quadmesh);
  cout << indent << quote_string(quadmeshname) << ":\n";
  assert(quadmesh->block_no == -1);
  assert(quadmesh->group_no == -1);
  // cout << indent << "  name: " << quote_string(quadmesh->name) << "\n";
  // cycle
  cout << indent
       << "  coordinate_system: " << coordsys_string(quadmesh->coord_sys)
       << "\n";
  cout << indent << "  column_major: " << bool_string(quadmesh->major_order)
       << "\n";
  cout << indent << "  stride: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->stride[d] << ",";
  cout << "]\n";
  cout << indent
       << "  coordinate_type: " << coordtype_string(quadmesh->coordtype)
       << "\n";
  // facetype
  // planar
  cout << indent << "  coordinates:\n";
  for (int d = 0; d < quadmesh->ndims; ++d) {
    assert(quadmesh->datatype == DB_DOUBLE);
    const double *const coords =
        static_cast<const double *>(quadmesh->coords[d]);
    cout << indent << "    " << d << ": [";
    assert(quadmesh->coordtype == DB_COLLINEAR);
    for (int i = 0; i < quadmesh->dims[d]; ++i)
      cout << coords[i] << ", ";
    cout << "]\n";
  }
  // coords
  // datatype
  // time
  // dtime
  cout << indent << "  min_extents: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->min_extents[d] << ",";
  cout << "]\n";
  cout << indent << "  max_extents: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->max_extents[d] << ",";
  cout << "]\n";
  // labels
  // units
  // nspace
  // nnodes
  cout << indent << "  dimensions: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->dims[d] << ",";
  cout << "]\n";
  cout << indent << "  origin: " << quadmesh->origin << "\n";
  cout << indent << "  min_index: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->min_index[d] << ",";
  cout << "]\n";
  cout << indent << "  max_index: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->max_index[d] << ",";
  cout << "]\n";
  cout << indent << "  base_index: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->base_index[d] << ",";
  cout << "]   # ???\n";
  cout << indent << "  start_index: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->start_index[d] << ",";
  cout << "]   # ???\n";
  cout << indent << "  size_index: [";
  for (int d = 0; d < quadmesh->ndims; ++d)
    cout << quadmesh->size_index[d] << ",";
  cout << "]   # ???\n";
  cout << indent << "  hide_from_gui: " << bool_string(quadmesh->guihide)
       << "\n";
  assert(!quadmesh->mrgtree_name);
  assert(!quadmesh->ghost_node_labels);
  assert(!quadmesh->ghost_zone_labels);
  assert(!quadmesh->alt_nodenum_vars);
  assert(!quadmesh->alt_zonenum_vars);
}

////////////////////////////////////////////////////////////////////////////////

void list_quadvar(const Silo<DBfile> &file, const string &quadvarname,
                  const string &indent) {
  const Silo<const DBquadvar> quadvar =
      MakeSilo(DBGetQuadvar(file.get(), quadvarname.c_str()));
  assert(quadvar);
  cout << indent << quote_string(quadvarname) << ":\n";
  // cout << indent << "  name: " << quote_string(quadvar->name) << "\n";
  assert(!quadvar->units);
  assert(!quadvar->label);
  // cycle
  // vals
  cout << indent << "  type: " << vartype_string(quadvar->datatype) << "\n";
  // nels
  assert(quadvar->nvals == 1);
  cout << indent << "  dimensions: [";
  for (int d = 0; d < quadvar->ndims; ++d)
    cout << quadvar->dims[d] << ",";
  cout << "]\n";
  cout << indent << "  column_major: " << bool_string(quadvar->major_order)
       << "\n";
  cout << indent << "  stride: [";
  for (int d = 0; d < quadvar->ndims; ++d)
    cout << quadvar->stride[d] << ",";
  cout << "]\n";
  cout << indent << "  min_index: [";
  for (int d = 0; d < quadvar->ndims; ++d)
    cout << quadvar->min_index[d] << ",";
  cout << "]\n";
  cout << indent << "  max_index: [";
  for (int d = 0; d < quadvar->ndims; ++d)
    cout << quadvar->max_index[d] << ",";
  cout << "]\n";
  cout << indent << "  origin: " << quadvar->origin << "\n";
  // time
  // dtime
  cout << indent << "  align: [";
  for (int d = 0; d < quadvar->ndims; ++d)
    cout << quadvar->align[d] << ",";
  cout << "]\n";
  assert(!quadvar->mixvals);
  assert(quadvar->mixlen == 0);
  // use_specmf
  // ascii_labels
  cout << indent << "  mesh: " << quote_string(quadvar->meshname) << "\n";
  cout << indent << "  hide_from_gui: " << bool_string(quadvar->guihide)
       << "\n";
  assert(!quadvar->region_pnames);
  // conserved
  // extensive
  cout << indent << "  centering: " << centering_string(quadvar->centering)
       << "\n";
  // missing_value
}

////////////////////////////////////////////////////////////////////////////////

void list_var_char(const Silo<DBfile> &file, const string &varname) {
  const string var = read_char_var(file, varname);
  cout << quote_string(var);
}

template <typename T>
void list_var_typed(const Silo<DBfile> &file, const string &varname) {
  const vector<T> var = read_var<T>(file, varname);
  cout << "[";
  for (const T x : var)
    cout << x << ", ";
  cout << "]";
}

void list_var(const Silo<DBfile> &file, const string &varname,
              const string &indent) {
  const int type = DBGetVarType(file.get(), varname.c_str());
  // const int length = DBGetVarLength(file.get(), varname.c_str());
  const int maxdims = 100;
  vector<int> dims(maxdims, -1);
  const int ndims =
      DBGetVarDims(file.get(), varname.c_str(), maxdims, dims.data());
  assert(ndims >= 0);
  assert(ndims <= maxdims);
  dims.resize(ndims);
  cout << indent << quote_string(varname) << ":\n";
  cout << indent << "  type: " << vartype_string(type) << "\n";
  cout << indent << "  dimensions: [";
  for (const int dim : dims)
    cout << dim << ",";
  cout << "]\n";
  cout << indent << "  value: ";
  switch (type) {
  case DB_CHAR:
    list_var_char(file, varname);
    break;
  case DB_SHORT:
    list_var_typed<short>(file, varname);
    break;
  case DB_INT:
    list_var_typed<int>(file, varname);
    break;
  case DB_LONG:
    list_var_typed<long>(file, varname);
    break;
  case DB_LONG_LONG:
    list_var_typed<long long>(file, varname);
    break;
  case DB_FLOAT:
    list_var_typed<float>(file, varname);
    break;
  case DB_DOUBLE:
    list_var_typed<double>(file, varname);
    break;
  default:
    assert(0);
  }
  cout << "\n";
}

////////////////////////////////////////////////////////////////////////////////

void list_obj(const Silo<DBfile> &file, const string &objname,
              const string &indent) {
  cout << indent << "- " << quote_string(objname) << "\n";
}

////////////////////////////////////////////////////////////////////////////////

void list_dir(const Silo<DBfile> &file, const string &dirname,
              const string &indent) {
  int ierr;
  char saved_dirname[256];
  ierr = DBGetDir(file.get(), saved_dirname);
  assert(!ierr);
  ierr = DBSetDir(file.get(), dirname.c_str());
  assert(!ierr);
  // char current_dirname[256];
  // ierr = DBGetDir(file.get(), current_dirname);
  // assert(!ierr);

  cout << indent << quote_string(dirname) << ":\n";
  // cout << indent << "  # directory: " << current_dirname << "\n";
  const string nextindent = indent + "    ";
  const Silo<const DBtoc> toc = MakeSilo(DBGetToc(file.get()));
  assert(toc);
  assert(toc->ncurve == 0);
  const vector<string> multimeshnames(toc->multimesh_names,
                                      toc->multimesh_names + toc->nmultimesh);
  assert(toc->nmultimeshadj == 0);
  const vector<string> multivarnames(toc->multivar_names,
                                     toc->multivar_names + toc->nmultivar);
  assert(toc->nmultimat == 0);
  assert(toc->nmultimatspecies == 0);
  assert(toc->ncsgmesh == 0);
  assert(toc->ncsgvar == 0);
  assert(toc->ndefvars == 0);
  const vector<string> quadmeshnames(toc->qmesh_names,
                                     toc->qmesh_names + toc->nqmesh);
  const vector<string> quadvarnames(toc->qvar_names,
                                    toc->qvar_names + toc->nqvar);
  assert(toc->nucdmesh == 0);
  assert(toc->nucdvar == 0);
  assert(toc->nptmesh == 0);
  assert(toc->nptvar == 0);
  assert(toc->nmat == 0);
  assert(toc->nmatspecies == 0);
  const vector<string> varnames(toc->var_names, toc->var_names + toc->nvar);
  const vector<string> objnames(toc->obj_names, toc->obj_names + toc->nobj);
  const vector<string> dirnames(toc->dir_names, toc->dir_names + toc->ndir);
  assert(toc->narray == 0);
  assert(toc->nmrgtree == 0);
  assert(toc->ngroupelmap == 0);
  assert(toc->nmrgvar == 0);
  if (!multimeshnames.empty()) {
    cout << indent << "  multimeshes:\n";
    for (const auto &multimeshname : multimeshnames)
      list_multimesh(file, multimeshname, nextindent);
  }
  if (!multivarnames.empty()) {
    cout << indent << "  multivariables:\n";
    for (const auto &multivarname : multivarnames)
      list_multivar(file, multivarname, nextindent);
  }
  if (!quadmeshnames.empty()) {
    cout << indent << "  quadmeshes:\n";
    for (const auto &quadmeshname : quadmeshnames)
      list_quadmesh(file, quadmeshname, nextindent);
  }
  if (!quadvarnames.empty()) {
    cout << indent << "  quadvariables:\n";
    for (const auto &quadvarname : quadvarnames)
      list_quadvar(file, quadvarname, nextindent);
  }
  if (!varnames.empty()) {
    cout << indent << "  variables:\n";
    for (const auto &varname : varnames)
      list_var(file, varname, nextindent);
  }
  if (!objnames.empty()) {
    cout << indent << "  objects:\n";
    for (const auto &objname : objnames)
      list_obj(file, objname, nextindent);
  }
  if (!dirnames.empty()) {
    cout << indent << "  directories:\n";
    for (const auto &dirname : dirnames)
      list_dir(file, dirname, nextindent);
  }

  ierr = DBSetDir(file.get(), saved_dirname);
  assert(!ierr);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Synopsis:\n"
            "silo-list <filename>\n";
    exit(1);
  }

  cout << "# silo-list: Display contents of a Silo file\n";
  cout << "---\n";

  const string filename(argv[1]);
  const Silo<DBfile> file =
      MakeSilo(DBOpen(filename.c_str(), DB_UNKNOWN, DB_READ));
  assert(file);
  cout << "filename: " << quote_string(filename) << "\n";
  const string version = DBVersion();
  cout << "version: " << quote_string(version) << "\n";
  const string file_version = DBFileVersion(file.get());
  cout << "file_version: " << quote_string(file_version) << "\n";

  cout << "directories:\n";
  list_dir(file, "/", "  ");

  cout << "...\n";
  return 0;
}
