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

} // namespace SimulationIO
