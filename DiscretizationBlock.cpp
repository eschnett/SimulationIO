#include "DiscretizationBlock.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>
#include <array>

namespace SimulationIO {

bool DiscretizationBlock::invariant() const {
  return Common::invariant() && bool(discretization()) &&
         discretization()->discretizationblocks().count(name()) &&
         discretization()->discretizationblocks().at(name()).get() == this &&
         (!box().valid() ||
          (box().rank() == discretization()->manifold()->dimension() &&
           !box().empty())) &&
         (!active().valid() ||
          active().rank() == discretization()->manifold()->dimension());
}

#ifdef SIMULATIONIO_HAVE_HDF5
namespace {
template <int D>
void try_read_active(const H5::H5Object &group,
                     const DiscretizationBlock &discretizationblock,
                     region_t &active) {
  typedef RegionCalculus::box<long long, D> box_t;
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  static_assert(D > 0, "");
  assert(sizeof(box_t) == boxtype.getSize());
  auto attr = group.openAttribute("active");
  auto ftype = attr.getDataType();
  if (ftype == boxtype) {
    assert(!active.valid());
    vector<box_t> boxes;
    H5::readAttribute(group, "active", boxes, boxtype);
    active = region_t(
        RegionCalculus::make_unique1<RegionCalculus::wregion<long long, D>>(
            RegionCalculus::region<long long, D>(std::move(boxes))));
  }
}

template <>
void try_read_active<0>(const H5::H5Object &group,
                        const DiscretizationBlock &discretizationblock,
                        region_t &active) {
  constexpr int D = 0;
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  static_assert(D == 0, "");
  assert(sizeof(int) == boxtype.getSize());
  auto attr = group.openAttribute("active");
  auto ftype = attr.getDataType();
  auto fclass = ftype.getClass();
  if (ftype == boxtype) {
    assert(!active.valid());
    vector<int> iboxes;
    H5::readAttribute(group, "active", iboxes, boxtype);
    typedef RegionCalculus::box<long long, D> box_t;
    vector<box_t> boxes;
    for (const auto &ibox : iboxes)
      boxes.push_back(box_t(bool(ibox)));
    active = region_t(
        RegionCalculus::make_unique1<RegionCalculus::wregion<long long, D>>(
            RegionCalculus::region<long long, D>(std::move(boxes))));
  } else if (fclass == H5T_INTEGER) {
    // For backward compatibility
    assert(!active.valid());
    vector<int> iboxes;
    H5::readAttribute(group, "active", iboxes);
    typedef RegionCalculus::box<long long, D> box_t;
    vector<box_t> boxes;
    for (const auto &ibox : iboxes)
      boxes.push_back(box_t(bool(ibox)));
    active = region_t(
        RegionCalculus::make_unique1<RegionCalculus::wregion<long long, D>>(
            RegionCalculus::region<long long, D>(std::move(boxes))));
  }
}
} // namespace

void DiscretizationBlock::read(
    const H5::H5Location &loc, const string &entry,
    const shared_ptr<Discretization> &discretization) {
  m_discretization = discretization;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", discretization->manifold()->project()->enumtype) ==
         "DiscretizationBlock");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "discretization", "name") ==
         discretization->name());
  if (group.attrExists("offset")) {
    vector<long long> offset, shape;
    H5::readAttribute(group, "offset", offset);
    reverse(offset);
    H5::readAttribute(group, "shape", shape);
    reverse(shape);
    m_box = box_t(point_t(offset), point_t(offset) + point_t(shape));
    if (m_box.rank() == 0)
      assert(!m_box.empty()); // for consistency with writing
  }
  if (group.attrExists("active")) {
    try_read_active<0>(group, *this, m_active);
    try_read_active<1>(group, *this, m_active);
    try_read_active<2>(group, *this, m_active);
    try_read_active<3>(group, *this, m_active);
    try_read_active<4>(group, *this, m_active);
  }
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DiscretizationBlock::read(
    const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
    const shared_ptr<Discretization> &discretization) {
  assert(node.Tag() == "tag:github.com/eschnett/SimulationIO/asdf-cxx/"
                       "DiscretizationBlock-1.0.0");
  m_name = node["name"].Scalar();
  m_discretization = discretization;
  const auto &box = node["box"];
  if (box.IsDefined())
    m_box = box_t(box);
  const auto &active = node["active"];
  if (active.IsDefined())
    m_active = region_t(active);
}
#endif

void DiscretizationBlock::merge(
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  assert(discretization()->name() ==
         discretizationblock->discretization()->name());
  if (!m_box.valid())
    m_box = discretizationblock->box();
  if (!m_active.valid())
    m_active = discretizationblock->active();
  if (discretizationblock->box().valid())
    assert(m_box == discretizationblock->box());
  if (discretizationblock->active().valid())
    assert(m_active == discretizationblock->active());
}

ostream &DiscretizationBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscretizationBlock " << quote(name())
     << ": Discretization " << quote(discretization()->name());
  if (box().valid())
    os << " box=" << box();
  if (active().valid())
    os << " active=" << active();
  os << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
namespace {
template <int D>
void try_write_active(const H5::H5Object &group,
                      const DiscretizationBlock &discretizationblock,
                      const region_t &active) {
  if (active.rank() != D)
    return;
  typedef RegionCalculus::box<long long, D> box_t;
  vector<box_t> boxes =
      dynamic_cast<const RegionCalculus::wregion<long long, D> *>(
          active.val.get())
          ->val;
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  static_assert(D > 0, "");
  assert(sizeof(box_t) == boxtype.getSize());
  H5::createAttribute(group, "active", boxes, boxtype);
}

template <>
void try_write_active<0>(const H5::H5Object &group,
                         const DiscretizationBlock &discretizationblock,
                         const region_t &active) {
  constexpr int D = 0;
  if (active.rank() != D)
    return;
  typedef RegionCalculus::box<long long, D> box_t;
  vector<box_t> boxes =
      dynamic_cast<const RegionCalculus::wregion<long long, D> *>(
          active.val.get())
          ->val;
  vector<int> iboxes;
  for (const auto &box : boxes)
    iboxes.push_back(!box.empty());
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  assert(sizeof(int) == boxtype.getSize());
  H5::createAttribute(group, "active", iboxes, boxtype);
}
} // namespace

void DiscretizationBlock::write(const H5::H5Location &loc,
                                const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      discretization()->manifold()->project()->enumtype,
                      "DiscretizationBlock");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "discretization", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "discretization", "..");
  if (box().valid()) {
    if (box().rank() == 0)
      assert(!box().empty()); // we cannot write empty boxes
    // TODO: write using boxtype HDF5 type
    H5::createAttribute(group, "offset",
                        vector<long long>(box().lower().reversed()));
    H5::createAttribute(group, "shape",
                        vector<long long>(box().shape().reversed()));
  }
  if (active().valid()) {
    try_write_active<0>(group, *this, active());
    try_write_active<1>(group, *this, active());
    try_write_active<2>(group, *this, active());
    try_write_active<3>(group, *this, active());
    try_write_active<4>(group, *this, active());
  }
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> DiscretizationBlock::yaml_path() const {
  return concat(discretization()->yaml_path(),
                {"discretizationblocks", name()});
}

ASDF::writer &DiscretizationBlock::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  if (box().valid())
    aw.value("box", box());
  if (active().valid())
    aw.value("active", active());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> DiscretizationBlock::tiledb_path() const {
  return concat(discretization()->tiledb_path(),
                {"discretizationblocks", name()});
}

namespace {
template <int D>
void write_region(const tiledb_writer &w,
                  const DiscretizationBlock &discretizationblock,
                  const string &name, const region_t &region) {
  assert(region.valid() && region.rank() == D);

  typedef RegionCalculus::region<long long, D> regionD_t;
  regionD_t regionD(region);
  typedef RegionCalculus::box<long long, D> boxD_t;
  vector<boxD_t> boxesD(regionD);

  typedef array<long long, D> ipoint_t;
  typedef array<ipoint_t, 2> ibox_t;
  vector<ibox_t> iboxes;
  for (const auto &box : boxesD) {
    ipoint_t ilo(box.lower());
    ipoint_t ihi(box.upper());
    ibox_t ibox{ilo, ihi};
    iboxes.push_back(ibox);
  }

  w.add_attribute(name, iboxes);
}
template <>
void write_region<0>(const tiledb_writer &w,
                     const DiscretizationBlock &discretizationblock,
                     const string &name, const region_t &region) {
  assert(region.valid() && region.rank() == 0);

  vector<box_t> boxes(region);

  vector<unsigned char> bboxes;
  for (const auto &box : boxes)
    bboxes.push_back(!box.empty());

  w.add_attribute(name, bboxes);
}
} // namespace

void DiscretizationBlock::write(const tiledb::Context &ctx,
                                const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);

  auto b = box();
  if (b.valid()) {
    if (b.rank() == 0)
      assert(!b.empty()); // we cannot write empty boxes
    w.add_attribute("offset", vector<long long>(b.lower()));
    w.add_attribute("shape", vector<long long>(b.shape()));
  }
  auto a = active();
  if (a.valid()) {
    switch (a.rank()) {
    case 0:
      write_region<0>(w, *this, "active", a);
      break;
    case 1:
      write_region<1>(w, *this, "active", a);
      break;
    case 2:
      write_region<2>(w, *this, "active", a);
      break;
    case 3:
      write_region<3>(w, *this, "active", a);
      break;
    case 4:
      write_region<4>(w, *this, "active", a);
      break;
    default:
      assert(0);
    }
  }
}
#endif

} // namespace SimulationIO
