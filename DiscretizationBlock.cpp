#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

#include <algorithm>

namespace SimulationIO {

namespace {
template <int D>
void read_active(const H5::H5Location &group,
                 const DiscretizationBlock &discretizationblock,
                 region_t &active) {
  // The case D==0 is not yet handled correctly:
  // - C++ pads empty struct
  // - HDF5 cannot handle empty arrays
  static_assert(D > 0, "");
  if (active.valid())
    return;
  vector<RegionCalculus::box<long long, D>> boxes;
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  assert(sizeof(boxes[0]) == boxtype.getSize());
#if 1
  // Read the attribute if it exists, and if it has the right type
  H5E_BEGIN_TRY {
    try {
      H5::readAttribute(group, "active", boxes, boxtype);
      active = region_t(
          RegionCalculus::make_unique1<RegionCalculus::wregion<long long, D>>(
              RegionCalculus::region<long long, D>(std::move(boxes))));
    } catch (H5::AttributeIException ex) {
      // do nothing
    }
  }
  H5E_END_TRY;
#else
  if (group.attrExists("active")) {
    try {
      H5::readAttribute(group, "active", boxes, boxtype);
      active = region_t(
          RegionCalculus::make_unique1<RegionCalculus::wregion<long long, D>>(
              RegionCalculus::region<long long, D>(std::move(boxes))));
    } catch (H5::DataTypeIException ex) {
      // do nothing
    }
  }
#endif
}
}

void DiscretizationBlock::read(
    const H5::CommonFG &loc, const string &entry,
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
  }
  if (group.attrExists("active")) {
    // TODO read_active<0>(group, *this, active);
    read_active<1>(group, *this, m_active);
    read_active<2>(group, *this, m_active);
    read_active<3>(group, *this, m_active);
    read_active<4>(group, *this, m_active);
  }
}

void DiscretizationBlock::merge(
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  assert(discretization()->name() ==
         discretizationblock->discretization()->name());
  if (!m_box.valid())
    m_box = discretizationblock->box();
  if (!m_active.valid())
    m_active = discretizationblock->active();
  assert(m_box == discretizationblock->box());
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

namespace {
template <int D>
void write_active(const H5::H5Location &group,
                  const DiscretizationBlock &discretizationblock,
                  const region_t &active) {
  // The case D==0 is not yet handled correctly:
  // - C++ pads empty structs
  // - HDF5 cannot handle empty arrays
  static_assert(D > 0, "");
  if (active.rank() != D)
    return;
  vector<RegionCalculus::box<long long, D>> boxes =
      dynamic_cast<const RegionCalculus::wregion<long long, D> *>(
          active.val.get())
          ->val;
  auto boxtype =
      discretizationblock.discretization()->manifold()->project()->boxtypes.at(
          D);
  assert(sizeof boxes[0] == boxtype.getSize());
  H5::createAttribute(group, "active", boxes, boxtype);
}
}

void DiscretizationBlock::write(const H5::CommonFG &loc,
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
    // TODO: write using boxtype HDF5 type
    H5::createAttribute(group, "offset",
                        vector<long long>(box().lower().reversed()));
    H5::createAttribute(group, "shape",
                        vector<long long>(box().shape().reversed()));
  }
  if (active().valid()) {
    // TODO write_active<0>(group, *this, active);
    write_active<1>(group, *this, active());
    write_active<2>(group, *this, active());
    write_active<3>(group, *this, active());
    write_active<4>(group, *this, active());
  }
}
}
