#warning "TODO"
#include <iostream>

#include "Common.hpp"

#include "Helpers.hpp"

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace SimulationIO {

using std::hex;
using std::min;
using std::oct;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::string;
using std::vector;

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
const asdf_writer_ Common::asdf_writer(ASDF::writer &w) const {
  return asdf_writer_(*this, w);
}

asdf_writer_::asdf_writer_(const Common &common, ASDF::writer &w)
    : m_common(common), m_writer(w) {
  m_writer << YAML::LocalTag("sio", m_common.type() + "-1.0.0");
  m_writer << YAML::BeginMap;
  m_writer << YAML::Key << "name" << YAML::Value << m_common.name();
}

asdf_writer_::~asdf_writer_() { m_writer << YAML::EndMap; }
#endif

#ifdef SIMULATIONIO_HAVE_SILO
#if 0
vector<string> ls(DBfile *const file, const string &loc) {
  int ierr = DBSetDir(file, loc.c_str());
  assert(!ierr);
  const DBtoc *const toc = DBGetToc(file);
  vector<string> filenames;
  for (int n = 0; n < toc->ndir; ++n)
    filenames.push_back(toc->dir_names[n]);
  ierr = DBSetDir(file, "/");
  assert(!ierr);
  return filenames;
}

void mkdirp(DBfile *const file, const string &loc, const string &name) {
  int ierr = DBSetDir(file, loc.c_str());
  assert(!ierr);
  const DBtoc *const toc = DBGetToc(file);
  bool dir_exists = false;
  for (int n = 0; n < toc->ndir; ++n) {
    if (toc->dir_names[n] == name) {
      dir_exists = true;
      break;
    }
  }
  if (!dir_exists) {
    ierr = DBMkDir(file, name.c_str());
    assert(!ierr);
  }
  ierr = DBSetDir(file, "/");
  assert(!ierr);
}
#endif

string legalize_silo_name(const string &name) {
  ostringstream buf;
  for (const char c : name) {
    if (isalnum(c) || c == '_')
      buf << c;
    else
      buf << '_';
  }
  return buf.str();
}

void write_symlink(DBfile *const file, const string &loc, const string &name,
                   const string &value) {
  const int dims = value.size();
  int ierr =
      DBWrite(file, (loc + name).c_str(), value.c_str(), &dims, 1, DB_CHAR);
  assert(!ierr);
}

string read_symlink(DBfile *const file, const string &loc, const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_CHAR);
  const int length = DBGetVarLength(file, varname.c_str());
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  const string symlink(static_cast<const char *>(vardata), length);
  return symlink;
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const int value) {
  const int dims = 1;
  int ierr = DBWrite(file, (loc + name).c_str(), &value, &dims, 1, DB_INT);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const long long value) {
  const int dims = 1;
  int ierr =
      DBWrite(file, (loc + name).c_str(), &value, &dims, 1, DB_LONG_LONG);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const long value) {
  const int dims = 1;
  int ierr = DBWrite(file, (loc + name).c_str(), &value, &dims, 1, DB_LONG);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const double value) {
  const int dims = 1;
  int ierr = DBWrite(file, (loc + name).c_str(), &value, &dims, 1, DB_DOUBLE);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const string &value) {
  const int dims = value.size();
  int ierr =
      DBWrite(file, (loc + name).c_str(), value.c_str(), &dims, 1, DB_CHAR);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const vector<int> &values) {
  assert(values.size() <= INT_MAX);
  if (values.empty()) {
    DBobject *const obj = DBMakeObject((loc + name).c_str(), DB_USERDEF, 0);
    assert(obj);
    int ierr = DBAddIntComponent(obj, "dummy", 0);
    assert(!ierr);
    ierr = DBWriteObject(file, obj, 1);
    assert(!ierr);
  } else {
    const int dims = values.size();
    assert(dims >= 1);
    int ierr =
        DBWrite(file, (loc + name).c_str(), values.data(), &dims, 1, DB_INT);
    assert(!ierr);
  }
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const vector<long long> &values) {
  assert(values.size() <= INT_MAX);
  if (values.empty()) {
    DBobject *const obj = DBMakeObject((loc + name).c_str(), DB_USERDEF, 0);
    assert(obj);
    int ierr = DBAddIntComponent(obj, "dummy", 0);
    assert(!ierr);
    ierr = DBWriteObject(file, obj, 1);
    assert(!ierr);
  } else {
    const int dims = values.size();
    assert(dims >= 1);
    int ierr = DBWrite(file, (loc + name).c_str(), values.data(), &dims, 1,
                       DB_LONG_LONG);
    assert(!ierr);
  }
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const vector<double> &values) {
  assert(values.size() <= INT_MAX);
  const int dims = values.size();
  assert(dims >= 1);
  int ierr =
      DBWrite(file, (loc + name).c_str(), values.data(), &dims, 1, DB_DOUBLE);
  assert(!ierr);
}

bool has_attribute(DBfile *const file, const string &loc, const string &name) {
  const int iexists = DBInqVarExists(file, (loc + name).c_str());
  return iexists;
}

int attribute_type(DBfile *const file, const string &loc, const string &name) {
  const int datatype = DBGetVarType(file, (loc + name).c_str());
  return datatype;
}

void read_attribute(int &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_INT ||
         (sizeof(int) == sizeof(long) && datatype == DB_LONG));
  const int length = DBGetVarLength(file, varname.c_str());
  assert(length == 1);
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  value = *static_cast<const int *>(vardata);
}

void read_attribute(long &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_LONG ||
         (sizeof(long) == sizeof(int) && datatype == DB_INT) ||
         (sizeof(long) == sizeof(long long) && datatype == DB_LONG_LONG));
  const int length = DBGetVarLength(file, varname.c_str());
  assert(length == 1);
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  value = *static_cast<const long long *>(vardata);
}

void read_attribute(long long &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_LONG_LONG ||
         (sizeof(long long) == sizeof(long) && datatype == DB_LONG));
  const int length = DBGetVarLength(file, varname.c_str());
  assert(length == 1);
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  value = *static_cast<const long long *>(vardata);
}

void read_attribute(double &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_DOUBLE);
  const int length = DBGetVarLength(file, varname.c_str());
  assert(length == 1);
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  value = *static_cast<const double *>(vardata);
}

void read_attribute(string &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int datatype = DBGetVarType(file, varname.c_str());
  assert(datatype == DB_CHAR);
  const int length = DBGetVarLength(file, varname.c_str());
  const void *const vardata = DBGetVar(file, varname.c_str());
  assert(vardata);
  value = string(static_cast<const char *>(vardata), length);
}

void read_attribute(vector<int> &value, DBfile *const file, const string &loc,
                    const string &name) {
  const string varname = loc + name;
  const int vartype = DBInqVarType(file, varname.c_str());
  if (vartype == DB_USERDEF) {
    // empty array
    const int datatype = DBGetComponentType(file, varname.c_str(), "dummy");
    assert(datatype >= 0);
    if (!(datatype == DB_INT ||
          (sizeof(int) == sizeof(long) && datatype == DB_LONG)))
      std::cerr << "read_attribute<vector<int>>: varname=" << varname
                << " datatype=" << datatype << "\n";
    assert(datatype == DB_INT ||
           (sizeof(int) == sizeof(long) && datatype == DB_LONG));
    value.resize(0);
  } else {
    assert(vartype == DB_VARIABLE);
    const int datatype = DBGetVarType(file, varname.c_str());
    assert(datatype >= 0);
    if (!(datatype == DB_INT ||
          (sizeof(int) == sizeof(long) && datatype == DB_LONG)))
      std::cerr << "read_attribute<vector<int>>: varname=" << varname
                << " datatype=" << datatype << "\n";
    assert(datatype == DB_INT ||
           (sizeof(int) == sizeof(long) && datatype == DB_LONG));
    const int length = DBGetVarLength(file, varname.c_str());
    value.resize(length);
    int ierr = DBReadVar(file, varname.c_str(), value.data());
    assert(!ierr);
  }
}

void read_group(DBfile *const file, const string &loc, const string &name,
                const function<void(const string &loc)> &process_entry) {
  int ierr = DBSetDir(file, (loc + name).c_str());
  assert(!ierr);
  const DBtoc *const toc = DBGetToc(file);
  assert(toc);
  vector<string> dirs;
  for (int n = 0; n < toc->ndir; ++n)
    dirs.push_back(toc->dir_names[n]);
  ierr = DBSetDir(file, "/");
  assert(!ierr);
  for (const auto &dir : dirs)
    process_entry(loc + name + "/" + dir + "/");
}

void read_symlink_group(
    DBfile *const file, const string &loc, const string &name,
    const function<void(const string &loc)> &process_entry) {
  int ierr = DBSetDir(file, (loc + name).c_str());
  assert(!ierr);
  const DBtoc *const toc = DBGetToc(file);
  assert(toc);
  vector<string> vars;
  for (int n = 0; n < toc->nvar; ++n)
    vars.push_back(toc->var_names[n]);
  ierr = DBSetDir(file, "/");
  assert(!ierr);
  const string grouploc = loc + name + "/";
  for (const auto &var : vars) {
    const auto symlink = read_symlink(file, grouploc, var);
    process_entry(symlink);
  }
}

#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
tiledb_writer::tiledb_writer(const Common &common, const tiledb::Context &ctx,
                             const string &loc)
    : m_common(common), m_ctx(ctx), m_loc(loc) {
  tiledb::create_group(m_ctx, m_loc);
  add_attribute("type", m_common.type());
  add_attribute("name", m_common.name());
}

void tiledb_writer::add_attribute(const string &name, bool value) const {
  add_attribute(name, static_cast<bool>(value));
}
void tiledb_writer::add_attribute(const string &name, int value) const {
  add_attribute(name, static_cast<long long>(value));
}
void tiledb_writer::add_attribute(const string &name, long value) const {
  add_attribute(name, static_cast<long long>(value));
}
void tiledb_writer::add_attribute(const string &name, long long value) const {
  add_attribute_fixed(name, value);
}
void tiledb_writer::add_attribute(const string &name, double value) const {
  add_attribute_fixed(name, value);
}
void tiledb_writer::add_attribute(const string &name,
                                  const string &value) const {
  add_attribute_variable(name, value);
}

#if 0
void tiledb_writer::add_symlink(const string &name,
                                const string &destination) const {
  // We don't want absolute paths
  assert(!starts_with(name, "/"));
  // We don't want trailing slashes
  assert(!ends_with(name, "/"));
  string source = m_loc + "/" + name;

  // Normalize destination
  // We don't want absolute paths
  assert(!starts_with(destination, "/"));
  vector<string> path;
  ostringstream buf;
  for (auto ch : destination) {
    if (ch == '/') {
      path.push_back(buf.str());
      buf = ostringstream();
    } else {
      buf << ch;
    }
  }
  path.push_back(buf.str());
  buf = ostringstream();
  for (vector<string>::iterator p = path.begin(); p < path.end();) {
    if (*p == "" || *p == ".") {
      p = path.erase(p);
    } else if (p != path.begin() && *p == ".." && p[-1] != "..") {
      p = path.erase(p - 1, p + 1);
    } else {
      ++p;
    }
  }
  if (path.empty())
    path = {"."};
  for (vector<string>::const_iterator p = path.begin(); p < path.end(); ++p) {
    if (p != path.begin())
      buf << '/';
    buf << *p;
  }
  string target = buf.str();

  std::cout << "* Creating symlink from: \"" << source << "\"\n"
            << "*                    to: \"" << target << "\"\n";
  int ierr = symlink(target.c_str(), source.c_str());
  if (ierr) {
    std::cerr << "Failed to create symlink pointing from \"" << source
              << "\" to \"" << target << "\" (original destination \""
              << destination << "\")\n";
  }
  assert(!ierr);
  DIR *dir = opendir(source.c_str());
  if (!dir) {
    std::cerr << "The symlink pointing from \"" << source << "\" to \""
              << target << "\" (original destination \"" << destination
              << "\") does not point to a directory\n";
  }
  assert(dir);
  ierr = closedir(dir);
  assert(!ierr);
}
#endif

void tiledb_writer::add_symlink(const vector<string> &source_path,
                                const vector<string> &destination_path) const {
  assert(!source_path.empty());
  assert(!destination_path.empty());

  string source = joinpath(source_path);

  // Count the common prefix of source and destination path.
  // The last element of the path is the file name, not a directory name.
  size_t max_prefix = min(source_path.size(), destination_path.size()) - 1;
  size_t common_prefix = max_prefix;
  for (size_t i = 0; i < max_prefix; ++i)
    if (source_path[i] != destination_path[i]) {
      common_prefix = i;
      break;
    }

  // Construct destination
  ostringstream buf;
  // First go up several directories
  for (size_t i = common_prefix; i < source_path.size() - 1; ++i)
    buf << "../";
  // Then go downwards to the destination
  for (size_t i = common_prefix; i < destination_path.size(); ++i) {
    if (i > common_prefix)
      buf << '/';
    buf << destination_path[i];
  }
  string destination = buf.str();

  // std::cout << "Creating symlink from: \"" << source << "\"\n"
  //           << "        `          to: \"" << destination << "\"\n"
  //           << "        `  originally: \"" << joinpath(destination_path)
  //           << "\"\n";
  int ierr = symlink(destination.c_str(), source.c_str());
  if (ierr)
    std::cerr << "Failed to create symlink\n"
              << "      from: \"" << source << "\"\n"
              << "        to: \"" << destination << "\"\n"
              << "originally: \"" << joinpath(destination_path) << "\"\n";
  assert(!ierr);
  DIR *dir = opendir(source.c_str());
  if (!dir)
    std::cerr << "Symlink does not point to a directory\n"
              << "      from: \"" << source << "\"\n"
              << "        to: \"" << destination << "\"\n"
              << "originally: \"" << joinpath(destination_path) << "\"\n";
  assert(dir);
  ierr = closedir(dir);
  assert(!ierr);
}

void tiledb_writer::create_group(const string &name) const {
  // We don't handle absolute paths
  assert(!starts_with(name, "/"));
  // We don't want trailing slashes
  assert(!ends_with(name, "/"));
  string grouploc = m_loc + "/" + name;
  tiledb::create_group(m_ctx, grouploc);
}
#endif

} // namespace SimulationIO
