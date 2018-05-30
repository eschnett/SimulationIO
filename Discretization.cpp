#include "Discretization.hpp"

#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

bool Discretization::invariant() const {
  return Common::invariant() && bool(manifold()) &&
         manifold()->discretizations().count(name()) &&
         manifold()->discretizations().at(name()).get() == this &&
         bool(configuration()) &&
         configuration()->discretizations().count(name()) &&
         configuration()->discretizations().at(name()).lock().get() == this;
}

void Discretization::read(const H5::H5Location &loc, const string &entry,
                          const shared_ptr<Manifold> &manifold) {
  m_manifold = manifold;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", manifold->project()->enumtype) == "Discretization");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name());
  m_configuration = manifold->project()->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/discretizations/" + name(), "name") ==
         name());
  H5::readGroup(group, "discretizationblocks",
                [&](const H5::Group &group, const string &name) {
                  readDiscretizationBlock(group, name);
                });
  m_configuration->insert(name(), shared_from_this());
}

void Discretization::read(const ASDF::reader_state &rs, const YAML::Node &node,
                          const shared_ptr<Manifold> &manifold) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Discretization-1.0.0");
  m_name = node["name"].Scalar();
  m_manifold = manifold;
  m_configuration = manifold->project()->configurations().at(
      node["configuration"]["name"].Scalar());
  for (const auto &kv : node["discretizationblocks"])
    readDiscretizationBlock(rs, kv.second);
  m_configuration->insert(name(), shared_from_this());
}

void Discretization::merge(const shared_ptr<Discretization> &discretization) {
  assert(manifold()->name() == discretization->manifold()->name());
  assert(m_configuration->name() == discretization->configuration()->name());
  for (const auto &iter : discretization->discretizationblocks()) {
    const auto &discretizationblock = iter.second;
    if (!m_discretizationblocks.count(discretizationblock->name()))
      createDiscretizationBlock(discretizationblock->name());
    m_discretizationblocks.at(discretizationblock->name())
        ->merge(discretizationblock);
  }
}

ostream &Discretization::output(ostream &os, int level) const {
  os << indent(level) << "Discretization " << quote(name())
     << ": Configuration " << quote(configuration()->name()) << " Manifold "
     << quote(manifold()->name()) << "\n";
  for (const auto &db : discretizationblocks())
    db.second->output(os, level + 1);
  return os;
}

void Discretization::write(const H5::H5Location &loc,
                           const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", manifold()->project()->enumtype,
                      "Discretization");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "manifold", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "manifold", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "project/configurations/" +
  //                    configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../project/configurations/" + configuration()->name());
  H5::createHardLink(group,
                     "manifold/project/configurations/" +
                         configuration()->name() + "/discretizations",
                     name(), group, ".");
  H5::createGroup(group, "discretizationblocks", discretizationblocks());
  group.createGroup("child_discretizations");
  group.createGroup("parent_discretizations");
}

string Discretization::yaml_alias() const {
  return manifold()->yaml_alias() + "/" + type() + "/" + name();
}

ASDF::writer &Discretization::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.group("discretizationblocks", discretizationblocks());
  return w;
}

shared_ptr<DiscretizationBlock>
Discretization::createDiscretizationBlock(const string &name) {
  auto discretizationblock =
      DiscretizationBlock::create(name, shared_from_this());
  checked_emplace(m_discretizationblocks, discretizationblock->name(),
                  discretizationblock, "Discretization",
                  "discretizationblocks");
  assert(discretizationblock->invariant());
  return discretizationblock;
}

shared_ptr<DiscretizationBlock>
Discretization::getDiscretizationBlock(const string &name) {
  auto loc = m_discretizationblocks.find(name);
  if (loc != m_discretizationblocks.end()) {
    const auto &discretizationblock = loc->second;
    return discretizationblock;
  }
  return createDiscretizationBlock(name);
}

shared_ptr<DiscretizationBlock> Discretization::copyDiscretizationBlock(
    const shared_ptr<DiscretizationBlock> &discretizationblock,
    bool copy_children) {
  auto discretizationblock2 =
      getDiscretizationBlock(discretizationblock->name());
  auto box = discretizationblock->getBox();
  if (!box.valid())
    discretizationblock2->setBox();
  else
    discretizationblock2->setBox(box);
  auto active = discretizationblock->getActive();
  if (!active.valid())
    discretizationblock2->setActive();
  else
    discretizationblock2->setActive(active);
  return discretizationblock2;
}

shared_ptr<DiscretizationBlock>
Discretization::readDiscretizationBlock(const H5::H5Location &loc,
                                        const string &entry) {
  auto discretizationblock =
      DiscretizationBlock::create(loc, entry, shared_from_this());
  checked_emplace(m_discretizationblocks, discretizationblock->name(),
                  discretizationblock, "Discretization",
                  "discretizationblocks");
  assert(discretizationblock->invariant());
  return discretizationblock;
}

shared_ptr<DiscretizationBlock>
Discretization::readDiscretizationBlock(const ASDF::reader_state &rs,
                                        const YAML::Node &node) {
  auto discretizationblock =
      DiscretizationBlock::create(rs, node, shared_from_this());
  checked_emplace(m_discretizationblocks, discretizationblock->name(),
                  discretizationblock, "Discretization",
                  "discretizationblocks");
  assert(discretizationblock->invariant());
  return discretizationblock;
}

} // namespace SimulationIO
