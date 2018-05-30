#include "Common.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace SimulationIO {

using std::hex;
using std::oct;
using std::ostringstream;
using std::setfill;
using std::setw;

// Python's YAML library only accepts alphanumerical characters and "-_" in
// anchors
string quote_alias(const string &alias) {
  ostringstream buf;
  for (auto ch : alias)
    if (isalnum(ch))
      buf << ch;
    else if (ch == '/')
      buf << '-';
    else
      // buf << oct << setw(3) << setfill('0') << int(ch);
      buf << '_' << hex << setw(2) << setfill('0') << int(ch);
  return buf.str();
}

const asdf_writer_ Common::asdf_writer(ASDF::writer &w) const {
  return asdf_writer_(*this, w);
}

asdf_writer_::asdf_writer_(const Common &common, ASDF::writer &w)
    : m_common(common), m_writer(w) {
  m_writer << YAML::LocalTag("sio", m_common.type() + "-1.0.0");
  m_writer << YAML::Anchor(quote_alias(m_common.yaml_alias()));
  m_writer << YAML::BeginMap;
  m_writer << YAML::Key << "name" << YAML::Value << m_common.name();
}

asdf_writer_::~asdf_writer_() { m_writer << YAML::EndMap; }

} // namespace SimulationIO
