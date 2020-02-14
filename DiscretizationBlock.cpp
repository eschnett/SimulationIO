#include "DiscretizationBlock.hpp"

#include "CoordinateField.hpp"
#include "CoordinateSystem.hpp"
#include "DataBlock.hpp"
#include "DiscreteField.hpp"
#include "DiscreteFieldBlock.hpp"
#include "DiscreteFieldBlockComponent.hpp"

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

#ifdef SIMULATIONIO_HAVE_SILO
void DiscretizationBlock::read(
    const Silo<DBfile> &file, const string &loc,
    const shared_ptr<Discretization> &discretization) {
  read_attribute(m_name, file, loc, "name");
  m_discretization = discretization;
  const int box_exists = DBInqVarExists(file.get(), (loc + "box").c_str());
  if (box_exists) {
    const DBObjectType objtype =
        DBInqVarType(file.get(), (loc + "box").c_str());
    if (objtype == DB_USERDEF) {
      m_box = box_t(point_t(0), point_t(0));
      assert(!m_box.empty());
    } else {
      // assert(objtype == DB_QUADMESH);
      assert(objtype == DB_QUAD_RECT); // why?
      const auto &box =
          MakeSilo(DBGetQuadmesh(file.get(), (loc + "box").c_str()));
      if (box) {
        const int dims = box->ndims;
        assert(dims <= 3);
        const vector<int> base(&box->base_index[0], &box->base_index[dims]),
            size(&box->size_index[0], &box->size_index[dims]);
        m_box = box_t(point_t(base), point_t(base) + point_t(size));
      }
    }
  }
  const int active_exists =
      DBInqVarExists(file.get(), (loc + "active").c_str());
  if (active_exists) {
    const auto &active =
        MakeSilo(DBGetMultimesh(file.get(), (loc + "active").c_str()));
    if (active) {
      vector<box_t> boxes;
      for (int n = 0; n < active->nblocks; ++n) {
        const char *const meshname = active->meshnames[n];
        const auto &box = MakeSilo(DBGetQuadmesh(file.get(), meshname));
        const int dims = box->ndims;
        const vector<int> base(&box->base_index[0], &box->base_index[dims]),
            size(&box->size_index[0], &box->size_index[dims]);
        boxes.emplace_back(point_t(base), point_t(base) + point_t(size));
      }
      m_active = region_t(move(boxes));
    }
  }
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

#ifdef SIMULATIONIO_HAVE_SILO
string DiscretizationBlock::silo_path() const {
  return discretization()->silo_path() + "discretizationblocks/" +
         legalize_silo_name(name()) + "/";
}

void DiscretizationBlock::write(const Silo<DBfile> &file,
                                const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "name", name());
  const auto b = box();
  if (b.valid()) {
    if (b.rank() == 0) {
      assert(!b.empty()); // we cannot write empty boxes
      DBobject *const obj = DBMakeObject((loc + "box").c_str(), DB_USERDEF, 0);
      assert(obj);
      int ierr = DBAddIntComponent(obj, "dummy", 0);
      assert(!ierr);
      ierr = DBWriteObject(file.get(), obj, 1);
      assert(!ierr);
    } else {
      vector<vector<double>> coords(b.rank());
      for (size_t d = 0; d < coords.size(); ++d) {
        auto &coordsd = coords.at(d);
        coordsd.resize(b.shape()[d]);
      }
      // Look for a coordinate system
      const auto &manifold = discretization()->manifold();
      const auto &coordinatesystems = manifold->coordinatesystems();
      // Use first coordinate system that has DataRange objects
      bool found_coordinates = false;
      vector<shared_ptr<DataRange>> dataranges(coords.size());
      for (const auto &kv : coordinatesystems) {
        const auto &coordinatesystem = kv.second.lock();
        dataranges.assign(dataranges.size(), nullptr);
        for (size_t d = 0; d < dataranges.size(); ++d) {
          // Assume the coordinate field exists
          if (!coordinatesystem->directions().count(d))
            break;
          const auto &coordinatefield = coordinatesystem->directions().at(d);
          const auto &field = coordinatefield->field();
          // Assume the coordinate field has the same discretization
          shared_ptr<DiscreteField> discretefield;
          for (const auto &kv : field->discretefields()) {
            const auto &df = kv.second;
            if (df->discretization() == discretization()) {
              discretefield = df;
              break;
            }
          }
          if (!discretefield)
            break;
          // Assume the discretization block exists
          shared_ptr<DiscreteFieldBlock> discretefieldblock;
          for (const auto &kv : discretefield->discretefieldblocks()) {
            const auto &dfb = kv.second;
            if (dfb->discretizationblock().get() == this) {
              discretefieldblock = dfb;
              break;
            }
          }
          if (!discretefieldblock)
            break;
          const auto &discretefieldblockcomponents =
              discretefieldblock->discretefieldblockcomponents();
          // Coordinate fields are scalars
          assert(discretefieldblockcomponents.size() == 1);
          const auto &discretefieldblockcomponent =
              discretefieldblockcomponents.begin()->second;
          // Assume the discrete field block component is a datarange
          const auto &datarange = discretefieldblockcomponent->datarange();
          if (!datarange)
            break;
          dataranges.at(d) = datarange;
        }
        found_coordinates = true;
        for (size_t d = 0; d < dataranges.size(); ++d)
          found_coordinates &= bool(dataranges.at(d));
        if (found_coordinates)
          break;
      } // for coordinatesystem
      if (found_coordinates) {
        for (size_t d = 0; d < coords.size(); ++d) {
          auto &coordsd = coords.at(d);
          const auto &datarange = dataranges.at(d);
          const double origin = datarange->origin();
          const double delta = datarange->delta().at(d);
          for (size_t i = 0; i < coordsd.size(); ++i)
            coordsd.at(i) = origin + i * delta;
        }
      } else {
        // Invent a coordinate system
        for (size_t d = 0; d < coords.size(); ++d) {
          auto &coordsd = coords.at(d);
          const auto lo = b.lower()[d];
          for (size_t i = 0; i < coordsd.size(); ++i)
            coordsd.at(i) = lo + i;
        }
      }
      assert(b.rank() <= 3);
      vector<int> dims(3, 0);
      for (size_t d = 0; d < dims.size(); ++d) {
        const auto sh = b.shape()[d];
        assert(sh >= INT_MIN && sh <= INT_MAX);
        dims.at(d) = sh;
      }
      const auto &optlist = MakeSilo(DBMakeOptlist(10));
      assert(optlist);
      int cartesian = DB_CARTESIAN;
      int ierr = DBAddOption(optlist.get(), DBOPT_COORDSYS, &cartesian);
      assert(!ierr);
      int ione = 1;
      ierr = DBAddOption(optlist.get(), DBOPT_MAJORORDER, &ione);
      assert(!ierr);
      assert(b.rank() <= 3);
      vector<int> base(3, 0);
      for (size_t d = 0; d < dims.size(); ++d) {
        const auto lo = b.lower()[d];
        assert(lo >= INT_MIN && lo <= INT_MAX);
        base.at(d) = lo;
      }
      ierr = DBAddOption(optlist.get(), DBOPT_BASEINDEX, base.data());
      assert(!ierr);
      ierr = DBPutQuadmesh(file.get(), (loc + "box").c_str(), nullptr,
                           coords.data(), dims.data(), b.rank(), DB_DOUBLE,
                           DB_COLLINEAR, optlist.get());
      assert(!ierr);
    }
  }
  const auto a = active();
  if (a.valid()) {
    const vector<box_t> boxes(a);
    const int nboxes = boxes.size();
    vector<string> meshnames;
    size_t n = 0;
    for (const auto &b : boxes) {
      const string meshname = loc + "active_" + to_string(n++);
      vector<vector<double>> coords(b.rank());
      for (size_t d = 0; d < coords.size(); ++d) {
        auto &coordsd = coords.at(d);
        coordsd.resize(b.shape()[d]);
        const auto lo = b.lower()[d];
        for (size_t i = 0; i < coordsd.size(); ++i)
          coordsd.at(i) = lo + i;
      }
      vector<int> dims(b.rank());
      for (size_t d = 0; d < dims.size(); ++d) {
        const auto sh = b.shape()[d];
        assert(sh >= INT_MIN && sh <= INT_MAX);
        dims.at(d) = sh;
      }
      const auto &optlist = MakeSilo(DBMakeOptlist(10));
      assert(optlist);
      int cartesian = DB_CARTESIAN;
      int ierr = DBAddOption(optlist.get(), DBOPT_COORDSYS, &cartesian);
      assert(!ierr);
      int ione = 1;
      ierr = DBAddOption(optlist.get(), DBOPT_MAJORORDER, &ione);
      assert(!ierr);
      ierr = DBAddOption(optlist.get(), DBOPT_HIDE_FROM_GUI, &ione);
      assert(!ierr);
      assert(b.rank() <= 3);
      vector<int> base(3, 0);
      for (size_t d = 0; d < dims.size(); ++d) {
        const auto lo = b.lower()[d];
        assert(lo >= INT_MIN && lo <= INT_MAX);
        base.at(d) = lo;
      }
      ierr = DBAddOption(optlist.get(), DBOPT_BASEINDEX, base.data());
      assert(!ierr);
      ierr = DBPutQuadmesh(file.get(), meshname.c_str(), nullptr, coords.data(),
                           dims.data(), b.rank(), DB_DOUBLE, DB_COLLINEAR,
                           optlist.get());
      assert(!ierr);
      meshnames.push_back(meshname);
    }
    assert(n == nboxes);
    vector<const char *> meshnames_c;
    for (const auto &meshname : meshnames)
      meshnames_c.push_back(meshname.c_str());
    const auto &optlist = MakeSilo(DBMakeOptlist(10));
    assert(optlist);
    int quad_rect = DB_QUAD_RECT;
    int ierr = DBAddOption(optlist.get(), DBOPT_MB_BLOCK_TYPE, &quad_rect);
    assert(!ierr);
    // const int ione = 1;
    // ierr = DBAddOption(optlist, DBOPT_HIDE_FROM_GUI, &ione);
    // assert(!ierr);
    ierr = DBPutMultimesh(file.get(), (loc + "active").c_str(), nboxes,
                          meshnames_c.data(), nullptr, optlist.get());
    assert(!ierr);
  }
}

string DiscretizationBlock::silo_meshname() const {
  return silo_path() + "box";
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
