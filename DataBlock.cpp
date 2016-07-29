#include "DataBlock.hpp"

#include <algorithm>

namespace SimulationIO {

namespace {
vector<hsize_t> choose_chunksize(const vector<hsize_t> &size,
                                 hsize_t typesize) {
  // Guess a good chunk size:
  // - chunk should fit in L3 cache
  // - chunk should be larger than stripe size
  // - chunk should be larger than bandwidth-latency product
  // A typical L3 cache size is several MByte
  // A typical stripe size is 128 kByte
  // A typical disk latency-bandwidth product is
  //    L = 1 / (10,000/min) / 2 = 0.006 sec
  //    BW = 100 MByte/sec
  //    BW * L = 600 kByte
  // We choose a chunk size of 64^3:
  //    64^3 * 8 B = 2 MByte
  int dim = size.size();
  hsize_t length = 1;
  for (int d = 0; d < dim; ++d)
    length *= size.at(d);
  if (length == 0)
    return size;
  hsize_t target_bytes = 2 * 1024 * 2014;
  hsize_t target_chunklength =
      min(length, max(hsize_t(1), target_bytes / typesize));
  vector<hsize_t> chunksize(dim, 1);
  hsize_t chunklength = 1;
  assert(chunklength <= target_chunklength);
  for (;;) {
    hsize_t old_chunklength = chunklength;
    // Loop in C index order
    for (int dir = dim - 1; dir >= 0; --dir) {
      vector<hsize_t> new_chunksize(chunksize);
      new_chunksize.at(dir) = min(2 * chunksize.at(dir), size.at(dir));
      hsize_t new_chunklength = 1;
      for (int d = 0; d < dim; ++d)
        new_chunklength *= new_chunksize.at(d);
      if (new_chunklength > target_chunklength)
        return chunksize;
      chunksize = new_chunksize;
      chunklength = new_chunklength;
    }
    // Ensure progress
    if (chunklength == old_chunklength)
      return chunksize;
  }
}
}

// DataBlock

const vector<DataBlock::reader_t> DataBlock::readers = {
    DataRange::read, DataSet::read, CopyObj::read, ExtLink::read,
};

shared_ptr<DataBlock> DataBlock::read(const H5::Group &group,
                                      const string &entry, const box_t &box) {
  for (const auto &reader : readers) {
    auto datablock = reader(group, entry, box);
    if (datablock)
      return datablock;
  }
  return nullptr;
}

// DataRange

shared_ptr<DataRange> DataRange::read(const H5::Group &group,
                                      const string &entry, const box_t &box) {
  if (group.attrExists(entry + "_origin")) {
    // entry is an attribute
    double origin;
    H5::readAttribute(group, entry + "_origin", origin);
    vector<double> delta;
    H5::readAttribute(group, entry + "_delta", delta);
    reverse(delta);
    return make_shared<DataRange>(box, origin, delta);
  }
  return nullptr;
}

ostream &DataRange::output(ostream &os) const {
  return os << "DataRange: origin=" << origin() << " delta=" << delta();
}

void DataRange::write(const H5::Group &group, const string &entry) const {
  H5::createAttribute(group, entry + "_origin", origin());
  auto tmp = reversed(delta());
  H5::createAttribute(group, entry + "_delta", tmp);
}

// DataSet

shared_ptr<DataSet> DataSet::read(const H5::Group &group, const string &entry,
                                  const box_t &box) {
// Reading a dataset always produces either CopyObj
#if 0
  auto lapl = H5::take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto exists = H5Lexists(group.getLocId(), entry.c_str(), lapl);
  assert(exists >= 0);
  if (exists) {
    // entry is a link
    // Check whether it is an external link
    bool have_extlink;
    string extlink_filename, extlink_objname;
    H5::readExternalLink(group, entry, have_extlink, extlink_filename,
                         extlink_objname);
    if (!have_extlink) {
      H5O_info_t info;
      herr_t herr =
          H5Oget_info_by_name(group.getLocId(), entry.c_str(), &info, lapl);
      assert(!herr);
      assert(info.type == H5O_TYPE_DATASET);
      auto dataset = group.openDataSet(entry);
      auto datatype = H5::DataType(H5Dget_type(dataset.getId()));
      auto dataspace = dataset.getSpace();
      return make_shared<DataSet>(dataspace, datatype, dataset);
    }
  }
#endif
  return nullptr;
}

ostream &DataSet::output(ostream &os) const {
  auto cls = datatype().getClass();
  auto clsname = H5::className(cls);
  auto typesize = datatype().getSize();
  assert(dataspace().isSimple());
  int dim = dataspace().getSimpleExtentNdims();
  vector<hsize_t> size(dim);
  dataspace().getSimpleExtentDims(size.data());
  reverse(size);
  os << "DataSet: type=" << clsname << "(" << (8 * typesize)
     << " bit) shape=" << size;
  return os;
}

void DataSet::write(const H5::Group &group, const string &entry) const {
  assert(!m_have_dataset);
  m_location_group = group;
  m_location_name = entry;
  m_have_location = true;
  create_dataset();
}

void DataSet::create_dataset() const {
  if (m_have_dataset)
    return;
  assert(m_have_location);
  auto proplist = H5::DSetCreatPropList();
  assert(dataspace().isSimple());
  const int dim = dataspace().getSimpleExtentNdims();
  vector<hsize_t> size(dim);
  dataspace().getSimpleExtentDims(size.data());
  if (dim > 0) {
    // Zero-dimensional (scalar) datasets cannot be chunked
    auto chunksize = choose_chunksize(size, datatype().getSize());
    proplist.setChunk(dim, chunksize.data());
    proplist.setShuffle(); // Shuffling improves compression
    const int level = 1;   // Level 1 is fast, but still offers good compression
    proplist.setDeflate(level);
    proplist.setFletcher32();
  }
  assert(m_have_location);
  m_dataset = m_location_group.createDataSet(m_location_name, datatype(),
                                             dataspace(), proplist);
  m_have_dataset = true;
}

void DataSet::construct_spaces(const box_t &membox, H5::DataSpace &memspace,
                               H5::DataSpace &filespace) const {
  assert(membox <= box());
  create_dataset();
  filespace = m_dataspace;
  // int ndims = filespace.getSimpleExtentNdims();
  // assert(ndims == rank());
  // vector<hsize_t> dims(rank());
  // filespace.getSimpleExtentDims(dims.data());
  // reverse(dims);
  // assert(all(point_t(dims) == shape()));
  if (rank() == 0) {
    // Cannot yet handle empty scalar box
    assert(!membox.empty());
  } else {
    filespace.selectHyperslab(
        H5S_SELECT_SET, reversed(vector<hsize_t>(membox.shape())).data(),
        reversed(vector<hsize_t>(membox.lower() - box().lower())).data());
  }
  memspace =
      H5::DataSpace(rank(), reversed(vector<hsize_t>(membox.shape())).data());
}

// CopyObj

shared_ptr<CopyObj> CopyObj::read(const H5::Group &group, const string &entry,
                                  const box_t &box) {
  auto lapl = H5::take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto exists = H5Lexists(group.getLocId(), entry.c_str(), lapl);
  assert(exists >= 0);
  if (exists) {
// entry is a link
#if 0
    // Check whether it is an external link
    bool have_extlink;
    string filename, objname;
    H5::readExternalLink(group, entry, have_extlink, filename, objname);
    if (!have_extlink) {
      // H5O_info_t info;
      // herr_t herr =
      //     H5Oget_info_by_name(group.getLocId(), entry.c_str(), &info, lapl);
      // assert(!herr);
      // assert(info.type == H5O_TYPE_DATASET);
      return make_shared<CopyObj>(box, group, entry);
    }
#else
    return make_shared<CopyObj>(box, group, entry);
#endif
  }
  return nullptr;
}

ostream &CopyObj::output(ostream &os) const {
  return os << "CopyObj: "
            << "???"
            // << "hid:0x" << std::hex << location() << std::dec
            << ":" << quote(name());
}

void CopyObj::write(const H5::Group &group, const string &entry) const {
  auto ocpypl = H5::take_hid(H5Pcreate(H5P_OBJECT_COPY));
  assert(ocpypl.valid());
  herr_t herr = H5Pset_copy_object(ocpypl, H5O_COPY_WITHOUT_ATTR_FLAG);
  assert(!herr);
  auto lcpl = H5::take_hid(H5Pcreate(H5P_LINK_CREATE));
  assert(lcpl.valid());
  herr = H5Ocopy(this->group().getId(), name().c_str(), group.getId(),
                 entry.c_str(), ocpypl, lcpl);
  assert(!herr);
}

// ExtLink

shared_ptr<ExtLink> ExtLink::read(const H5::Group &group, const string &entry,
                                  const box_t &box) {
// Reading a dataset always produces a CopyObj
#if 0
  auto lapl = H5::take_hid(H5Pcreate(H5P_LINK_ACCESS));
  assert(lapl.valid());
  auto exists = H5Lexists(group.getLocId(), entry.c_str(), lapl);
  assert(exists >= 0);
  if (exists) {
    // entry is a link
    // Check whether it is an external link
    bool have_extlink;
    string filename, objname;
    H5::readExternalLink(group, entry, have_extlink, filename, objname);
    if (have_extlink) {
      return make_shared<ExtLink>(box, filename, objname);
    }
  }
#endif
  return nullptr;
}

ostream &ExtLink::output(ostream &os) const {
  return os << "ExtLink: " << quote(filename()) << ":" << quote(objname());
}

void ExtLink::write(const H5::Group &group, const string &entry) const {
  H5::createExternalLink(group, entry, filename(), objname());
}
}
