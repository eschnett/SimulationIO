#include "DataBlock.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <type_traits>

namespace SimulationIO {
using namespace std;

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

namespace HyperSlab {

template <int D, typename T> array<T, D> mkarray(const vector<T> &xs) {
  assert(xs.size() == D);
  array<T, D> rs;
  for (int d = 0; d < D; ++d)
    rs[d] = xs[d];
  return rs;
}

template <int D> array<ptrdiff_t, D> mkarray(const point_t &xs) {
  assert(xs.rank() == D);
  array<ptrdiff_t, D> rs;
  for (int d = 0; d < D; ++d)
    rs[d] = xs[d];
  return rs;
}

template <int D> class copy_t {
  array<ptrdiff_t, D> outstrides;
  array<ptrdiff_t, D> instrides;
  array<ptrdiff_t, D> shape;
  ptrdiff_t type_size;
  unsigned char *outptr_min;
  unsigned char *outptr_max;
  const unsigned char *inptr_min;
  const unsigned char *inptr_max;

  bool outcheck(unsigned char *const outptr, size_t nbytes) const {
    return outptr_min <= outptr && outptr + nbytes <= outptr_max;
  }
  bool incheck(const unsigned char *const inptr, size_t nbytes) const {
    return inptr_min <= inptr && inptr + nbytes <= inptr_max;
  }

  // This implementation is efficient for Fortran array order

  template <size_t N, int DD, typename enable_if<DD == 0>::type * = nullptr>
  void copy_nd_f(unsigned char *const outptr,
                 const unsigned char *const inptr) const {
    // assert(outcheck(outptr, N));
    // assert(incheck(inptr, N));
    memcpy(outptr, inptr, N);
  }

  template <size_t N, int DD,
            typename enable_if<(DD > 0 && DD <= D)>::type * = nullptr>
  void copy_nd_f(unsigned char *const outptr,
                 const unsigned char *const inptr) const {
    const ptrdiff_t outdi = outstrides[DD - 1];
    const ptrdiff_t indi = instrides[DD - 1];
    const ptrdiff_t ni = shape[DD - 1];
    for (ptrdiff_t i = 0; i < ni; ++i)
      copy_nd_f<N, DD - 1>(outptr + i * outdi, inptr + i * indi);
  }

  // This implementation is efficient for C array order

  template <size_t N, int DD, typename enable_if<DD == D>::type * = nullptr>
  void copy_nd_c(unsigned char *const outptr,
                 const unsigned char *const inptr) const {
    // assert(outcheck(outptr, N));
    // assert(incheck(inptr, N));
    memcpy(outptr, inptr, N);
  }

  template <size_t N, int DD,
            typename enable_if<(DD >= 0 && DD < D)>::type * = nullptr>
  void copy_nd_c(unsigned char *const outptr,
                 const unsigned char *const inptr) const {
    const ptrdiff_t outdi = outstrides[DD];
    const ptrdiff_t indi = instrides[DD];
    const ptrdiff_t ni = shape[DD];
    for (ptrdiff_t i = 0; i < ni; ++i)
      copy_nd_c<N, DD + 1>(outptr + i * outdi, inptr + i * indi);
  }

public:
  copy_t(unsigned char *const outptr, const ptrdiff_t outnbytes,
         const ptrdiff_t outoffset, const array<ptrdiff_t, D> &outstrides,
         const unsigned char *const inptr, const ptrdiff_t innbytes,
         const ptrdiff_t inoffset, const array<ptrdiff_t, D> &instrides,
         const array<ptrdiff_t, D> &shape, const ptrdiff_t type_size)
      : outstrides(outstrides), instrides(instrides), shape(shape),
        type_size(type_size), outptr_min(outptr),
        outptr_max(outptr + outnbytes), inptr_min(inptr),
        inptr_max(inptr + innbytes) {
    unsigned char *const outptr1 = outptr + outoffset;
    const unsigned char *const inptr1 = inptr + inoffset;
    if (D == 0 || outstrides[0] <= outstrides[D - 1]) {
      // prefer Fortran order
      switch (type_size) {
      case 1:
        copy_nd_f<1, D>(outptr1, inptr1);
        break;
      case 2:
        copy_nd_f<2, D>(outptr1, inptr1);
        break;
      case 4:
        copy_nd_f<4, D>(outptr1, inptr1);
        break;
      case 8:
        copy_nd_f<8, D>(outptr1, inptr1);
        break;
      case 16:
        copy_nd_f<16, D>(outptr1, inptr1);
        break;
      default:
        assert(0);
      }
    } else {
      // prefer C order
      switch (type_size) {
      case 1:
        copy_nd_c<1, 0>(outptr1, inptr1);
        break;
      case 2:
        copy_nd_c<2, 0>(outptr1, inptr1);
        break;
      case 4:
        copy_nd_c<4, 0>(outptr1, inptr1);
        break;
      case 8:
        copy_nd_c<8, 0>(outptr1, inptr1);
        break;
      case 16:
        copy_nd_c<16, 0>(outptr1, inptr1);
        break;
      default:
        assert(0);
      }
    }
  }
};

ptrdiff_t layout2offset(ptrdiff_t offset, const point_t &strides,
                        const box_t &virtual_layout, const box_t &box) {
  assert(box <= virtual_layout);
  const int rank = strides.rank();
  assert(virtual_layout.rank() == rank);
  assert(box.rank() == rank);
  for (int d = 0; d < rank; ++d)
    offset += (box.lower()[d] - virtual_layout.lower()[d]) * strides[d];
  return offset;
}

pair<ptrdiff_t, point_t> layout2strides(const box_t &layout, const box_t &box,
                                        const size_t type_size) {
  assert(box <= layout);
  const int rank = layout.rank();
  vector<ptrdiff_t> strides(rank);
  ptrdiff_t offset = 0, stride = type_size;
  for (int d = 0; d < rank; ++d) {
    strides[d] = stride;
    offset += (box.lower()[d] - layout.lower()[d]) * stride;
    stride *= layout.shape()[d];
  }
  assert(stride == layout.size() * type_size);
  // Test layout2offsets implementation
  auto other_off = layout2offset(0, point_t(strides), layout, box);
  assert(other_off == offset);
  return make_pair(offset, point_t(strides));
}

void copy(void *const outptr0, const ptrdiff_t outnbytes,
          const ptrdiff_t outoffset, const point_t &outstrides,
          const void *const inptr0, const ptrdiff_t innbytes,
          const ptrdiff_t inoffset, const point_t &instrides,
          const point_t &shape, const size_t type_size) {
  const int rank = shape.rank();
  assert(outstrides.rank() == rank);
  assert(instrides.rank() == rank);

  // Check all corners whether they are contained in the array
  for (size_t corner = 0; corner < (size_t(1) << rank); ++corner) {
    ptrdiff_t outpos = outoffset;
    for (int d = 0; d < rank; ++d) {
      size_t dbit = size_t(1) << d;
      outpos += outstrides[d] * (corner & dbit ? shape[d] - 1 : 0);
    }
    assert(outpos >= 0 && outpos + type_size <= outnbytes);
    ptrdiff_t inpos = inoffset;
    for (int d = 0; d < rank; ++d) {
      size_t dbit = size_t(1) << d;
      inpos += instrides[d] * (corner & dbit ? shape[d] - 1 : 0);
    }
    assert(inpos >= 0 && inpos + type_size <= innbytes);
  }

  const auto outptr = static_cast<unsigned char *>(outptr0);
  const auto inptr = static_cast<const unsigned char *>(inptr0);

  switch (rank) {
  case 0:
    copy_t<0>(outptr, outnbytes, outoffset, mkarray<0>(outstrides), inptr,
              innbytes, inoffset, mkarray<0>(instrides), mkarray<0>(shape),
              type_size);
    break;
  case 1:
    copy_t<1>(outptr, outnbytes, outoffset, mkarray<1>(outstrides), inptr,
              innbytes, inoffset, mkarray<1>(instrides), mkarray<1>(shape),
              type_size);
    break;
  case 2:
    copy_t<2>(outptr, outnbytes, outoffset, mkarray<2>(outstrides), inptr,
              innbytes, inoffset, mkarray<2>(instrides), mkarray<2>(shape),
              type_size);
    break;
  case 3:
    copy_t<3>(outptr, outnbytes, outoffset, mkarray<3>(outstrides), inptr,
              innbytes, inoffset, mkarray<3>(instrides), mkarray<3>(shape),
              type_size);
    break;
  case 4:
    copy_t<4>(outptr, outnbytes, outoffset, mkarray<4>(outstrides), inptr,
              innbytes, inoffset, mkarray<4>(instrides), mkarray<4>(shape),
              type_size);
    break;
  default:
    assert(0);
  }
}

void copy(void *const outptr0, const ptrdiff_t outnpoints,
          const box_t &outlayout, const box_t &outbox, const void *const inptr0,
          const ptrdiff_t innpoints, const box_t &inlayout, const box_t &inbox,
          const size_t type_size) {
  const int rank = outlayout.rank();
  assert(outbox.rank() == rank);
  assert(inlayout.rank() == rank);
  assert(inbox.rank() == rank);

  assert(outlayout.size() <= outnpoints);
  // assert(outbox <= outlayout);
  // const auto outshape = outbox.shape();
  // // const auto outstrides = outlayout.shape() * point_t(rank, type_size);
  // vector<ptrdiff_t> outstrides(rank);
  // ptrdiff_t outoffset = 0, outstride = type_size;
  // for (int d = 0; d < rank; ++d) {
  //   outstrides[d] = outstride;
  //   outoffset += (outbox.lower()[d] - outlayout.lower()[d]) * outstride;
  //   outstride *= outlayout.shape()[d];
  // }
  // assert(outstride == outlayout.size() * type_size);
  const auto out_off_str = layout2strides(outlayout, outbox, type_size);
  const auto &outoffset = out_off_str.first;
  const auto &outstrides = out_off_str.second;

  assert(inlayout.size() <= innpoints);
  // assert(inbox <= inlayout);
  // const auto inshape = inbox.shape();
  // // const auto instrides = inlayout.shape() * point_t(rank, type_size);
  // vector<ptrdiff_t> instrides(rank);
  // ptrdiff_t inoffset = 0, instride = type_size;
  // for (int d = 0; d < rank; ++d) {
  //   instrides[d] = instride;
  //   inoffset += (inbox.lower()[d] - inlayout.lower()[d]) * instride;
  //   instride *= inlayout.shape()[d];
  // }
  // assert(instride == inlayout.size() * type_size);
  const auto in_off_str = layout2strides(inlayout, inbox, type_size);
  const auto &inoffset = in_off_str.first;
  const auto &instrides = in_off_str.second;

  const auto shape = outbox.shape();
  assert(all(inbox.shape() == shape));

  copy(outptr0, type_size * outnpoints, outoffset, point_t(outstrides), inptr0,
       type_size * innpoints, inoffset, point_t(instrides), shape, type_size);
}

} // namespace HyperSlab

// DataBlock

bool DataBlock::invariant() const { return !m_box.empty(); }

#ifdef SIMULATIONIO_HAVE_HDF5
const vector<DataBlock::reader_t> DataBlock::readers = {
    DataRange::read, DataSet::read, DataBufferEntry::read,
    CopyObj::read,   ExtLink::read,
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
    ASDFData::read,  ASDFRef::read,
#endif
};
#endif // #ifdef SIMULATIONIO_HAVE_HDF5

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
const vector<DataBlock::asdf_reader_t> DataBlock::asdf_readers = {
    DataRange::read_asdf,
#ifdef SIMULATIONIO_HAVE_HDF5
    DataSet::read_asdf,   DataBufferEntry::read_asdf,
    CopyObj::read_asdf,   ExtLink::read_asdf,
#endif
    ASDFData::read_asdf,  ASDFRef::read_asdf,
};
#endif // #ifdef SIMULATIONIO_HAVE_ASDF_CXX

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<DataBlock> DataBlock::read(const H5::Group &group,
                                      const string &entry, const box_t &box) {
  for (const auto &reader : readers) {
    auto datablock = reader(group, entry, box);
    if (datablock)
      return datablock;
  }
  return nullptr;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataBlock>
DataBlock::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
                     const YAML::Node &node, const box_t &box) {
  for (const auto &asdf_reader : asdf_readers) {
    auto datablock = asdf_reader(rs, node, box);
    if (datablock)
      return datablock;
  }
  return nullptr;
}
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
void DataBlock::construct_spaces(const box_t &memlayout, const box_t &membox,
                                 const H5::DataSpace &dataspace,
                                 H5::DataSpace &memspace,
                                 H5::DataSpace &filespace) const {
  if (rank() == 0)
    assert(!membox.empty()); // HDF5 cannot handle an empty scalar box
  assert(membox <= memlayout);
  assert(membox <= box());
  // TODO: This might not work for scalar spaces
  int ndims = dataspace.getSimpleExtentNdims();
  assert(ndims == rank());
  vector<hsize_t> dims(rank());
  dataspace.getSimpleExtentDims(dims.data());
  reverse(dims);
  assert(all(point_t(dims) == shape()));
  memspace = H5::DataSpace(rank(),
                           reversed(vector<hsize_t>(memlayout.shape())).data());
  if (rank() > 0)
    memspace.selectHyperslab(
        H5S_SELECT_SET, reversed(vector<hsize_t>(membox.shape())).data(),
        // reversed(vector<hsize_t>(membox.lower() - box().lower())).data()
        reversed(vector<hsize_t>(membox.lower() - memlayout.lower())).data());
  filespace.copy(dataspace);
  if (rank() > 0)
    filespace.selectHyperslab(
        H5S_SELECT_SET, reversed(vector<hsize_t>(membox.shape())).data(),
        reversed(vector<hsize_t>(membox.lower() - box().lower())).data());
}
#endif

// DataRange

bool DataRange::invariant() const {
  return DataBlock::invariant() && int(m_delta.size()) == rank();
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DataRange>
DataRange::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
                     const YAML::Node &node, const box_t &box) {
  if (node.Tag() !=
      "tag:github.com/eschnett/SimulationIO/asdf-cxx/DataRange-1.0.0")
    return nullptr;
  auto origin = node["origin"].as<double>();
  auto delta = node["delta"].as<vector<double>>();
  return make_shared<DataRange>(box, origin, move(delta));
}
#endif

ostream &DataRange::output(ostream &os) const {
  using namespace Output;
  return os << "DataRange: origin=" << origin() << " delta=" << delta();
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DataRange::write(const H5::Group &group, const string &entry) const {
  H5::createAttribute(group, entry + "_origin", origin());
  auto tmp = reversed(delta());
  H5::createAttribute(group, entry + "_delta", tmp);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DataRange::write(ASDF::writer &w, const string &entry) const {
  w << YAML::Key << entry;
  w << YAML::LocalTag("sio", "DataRange-1.0.0") << YAML::BeginMap;
  // w << YAML::Key << "box" << YAML::Value << box();
  w << YAML::Key << "origin" << YAML::Value << m_origin;
  w << YAML::Key << "delta" << YAML::Value << m_delta;
  w << YAML::EndMap;
}
#endif

#ifdef SIMULATIONIO_HAVE_HDF5

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
shared_ptr<DataSet> DataSet::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
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
    writeData(m_attached_data.data(), m_memtype, m_memlayout, m_membox);
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
                        const box_t &datalayout, const box_t &databox) const {
  // create_dataset();
  H5::DataSpace memspace, filespace;
  construct_spaces(datalayout, databox, m_dataspace, memspace, filespace);
  m_dataset.write(data, datatype, memspace, filespace);
}

void DataSet::attachData(const vector<char> &data, const H5::DataType &datatype,
                         const box_t &datalayout, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);

  m_memtype = datatype;
  m_memlayout = datalayout;
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  assert(data.size() == count * typesize);
  m_attached_data = data;
  m_have_attached_data = true;
}

void DataSet::attachData(vector<char> &&data, const H5::DataType &datatype,
                         const box_t &datalayout, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);

  m_memtype = datatype;
  m_memlayout = datalayout;
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  assert(data.size() == count * typesize);
  m_attached_data = std::move(data);
  m_have_attached_data = true;
}

void DataSet::attachData(const void *data, const H5::DataType &datatype,
                         const box_t &datalayout, const box_t &databox) const {
  assert(not m_have_dataset);
  assert(not m_have_location);
  assert(not m_have_attached_data);
  assert(data);

  m_memtype = datatype;
  // m_memlayout = datalayout;
  m_memlayout = databox; // since we copy
  m_membox = databox;
  auto count = m_membox.size();
  auto typesize = m_memtype.getSize();

  assert(m_attached_data.empty());
  m_attached_data.resize(count * typesize);
  // memcpy(m_attached_data.data(), data, m_attached_data.size());
  HyperSlab::copy(m_attached_data.data(), count, m_memlayout, m_membox, data,
                  datalayout.size(), datalayout, databox, typesize);
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
DataBufferEntry::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
                           const YAML::Node &node, const box_t &box) {
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
shared_ptr<CopyObj> CopyObj::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
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

namespace {
ASDF::scalar_type_id_t asdf_type(const H5::DataType &h5type) {
  if (h5type == H5::getType(ASDF::bool8_t()))
    return ASDF::id_bool8;
  if (h5type == H5::getType(ASDF::int8_t()))
    return ASDF::id_int8;
  if (h5type == H5::getType(ASDF::int16_t()))
    return ASDF::id_int16;
  if (h5type == H5::getType(ASDF::int32_t()))
    return ASDF::id_int32;
  if (h5type == H5::getType(ASDF::int64_t()))
    return ASDF::id_int64;
  if (h5type == H5::getType(ASDF::uint8_t()))
    return ASDF::id_uint8;
  if (h5type == H5::getType(ASDF::uint16_t()))
    return ASDF::id_uint16;
  if (h5type == H5::getType(ASDF::uint32_t()))
    return ASDF::id_uint32;
  if (h5type == H5::getType(ASDF::uint64_t()))
    return ASDF::id_uint64;
  if (h5type == H5::getType(ASDF::float32_t()))
    return ASDF::id_float32;
  if (h5type == H5::getType(ASDF::float64_t()))
    return ASDF::id_float64;
  if (h5type == H5::getType(ASDF::complex64_t()))
    return ASDF::id_complex64;
  if (h5type == H5::getType(ASDF::complex128_t()))
    return ASDF::id_complex128;
  assert(0);
}
} // namespace

void CopyObj::write(ASDF::writer &w, const string &entry) const {
  auto dataset = group().openDataSet(name());
  auto type = dataset.getDataType();
  auto type_size = type.getSize();
  vector<unsigned char> data(size() * type_size);
  readData(data.data(), type, box(), box());
  auto arr = ASDFData(box(), move(data),
                      make_shared<ASDF::datatype_t>(asdf_type(type)));
  arr.write(w, entry);
}

#endif // #ifdef SIMULATIONIO_HAVE_ASDF_CXX

void CopyObj::readData(void *data, const H5::DataType &datatype,
                       const box_t &datalayout, const box_t &databox) const {
  if (rank() == 0)
    assert(!databox.empty()); // HDF5 cannot handle an empty scalar box
  assert(databox <= datalayout);
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
  construct_spaces(datalayout, databox, dataspace, memspace, filespace);
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
shared_ptr<ExtLink> ExtLink::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
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

#endif // #ifdef SIMULATIONIO_HAVE_HDF5

#ifdef SIMULATIONIO_HAVE_ASDF_CXX

// ASDFData

vector<int64_t> fortran_strides(const ASDF::datatype_t &datatype,
                                const vector<int64_t> &shape) {
  int64_t type_size = datatype.type_size();
  int dim = shape.size();
  vector<int64_t> strides(dim);
  int64_t stride = type_size;
  for (size_t d = 0; d < dim; ++d) {
    strides.at(d) = stride;
    stride *= shape.at(d);
  }
  return strides;
}

ASDFData::ASDFData(const box_t &box, const ASDF::memoized<ASDF::block_t> &mdata,
                   const shared_ptr<ASDF::datatype_t> &datatype)
    : DataBlock(box),
      m_ndarray(make_shared<ASDF::ndarray>(
          mdata, ASDF::block_format_t::block, ASDF::compression_t::zlib,
          vector<bool>(), datatype, ASDF::host_byteorder(),
          vector<int64_t>(shape()), 0,
          fortran_strides(*datatype, vector<int64_t>(shape())))) {}

ASDFData::ASDFData(const box_t &box, vector<unsigned char> data,
                   const shared_ptr<ASDF::datatype_t> &datatype)
    : DataBlock(box),
      m_ndarray(make_shared<ASDF::ndarray>(
          ASDF::make_constant_memoized(shared_ptr<ASDF::block_t>(
              make_shared<ASDF::typed_block_t<unsigned char>>(move(data)))),
          ASDF::block_format_t::block, ASDF::compression_t::zlib,
          vector<bool>(), datatype, ASDF::host_byteorder(),
          vector<int64_t>(shape()), 0,
          fortran_strides(*datatype, vector<int64_t>(shape())))) {}

ASDFData::ASDFData(const box_t &box, const void *data, size_t npoints,
                   const box_t &memlayout,
                   const shared_ptr<ASDF::datatype_t> &datatype)
    : DataBlock(box) {
  assert(data);
  assert(box <= memlayout);
  assert(npoints == memlayout.size());
  auto type_size = datatype->type_size();
  vector<unsigned char> buf(box.size() * type_size);
  HyperSlab::copy(buf.data(), buf.size(), box, box, data, type_size * npoints,
                  memlayout, box, type_size);
  m_ndarray = make_shared<ASDF::ndarray>(
      ASDF::make_constant_memoized(shared_ptr<ASDF::block_t>(
          make_shared<ASDF::typed_block_t<unsigned char>>(move(buf)))),
      ASDF::block_format_t::block, ASDF::compression_t::zlib, vector<bool>(),
      datatype, ASDF::host_byteorder(), vector<int64_t>(shape()), 0,
      fortran_strides(*datatype, vector<int64_t>(shape())));
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<ASDFData> ASDFData::read(const H5::Group &group, const string &entry,
                                    const box_t &box) {
  return nullptr;
}
#endif

shared_ptr<ASDFData>
ASDFData::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
                    const YAML::Node &node, const box_t &box) {
  if (node.Tag() == "tag:stsci.edu:asdf/core/ndarray-1.0.0")
    return make_shared<ASDFData>(box, make_shared<ASDF::ndarray>(rs, node));
  return nullptr;
}

ostream &ASDFData::output(ostream &os) const {
  return os << "ASDFData:"
            << " datatype=" << ASDF::yaml_encode(*m_ndarray->get_datatype())
            << " shape=" << ASDF::yaml_encode(m_ndarray->get_shape());
}

#ifdef SIMULATIONIO_HAVE_HDF5
void ASDFData::write(const H5::Group &group, const string &entry) const {
  assert(0);
}
#endif

void ASDFData::write(ASDF::writer &w, const string &entry) const {
  w << YAML::Key << entry << YAML::Value << *m_ndarray;
}

// ASDFRef

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<ASDFRef> ASDFRef::read(const H5::Group &group, const string &entry,
                                  const box_t &box) {
  return nullptr;
}
#endif

shared_ptr<ASDFRef> ASDFRef::read_asdf(const shared_ptr<ASDF::reader_state> &rs,
                                       const YAML::Node &node,
                                       const box_t &box) {
  if (node.IsMap() && node.size() == 1 && node["$ref"])
    return make_shared<ASDFRef>(box, make_shared<ASDF::reference>(rs, node));
  return nullptr;
}

ostream &ASDFRef::output(ostream &os) const {
  // TODO: output more detail
  return os << "ASDFRef";
}

#ifdef SIMULATIONIO_HAVE_HDF5
void ASDFRef::write(const H5::Group &group, const string &entry) const {
  assert(0);
}
#endif

void ASDFRef::write(ASDF::writer &w, const string &entry) const {
  w << YAML::Key << entry << YAML::Value << *m_reference;
}

#endif // #ifdef SIMULATIONIO_HAVE_ASDF_CXX

} // namespace SimulationIO
