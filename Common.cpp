#include "Common.hpp"

namespace SimulationIO {

const asdf_writer_ Common::asdf_writer(ASDF::writer &w) const {
  return asdf_writer_(*this, w);
}

asdf_writer_::asdf_writer_(const Common &common, ASDF::writer &w)
    : m_common(common), m_writer(w) {
  m_writer << YAML::LocalTag("sio", m_common.type() + "-1.0.0");
  m_writer << YAML::Anchor(m_common.yaml_alias());
  m_writer << YAML::BeginMap;
  m_writer << YAML::Key << "name" << YAML::Value << m_common.name();
}

asdf_writer_::~asdf_writer_() { m_writer << YAML::EndMap; }

} // namespace SimulationIO
