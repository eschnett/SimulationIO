#include "DataBlock.hpp"

#include <algorithm>
#include <cstring>

namespace SimulationIO {
using namespace std;

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
} // namespace

// DataBlock

bool DataBlock::invariant() const { return !m_box.empty(); }

const vector<DataBlock::reader_t> DataBlock::readers = {
    DataRange::read, DataSet::read,   DataBufferEntry::read,
    CopyObj::read,   ExtLink::read,
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
    ASDFData::read,  ASDFArray::read,
#endif
};
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
const vector<DataBlock::asdf_reader_t> DataBlock::asdf_readers = {
    DataRange::read_asdf, DataSet::read_asdf, DataBufferEntry::read_asdf,
    CopyObj::read_asdf,   ExtLink::read_asdf, ASDFData::read_asdf,
    ASDFArray::read_asdf,
};
#endif

shared_ptr<DataBlock> DataBlock::read(const H5::Group &group,
                                      const string &entry, const box_t &box) {
  for (const auto &reader : readers) {
    auto datablock = reader(group, entry, box);
    if (datablock)
      return datablock;
  }
  return nullptr;
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataBlock> DataBlock::read_asdf(const ASDF::reader_state &rs,
                                           const YAML::Node &node,
                                           const box_t &box) {
  for (const auto &asdf_reader : asdf_readers) {
    auto datablock = asdf_reader(rs, node, box);
    if (datablock)
      return datablock;
  }
  return nullptr;
}
#endif

void DataBlock::construct_spaces(const box_t &memshape, const box_t &membox,
                                 const H5::DataSpace &dataspace,
                                 H5::DataSpace &memspace,
                                 H5::DataSpace &filespace) const {
  if (rank() == 0)
    assert(!membox.empty()); // HDF5 cannot handle an empty scalar box
  assert(membox <= memshape);
  assert(membox <= box());
  // TODO: This might not work for scalar spaces
  int ndims = dataspace.getSimpleExtentNdims();
  assert(ndims == rank());
  vector<hsize_t> dims(rank());
  dataspace.getSimpleExtentDims(dims.data());
  reverse(dims);
  assert(all(point_t(dims) == shape()));
  memspace =
      H5::DataSpace(rank(), reversed(vector<hsize_t>(memshape.shape())).data());
  if (rank() > 0)
    memspace.selectHyperslab(
        H5S_SELECT_SET, reversed(vector<hsize_t>(membox.shape())).data(),
        reversed(vector<hsize_t>(membox.lower() - box().lower())).data());
  filespace.copy(dataspace);
  if (rank() > 0)
    filespace.selectHyperslab(
        H5S_SELECT_SET, reversed(vector<hsize_t>(membox.shape())).data(),
        reversed(vector<hsize_t>(membox.lower() - box().lower())).data());
}

// DataRange

bool DataRange::invariant() const {
  return DataBlock::invariant() && int(m_delta.size()) == rank();
}

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

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataRange> DataRange::read_asdf(const ASDF::reader_state &rs,
                                           const YAML::Node &node,
                                           const box_t &box) {
  return nullptr;
}
#endif

ostream &DataRange::output(ostream &os) const {
  using namespace Output;
  return os << "DataRange: origin=" << origin() << " delta=" << delta();
}

void DataRange::write(const H5::Group &group, const string &entry) const {
  H5::createAttribute(group, entry + "_origin", origin());
  auto tmp = reversed(delta());
  H5::createAttribute(group, entry + "_delta", tmp);
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DataRange::write(ASDF::writer &w, const string &entry) const { assert(0); }
#endif

// DataSet

bool DataSet::invariant() const {
  bool inv = DataBlock::invariant();
  int ndims = m_dataspace.getSimpleExtentNdims();
  inv &= ndims == rank();
  vector<hsize_t> dims(rank());
  m_dataspace.getSimpleExtentDims(dims.data());
  reverse(dims);
  inv &= all(point_t(dims) == shape());
  return inv;
}

shared_ptr<DataSet> DataSet::read(const H5::Group &group, const string &entry,
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

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataSet> DataSet::read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box) {
  return nullptr;
}
#endif

ostream &DataSet::output(ostream &os) const {
  using namespace Output;
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
  if (m_have_attached_data) {
    writeData(m_attached_data.data(), m_memtype, m_memshape, m_membox);
    m_attached_data.clear();
    m_have_attached_data = false;
  }
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DataSet::write(ASDF::writer &w, const string &entry) const { assert(0); }
#endif

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
    proplist.setFletcher32();
    proplist.setShuffle(); // Shuffling improves compression
    const int level = 1;   // Level 1 is fast, but still offers good compression
    proplist.setDeflate(level);
  }
  assert(m_have_location);
  m_dataset = m_location_group.createDataSet(m_location_name, datatype(),
                                             dataspace(), proplist);
  m_have_dataset = true;
}

void DataSet::writeData(const void *data, const H5::DataType &datatype,
                        const box_t &datashape, const box_t &databox) const {
  // create_dataset();
  H5::DataSpace memspace, filespace;
  construct_spaces(datashape, databox, m_dataspace, memspace, filespace);
  m_dataset.write(data, datatype, memspace, filespace);
}

void DataSet::attachData(const vector<char> &data, const H5::DataType &datatype,
                         const box_t &datashape, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);

  m_memtype = datatype;
  m_memshape = datashape;
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  assert(data.size() == count * typesize);
  m_attached_data = data;
  m_have_attached_data = true;
}

void DataSet::attachData(vector<char> &&data, const H5::DataType &datatype,
                         const box_t &datashape, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);

  m_memtype = datatype;
  m_memshape = datashape;
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  assert(data.size() == count * typesize);
  m_attached_data = std::move(data);
  m_have_attached_data = true;
}

void DataSet::attachData(const void *data, const H5::DataType &datatype,
                         const box_t &datashape, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);
  assert(data);

  m_memtype = datatype;
  m_memshape = datashape;
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  m_attached_data.resize(count * typesize);
  memcpy(m_attached_data.data(), data, m_attached_data.size());
  m_have_attached_data = true;
}

// DataBuffer

shared_ptr<DataBuffer::dbuffer_t>
DataBuffer::dbuffer_t::make(const H5::DataType &datatype) {
  typedef long long long_long;
  if (datatype == H5::getType(int{})) {
    return make_shared<buffer_t<int>>();
  } else if (datatype == H5::getType(long_long{})) {
    return make_shared<buffer_t<long long>>();
  } else if (datatype == H5::getType(float{})) {
    return make_shared<buffer_t<float>>();
  } else if (datatype == H5::getType(double{})) {
    return make_shared<buffer_t<double>>();
  } else {
    assert(0);
  }
}

DataBuffer::DataBuffer(int dim, const H5::DataType &datatype)
    : m_datatype(datatype), m_concatenation(dconcatenation_t::make(dim)),
      m_buffer(dbuffer_t::make(datatype)) {}

void DataBuffer::write(const H5::Group &group, const string &entry) const {
  assert(false);
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DataBuffer::write(ASDF::writer &w, const string &entry) const {
  assert(0);
}
#endif

// DataBufferEntry

bool DataBufferEntry::invariant() const {
  assert(false);
  return true;
}

DataBufferEntry::DataBufferEntry(const box_t &box, const H5::DataType &datatype,
                                 const shared_ptr<DataBuffer> &databuffer)
    : DataBlock(box), m_databuffer(databuffer) {
  assert(datatype == databuffer->datatype());
  m_linearization = databuffer->concatenation()->push_back(box);
}

shared_ptr<DataBufferEntry> DataBufferEntry::read(const H5::Group &group,
                                                  const string &entry,
                                                  const box_t &box) {
  return nullptr;
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataBufferEntry>
DataBufferEntry::read_asdf(const ASDF::reader_state &rs, const YAML::Node &node,
                           const box_t &box) {
  return nullptr;
}
#endif

ostream &DataBufferEntry::output(ostream &os) const {
  using namespace Output;
  auto cls = datatype().getClass();
  auto clsname = H5::className(cls);
  auto typesize = datatype().getSize();
  // assert(dataspace().isSimple());
  // int dim = dataspace().getSimpleExtentNdims();
  // vector<hsize_t> size(dim);
  // dataspace().getSimpleExtentDims(size.data());
  // reverse(size);
  os << "DataBufferEntry: type=" << clsname << "(" << (8 * typesize)
     << " bit) box=" << m_linearization->box()
     << " pos=" << m_linearization->pos();
  return os;
}

void DataBufferEntry::write(const H5::Group &group, const string &entry) const {
  m_databuffer->write(group, entry);
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DataBufferEntry::write(ASDF::writer &w, const string &entry) const {
  assert(0);
}
#endif

// CopyObj

bool CopyObj::invariant() const {
  return DataBlock::invariant()
         // && m_group.valid()
         && !m_name.empty();
  // TODO: check rank
}

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

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<CopyObj> CopyObj::read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box) {
  return nullptr;
}
#endif

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

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void CopyObj::write(ASDF::writer &w, const string &entry) const { assert(0); }
#endif

void CopyObj::readData(void *data, const H5::DataType &datatype,
                       const box_t &datashape, const box_t &databox) const {
  if (rank() == 0)
    assert(!databox.empty()); // HDF5 cannot handle an empty scalar box
  assert(databox <= datashape);
  assert(databox <= box());
  auto dataset = group().openDataSet(name());
  auto dataspace = dataset.getSpace();
  int ndims = dataspace.getSimpleExtentNdims();
  assert(ndims == rank());
  vector<hsize_t> dims(rank());
  dataspace.getSimpleExtentDims(dims.data());
  reverse(dims);
  assert(all(point_t(dims) == shape()));
  H5::DataSpace memspace, filespace;
  construct_spaces(datashape, databox, dataspace, memspace, filespace);
  dataset.read(data, datatype, memspace, filespace);
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

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<ExtLink> ExtLink::read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box) {
  return nullptr;
}
#endif

ostream &ExtLink::output(ostream &os) const {
  return os << "ExtLink: " << quote(filename()) << ":" << quote(objname());
}

void ExtLink::write(const H5::Group &group, const string &entry) const {
  H5::createExternalLink(group, entry, filename(), objname());
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void ExtLink::write(ASDF::writer &w, const string &entry) const { assert(0); }
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX

// ASDFData

shared_ptr<ASDFData> ASDFData::read(const H5::Group &group, const string &entry,
                                    const box_t &box) {
  return nullptr;
}

shared_ptr<ASDFData> ASDFData::read_asdf(const ASDF::reader_state &rs,
                                         const YAML::Node &node,
                                         const box_t &box) {
  return nullptr;
}

ostream &ASDFData::output(ostream &os) const {
  return os << "ASDFData: ptr=" << m_blob->ptr()
            << " nbytes=" << m_blob->nbytes()
            << " datatype=" << ASDF::yaml_encode(*m_datatype);
}

void ASDFData::write(const H5::Group &group, const string &entry) const {
  assert(0);
}

void ASDFData::write(ASDF::writer &w, const string &entry) const {
  int64_t type_size = m_datatype->type_size();
  int dim = rank();
  vector<int64_t> strides(dim);
  int64_t stride = type_size;
  // SimulationIO uses Fortran array index ordering
  for (size_t d = 0; d < dim; ++d) {
    strides.at(d) = stride;
    stride *= shape()[d];
  }
  assert(stride == m_blob->nbytes());
  w << YAML::Key << entry << YAML::Value
    << ASDF::ndarray(m_blob, ASDF::block_format_t::block,
                     ASDF::compression_t::zlib, {}, m_datatype,
                     ASDF::host_byteorder(), vector<int64_t>(shape()), 0,
                     move(strides));
}

// ASDFArray

shared_ptr<ASDFArray> ASDFArray::read(const H5::Group &group,
                                      const string &entry, const box_t &box) {
  return nullptr;
}

shared_ptr<ASDFArray> ASDFArray::read_asdf(const ASDF::reader_state &rs,
                                           const YAML::Node &node,
                                           const box_t &box) {
  if (node.Tag() == "tag:stsci.edu:asdf/core/ndarray-1.0.0")
    return make_shared<ASDFArray>(box, make_shared<ASDF::ndarray>(rs, node));
  return nullptr;
}

ostream &ASDFArray::output(ostream &os) const {
  // TODO: output more detail
  return os << "ASDFArray";
}

void ASDFArray::write(const H5::Group &group, const string &entry) const {
  assert(0);
}

void ASDFArray::write(ASDF::writer &w, const string &entry) const {
  w << YAML::Key << entry << YAML::Value << *m_ndarray;
}

#endif

} // namespace SimulationIO
