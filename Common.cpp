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
void write_attribute(DBfile *const file, const string &loc, const string &name,
                     int value) {
  DBobject *const attr =
      DBMakeObject((loc + "/" + name).c_str(), DB_USERDEF, 0);
  assert(attr);
  int ierr = DBAddIntComponent(attr, "value", value);
  assert(!ierr);
  ierr = DBWriteObject(file, attr, 1);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     long long value) {
  assert(value >= INT_MIN && value <= INT_MAX);
  write_attribute(file, loc, name, int(value));
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     double value) {
  DBobject *const attr =
      DBMakeObject((loc + "/" + name).c_str(), DB_USERDEF, 0);
  assert(attr);
  int ierr = DBAddDblComponent(attr, "value", value);
  assert(!ierr);
  ierr = DBWriteObject(file, attr, 1);
  assert(!ierr);
}

void write_attribute(DBfile *const file, const string &loc, const string &name,
                     const string &value) {
  DBobject *const attr =
      DBMakeObject((loc + "/" + name).c_str(), DB_USERDEF, 0);
  assert(attr);
  int ierr = DBAddStrComponent(attr, "value", value.c_str());
  assert(!ierr);
  ierr = DBWriteObject(file, attr, 1);
  assert(!ierr);
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
