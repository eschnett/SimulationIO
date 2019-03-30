#include "Helpers.hpp"

namespace SimulationIO {

using std::ostringstream;
using std::string;
using std::vector;

// Indented output
string indent(int level) { return string(level * indentsize, indentchar); }

// Quote a string
string quote(const string &str) {
  ostringstream buf;
  buf << "\"";
  for (char ch : str) {
    if (ch == '"' || ch == '\\')
      buf << '\\';
    buf << ch;
  }
  buf << "\"";
  return buf.str();
}

// String queries
bool starts_with(const string &str, const string &prefix) {
  size_t strlen = str.length();
  size_t prefixlen = prefix.length();
  if (strlen < prefixlen)
    return false;
  return str.substr(0, prefixlen) == prefix;
}

bool ends_with(const string &str, const string &suffix) {
  size_t strlen = str.length();
  size_t suffixlen = suffix.length();
  if (strlen < suffixlen)
    return false;
  return str.substr(strlen - suffixlen) == suffix;
}

// Join a vector of strings
string joinpath(const vector<string> &path) {
  ostringstream buf;
  bool isstart = true;
  for (const auto &p : path) {
    if (!isstart)
      buf << '/';
    buf << p;
    isstart = false;
  }
  return buf.str();
}

string joinpath(const vector<string> &path, const vector<string> &path2) {
  ostringstream buf;
  bool isstart = true;
  for (const auto &p : path) {
    if (!isstart)
      buf << '/';
    buf << p;
    isstart = false;
  }
  for (const auto &p : path2) {
    if (!isstart)
      buf << '/';
    buf << p;
    isstart = false;
  }
  return buf.str();
}

} // namespace SimulationIO
