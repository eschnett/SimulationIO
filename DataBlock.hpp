#ifndef DATABLOCK_HPP
#define DATABLOCK_HPP

#include "Buffer.hpp"
#include "Config.hpp"
#include "Helpers.hpp"
#include "RegionCalculus.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace SimulationIO {

using namespace RegionCalculus;

using std::function;
using std::make_shared;
using std::ostream;
using std::ptrdiff_t;
using std::shared_future;
using std::shared_ptr;
using std::string;
using std::vector;

// A norm
namespace detail {
template <typename T> struct norm_traits {
  typedef typename std::conditional<std::is_integral<T>::value, double, T>::type
      real_type;
  // neutral values and addition for monoid
  static T min() {
    return std::numeric_limits<T>::has_infinity
               ? std::numeric_limits<T>::infinity()
               : std::numeric_limits<T>::max();
  }
  static T max() {
    return std::numeric_limits<T>::has_infinity
               ? -std::numeric_limits<T>::infinity()
               : std::numeric_limits<T>::lowest();
  }
  static T min(T x, T y) { return std::min(x, y); }
  static T max(T x, T y) { return std::max(x, y); }
};
template <typename T> struct norm_traits<std::complex<T>> {
  typedef typename std::complex<T>::value_type real_type;
  // neutral values and addition for monoid
  static std::complex<T> min() {
    return {norm_traits<T>::min(), norm_traits<T>::min()};
  }
  static std::complex<T> max() {
    return {norm_traits<T>::max(), norm_traits<T>::max()};
  }
  static std::complex<T> min(std::complex<T> x, std::complex<T> y) {
    return {norm_traits<T>::min(x.real(), y.real()),
            norm_traits<T>::min(x.imag(), y.imag())};
  }
  static std::complex<T> max(std::complex<T> x, std::complex<T> y) {
    return {norm_traits<T>::max(x.real(), y.real()),
            norm_traits<T>::max(x.imag(), y.imag())};
  }
};
} // namespace detail

template <typename T> class norm_t {
  // Ensure that abs and sqrt signatures are correct
  static_assert(std::is_same<decltype(std::abs(int())), int>::value, "");
  static_assert(std::is_same<decltype(std::abs(double())), double>::value, "");
  static_assert(
      std::is_same<decltype(std::abs(std::complex<double>())), double>::value,
      "");
  static_assert(std::is_same<decltype(std::sqrt(int())), double>::value, "");
  static_assert(std::is_same<decltype(std::sqrt(double())), double>::value, "");

  template <typename U> static U sqr(U x) { return x * x; }

public:
  typedef T value_type;
  typedef std::ptrdiff_t int_type;
  typedef typename detail::norm_traits<T>::real_type real_type;

private:
  int_type m_count, m_zeros;
  value_type m_min, m_max, m_sum;
  real_type m_sum_abs, m_sum_abs_squared;

public:
  norm_t(const norm_t &) = default;
  norm_t(norm_t &&) = default;
  norm_t &operator=(const norm_t &) = default;
  norm_t &operator=(norm_t &&) = default;

  norm_t()
      : m_count(0), m_zeros(0), m_min(detail::norm_traits<T>::min()),
        m_max(detail::norm_traits<T>::max()), m_sum(0), m_sum_abs(0),
        m_sum_abs_squared(0) {}
  norm_t(T x)
      : m_count(1), m_zeros(x == T(0)), m_min(x), m_max(x), m_sum(x),
        m_sum_abs(std::abs(x)), m_sum_abs_squared(sqr(std::abs(x))) {}
  norm_t &operator+=(const norm_t &n) {
    m_count += n.m_count;
    m_zeros += n.m_zeros;
    m_min = detail::norm_traits<T>::min(m_min, n.m_min);
    m_max = detail::norm_traits<T>::max(m_max, n.m_max);
    m_sum += n.m_sum;
    m_sum_abs += n.m_sum_abs;
    m_sum_abs_squared += n.m_sum_abs_squared;
    return *this;
  }
  norm_t operator+(const norm_t &n) const { return norm_t(*this) += n; }
  norm_t &operator+=(T x) { return *this += norm_t(x); }
  norm_t operator+(T x) const { return *this + norm_t(x); }

  norm_t(const std::vector<T> &xs) {
    *this = norm_t();
    for (auto x : xs)
      *this += x;
  }

  int_type count() const { return m_count; }
  int_type zeros() const { return m_zeros; }
  value_type sum() const { return m_sum; }
  value_type min() const { return m_min; }
  value_type max() const { return m_max; }
  // Avoid integer division by zero
  value_type avg() const {
    return m_count == 0 ? value_type(0) : m_sum / m_count;
  }
  real_type sdv() const {
    return std::sqrt(detail::norm_traits<real_type>::max(
        real_type(0), sqr(norm2()) - sqr(norm1())));
  }
  real_type sum_abs() const { return m_sum_abs; }
  real_type sum_abs_squared() const { return m_sum_abs_squared; }
  real_type norm1() const {
    return m_count == 0 ? real_type(0) : m_sum_abs / m_count;
  }
  real_type norm2() const {
    return m_count == 0 ? real_type(0) : std::sqrt(m_sum_abs_squared / m_count);
  }
};

////////////////////////////////////////////////////////////////////////////////

// An abstract block of data
class DataBlock {
#ifdef SIMULATIONIO_HAVE_HDF5
  typedef function<shared_ptr<DataBlock>(const H5::Group &group,
                                         const string &entry, const box_t &box)>
      reader_t;
  static const vector<reader_t> readers;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  typedef function<shared_ptr<DataBlock>(
      const ASDF::reader_state &rs, const YAML::Node &node, const box_t &box)>
      asdf_reader_t;
  static const vector<asdf_reader_t> asdf_readers;
#endif

  box_t m_box;

public:
  box_t box() const { return m_box; }
  int rank() const { return m_box.rank(); }
  point_t shape() const { return m_box.shape(); }
  typename point_t::prod_t size() const { return m_box.size(); }

#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<DataBlock> read(const H5::Group &group, const string &entry,
                                    const box_t &box);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DataBlock> read_asdf(const ASDF::reader_state &rs,
                                         const YAML::Node &node,
                                         const box_t &box);
#endif

  virtual bool invariant() const;

protected:
  DataBlock(const box_t &box) : m_box(box) {}

#ifdef SIMULATIONIO_HAVE_HDF5
  void construct_spaces(const box_t &memlayout, // allocated memory
                        const box_t &membox,    // memory to be transferred
                        const H5::DataSpace &dataspace, // file space
                        H5::DataSpace &memspace,
                        H5::DataSpace &filespace) const;
#endif

public:
  virtual ~DataBlock() {}

  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const DataBlock &datablock) {
    return datablock.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::Group &group, const string &entry) const = 0;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const = 0;
#endif
};

// A multi-linear range
class DataRange : public DataBlock {
  double m_origin;
  vector<double> m_delta;

public:
  double origin() const { return m_origin; }
  vector<double> delta() const { return m_delta; }

  DataRange(const box_t &box, double origin, const vector<double> &delta)
      : DataBlock(box), m_origin(origin), m_delta(delta) {
    assert(invariant());
  }

  virtual bool invariant() const;

  virtual ~DataRange() {}

#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<DataRange> read(const H5::Group &group, const string &entry,
                                    const box_t &box);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DataRange> read_asdf(const ASDF::reader_state &rs,
                                         const YAML::Node &node,
                                         const box_t &box);
#endif
  virtual ostream &output(ostream &os) const;
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::Group &group, const string &entry) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif
};

#ifdef SIMULATIONIO_HAVE_HDF5

// An HDF5 dataset
class DataSet : public DataBlock {
  H5::DataSpace m_dataspace;
  H5::DataType m_datatype;
  mutable bool m_have_location;
  mutable H5::Group m_location_group;
  mutable string m_location_name;
  mutable bool m_have_dataset;
  mutable H5::DataSet m_dataset;

  mutable bool m_have_attached_data;
  mutable vector<char> m_attached_data;
  mutable H5::DataType m_memtype;
  mutable box_t m_memlayout; // allocated memory
  mutable box_t m_membox;    // memory to be transferred

public:
  H5::DataSpace dataspace() const { return m_dataspace; }
  H5::DataType datatype() const { return m_datatype; }
  bool have_dataset() const { return m_have_dataset; }
  H5::DataSet dataset() const { return m_dataset; }

  virtual bool invariant() const;

  DataSet(const box_t &box, const H5::DataType &datatype)
      : DataBlock(box), m_dataspace(H5::DataSpace(
                            rank(), reversed(vector<hsize_t>(shape())).data())),
        m_datatype(datatype), m_have_location(false), m_have_dataset(false),
        m_have_attached_data(false) {
    assert(invariant());
  }
  template <typename T>
  DataSet(T, const box_t &box)
      : DataBlock(box), m_dataspace(H5::DataSpace(
                            rank(), reversed(vector<hsize_t>(shape())).data())),
        m_datatype(H5::getType(T{})), m_have_location(false),
        m_have_dataset(false), m_have_attached_data(false) {
    assert(invariant());
  }

  virtual ~DataSet() {}

  static shared_ptr<DataSet> read(const H5::Group &group, const string &entry,
                                  const box_t &box);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DataSet> read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box);
#endif
  virtual ostream &output(ostream &os) const;
  virtual void write(const H5::Group &group, const string &entry) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif

private:
  void create_dataset() const;

public:
  void writeData(const void *data, const H5::DataType &datatype,
                 const box_t &datashape, const box_t &databox) const;
  template <typename T>
  void writeData(const T *data, const box_t &datashape,
                 const box_t &databox) const {
    writeData(data, H5::getType(T{}), datashape, databox);
  }
  template <typename T>
  void writeData(const T *data, const box_t &databox) const {
    writeData(data, databox, databox);
  }
  template <typename T>
  void writeData(const vector<T> &data, const box_t &datashape,
                 const box_t &databox) const {
    assert(ptrdiff_t(data.size()) == datashape.size());
    writeData(data.data(), datashape, databox);
  }
  template <typename T>
  void writeData(const vector<T> &data, const box_t &databox) const {
    writeData(data.data(), databox, databox);
  }
  template <typename T> void writeData(const vector<T> &data) const {
    writeData(data, box());

    // TODO: Add function to add / update minimum and maximum; call it after
    // copying
    // TODO: Only update the attributes, do not set them (cache them!)
    // auto minmaxit = std::minmax_element(data.begin(), data.end());
    // H5::createAttribute(m_dataset, "minimum", *minmaxit.first);
    // H5::createAttribute(m_dataset, "maximum", *minmaxit.second);
    norm_t<T> norm(data);
    H5::createAttribute(m_dataset, "num_zeros", norm.zeros());
    H5::createAttribute(m_dataset, "sum", norm.sum());
    H5::createAttribute(m_dataset, "minimum", norm.min());
    H5::createAttribute(m_dataset, "maximum", norm.max());
    H5::createAttribute(m_dataset, "sum_abs", norm.sum_abs());
    H5::createAttribute(m_dataset, "sum_abs_squared", norm.sum_abs_squared());
  }

  void attachData(const vector<char> &data, const H5::DataType &datatype,
                  const box_t &datashape, const box_t &databox) const;
  void attachData(vector<char> &&data, const H5::DataType &datatype,
                  const box_t &datashape, const box_t &databox) const;
  void attachData(const void *data, const H5::DataType &datatype,
                  const box_t &datashape, const box_t &databox) const;
  template <typename T>
  void attachData(const T *data, const box_t &datashape,
                  const box_t &databox) const {
    attachData(data, H5::getType(T{}), datashape, databox);
  }
  template <typename T>
  void attachData(const T *data, const box_t &databox) const {
    attachData(data, databox, databox);
  }
  template <typename T>
  void attachData(const vector<T> &data, const box_t &datashape,
                  const box_t &databox) const {
    assert(ptrdiff_t(data.size()) == datashape.size());
    attachData(data.data(), datashape, databox);
  }
  template <typename T>
  void attachData(const vector<T> &data, const box_t &databox) const {
    attachData(data.data(), databox, databox);
  }
};

// An HDF5 dataset holding multiple concatenated blocks
class DataBuffer {
  H5::DataType m_datatype;
  shared_ptr<dconcatenation_t> m_concatenation;
  H5::DataSpace m_dataspace;
  H5::DataSet m_dataset;

  struct dbuffer_t {
    virtual ~dbuffer_t() {}
    static shared_ptr<dbuffer_t> make(const H5::DataType &datatype);
  };
  template <typename T> struct buffer_t : dbuffer_t { vector<T> vec; };
  shared_ptr<dbuffer_t> m_buffer;

public:
  H5::DataType datatype() const { return m_datatype; }
  shared_ptr<dconcatenation_t> concatenation() const { return m_concatenation; }

  DataBuffer() = delete;
  DataBuffer(int dim, const H5::DataType &datatype);

  void write(const H5::Group &group, const string &entry) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif
};

// A pointer into a DataBuffer
class DataBufferEntry : public DataBlock {
  shared_ptr<DataBuffer> m_databuffer;
  unique_ptr<dlinearization_t> m_linearization;

public:
  H5::DataType datatype() const { return m_databuffer->datatype(); }

  virtual bool invariant() const;

  virtual ~DataBufferEntry() {}

  DataBufferEntry(const box_t &box, const H5::DataType &datatype,
                  const shared_ptr<DataBuffer> &databuffer);

  static shared_ptr<DataBufferEntry>
  read(const H5::Group &group, const string &entry, const box_t &box);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DataBufferEntry> read_asdf(const ASDF::reader_state &rs,
                                               const YAML::Node &node,
                                               const box_t &box);
#endif
  virtual ostream &output(ostream &os) const;
  virtual void write(const H5::Group &group, const string &entry) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif
};

// A copy of an existing HDF5 dataset
class CopyObj : public DataBlock {
  H5::Group m_group;
  string m_name;

public:
  H5::Group group() const { return m_group; }
  string name() const { return m_name; }

  virtual bool invariant() const;

  CopyObj(const box_t &box, const H5::Group &group, const string &name)
      : DataBlock(box), m_group(group), m_name(name) {}
  CopyObj(const box_t &box, const H5::H5File &file, const string &name)
      : DataBlock(box), m_group(file.openGroup("/")), m_name(name) {}

  virtual ~CopyObj() {}

  static shared_ptr<CopyObj> read(const H5::Group &group, const string &entry,
                                  const box_t &box);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<CopyObj> read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box);
#endif
  virtual ostream &output(ostream &os) const;
  virtual void write(const H5::Group &group, const string &entry) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif

  void readData(void *data, const H5::DataType &datatype,
                const box_t &datashape, const box_t &databox) const;
  template <typename T>
  void readData(T *data, const box_t &datashape, const box_t &databox) const {
    readData(data, H5::getType(T{}), datashape, databox);
  }
  template <typename T> vector<T> readData(const box_t &databox) const {
    vector<T> data(databox.size());
    readData(data.data(), databox, databox);
    return data;
  }
  template <typename T> vector<T> readData() const {
    return readData<T>(box());
  }
};

// An external link to an HDF5 dataset
class ExtLink : public DataBlock {
  string m_filename;
  string m_objname;

public:
  string filename() const { return m_filename; }
  string objname() const { return m_objname; }

  virtual bool invariant() const {
    return DataBlock::invariant() && !m_filename.empty() && !m_objname.empty();
  }

  ExtLink(const box_t &box, const string &filename, const string &objname)
      : DataBlock(box), m_filename(filename), m_objname(objname) {}

  virtual ~ExtLink() {}

  static shared_ptr<ExtLink> read(const H5::Group &group, const string &entry,
                                  const box_t &box);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<ExtLink> read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box);
#endif
  virtual ostream &output(ostream &os) const;
  virtual void write(const H5::Group &group, const string &entry) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual void write(ASDF::writer &w, const string &entry) const;
#endif

  // TODO: implement readData
};

#endif // #ifdef SIMULATIONIO_HAVE_HDF5

#ifdef SIMULATIONIO_HAVE_ASDF_CXX

// A pointer to data for ASDF
class ASDFData : public DataBlock {
  shared_future<shared_ptr<ASDF::generic_blob_t>> m_fblob;
  shared_ptr<ASDF::datatype_t> m_datatype;

public:
  virtual bool invariant() const {
    return DataBlock::invariant() && m_fblob.valid() && bool(m_datatype);
  }

  ASDFData(const box_t &box,
           const shared_future<shared_ptr<ASDF::generic_blob_t>> &fblob,
           const shared_ptr<ASDF::datatype_t> &datatype);

  ASDFData(const box_t &box, const shared_ptr<ASDF::generic_blob_t> &blob,
           const shared_ptr<ASDF::datatype_t> &datatype)
      : ASDFData(box, ASDF::make_future<shared_ptr<ASDF::generic_blob_t>>(blob),
                 datatype) {}
  template <typename T>
  ASDFData(const box_t &box, const shared_ptr<ASDF::blob_t<T>> &blob)
      : ASDFData(
            box, blob,
            make_shared<ASDF::datatype_t>(ASDF::get_scalar_type_id<T>::value)) {
  }
  ASDFData(const box_t &box, const void *data, size_t npoints,
           const box_t &memlayout,
           const shared_ptr<ASDF::datatype_t> &datatype);
  template <typename T>
  ASDFData(const box_t &box, const vector<T> &data, const box_t &memlayout)
      : ASDFData(
            box, data.data(), data.size(), memlayout,
            make_shared<ASDF::datatype_t>(ASDF::get_scalar_type_id<T>::value)) {
  }

  virtual ~ASDFData() {}

#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<ASDFData> read(const H5::Group &group, const string &entry,
                                   const box_t &box);
#endif
  static shared_ptr<ASDFData> read_asdf(const ASDF::reader_state &rs,
                                        const YAML::Node &node,
                                        const box_t &box);
  virtual ostream &output(ostream &os) const;
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::Group &group, const string &entry) const;
#endif
  virtual void write(ASDF::writer &w, const string &entry) const;
};

// An ASDF ndarray
class ASDFArray : public DataBlock {
  shared_ptr<ASDF::ndarray> m_ndarray;

public:
  shared_ptr<ASDF::ndarray> ndarray() const { return m_ndarray; }

  virtual bool invariant() const {
    return DataBlock::invariant() && bool(m_ndarray);
  }

  ASDFArray(const box_t &box, const shared_ptr<ASDF::ndarray> &arr)
      : DataBlock(box), m_ndarray(arr) {}

  virtual ~ASDFArray() {}

#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<ASDFArray> read(const H5::Group &group, const string &entry,
                                    const box_t &box);
#endif
  static shared_ptr<ASDFArray> read_asdf(const ASDF::reader_state &rs,
                                         const YAML::Node &node,
                                         const box_t &box);
  virtual ostream &output(ostream &os) const;
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::Group &group, const string &entry) const;
#endif
  virtual void write(ASDF::writer &w, const string &entry) const;
};

// An ASDF reference
class ASDFRef : public DataBlock {
  shared_ptr<ASDF::reference> m_reference;

public:
  virtual bool invariant() const {
    return DataBlock::invariant() && bool(m_reference);
  }

  ASDFRef(const box_t &box, const shared_ptr<ASDF::reference> &ref)
      : DataBlock(box), m_reference(ref) {}

  virtual ~ASDFRef() {}

#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<ASDFRef> read(const H5::Group &group, const string &entry,
                                  const box_t &box);
#endif
  static shared_ptr<ASDFRef> read_asdf(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const box_t &box);
  virtual ostream &output(ostream &os) const;
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::Group &group, const string &entry) const;
#endif
  virtual void write(ASDF::writer &w, const string &entry) const;
};

#endif // #ifdef SIMULATIONIO_HAVE_ASDF_CXX

} // namespace SimulationIO

#define DATABLOCK_HPP_DONE
#endif // #ifndef DATABLOCK_HPP
#ifndef DATABLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
