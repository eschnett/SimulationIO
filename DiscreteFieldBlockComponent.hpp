#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#define DISCRETEFIELDBLOCKCOMPONENT_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "DataBlock.hpp"
#include "DiscreteFieldBlock.hpp"
#include "TensorComponent.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_SILO
#include <silo.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
#endif

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace SimulationIO {

using std::dynamic_pointer_cast;
using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlockComponent
    : public Common,
      public std::enable_shared_from_this<DiscreteFieldBlockComponent> {
  // Tensor component for a discrete field on a particular region
  weak_ptr<DiscreteFieldBlock> m_discretefieldblock; // parent
  shared_ptr<TensorComponent> m_tensorcomponent;     // without backlink
  shared_ptr<DataBlock> m_datablock;

  static string dataname() { return "data"; }

public:
  virtual string type() const { return "DiscreteFieldBlockComponent"; }

  shared_ptr<DiscreteFieldBlock> discretefieldblock() const {
    return m_discretefieldblock.lock();
  }
  shared_ptr<TensorComponent> tensorcomponent() const {
    return m_tensorcomponent;
  }
  shared_ptr<DataBlock> datablock() const { return m_datablock; }
  shared_ptr<DataRange> datarange() const {
    return dynamic_pointer_cast<DataRange>(m_datablock);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<DataSet> dataset() const {
    return dynamic_pointer_cast<DataSet>(m_datablock);
  }
  shared_ptr<CopyObj> copyobj() const {
    return dynamic_pointer_cast<CopyObj>(m_datablock);
  }
  shared_ptr<ExtLink> extlink() const {
    return dynamic_pointer_cast<ExtLink>(m_datablock);
  }
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<ASDFData> asdfdata() const {
    return dynamic_pointer_cast<ASDFData>(m_datablock);
  }
  shared_ptr<ASDFRef> asdfref() const {
    return dynamic_pointer_cast<ASDFRef>(m_datablock);
  }
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  shared_ptr<SiloVar> silovar() const {
    return dynamic_pointer_cast<SiloVar>(m_datablock);
  }
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  shared_ptr<TileDBData> tiledbdata() const {
    return dynamic_pointer_cast<TileDBData>(m_datablock);
  }
#endif

  virtual bool invariant() const;

  DiscreteFieldBlockComponent() = delete;
  DiscreteFieldBlockComponent(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent(DiscreteFieldBlockComponent &&) = delete;
  DiscreteFieldBlockComponent &
  operator=(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent &
  operator=(DiscreteFieldBlockComponent &&) = delete;

  friend class DiscreteFieldBlock;
  DiscreteFieldBlockComponent(
      hidden, const string &name,
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      const shared_ptr<TensorComponent> &tensorcomponent)
      : Common(name), m_discretefieldblock(discretefieldblock),
        m_tensorcomponent(tensorcomponent) {}
  DiscreteFieldBlockComponent(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const string &name,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
         const shared_ptr<TensorComponent> &tensorcomponent) {
    auto discretefieldblockcomponent = make_shared<DiscreteFieldBlockComponent>(
        hidden(), name, discretefieldblock, tensorcomponent);
    tensorcomponent->noinsert(discretefieldblockcomponent);
    return discretefieldblockcomponent;
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
    auto discretefieldblockcomponent =
        make_shared<DiscreteFieldBlockComponent>(hidden());
    discretefieldblockcomponent->read(loc, entry, discretefieldblock);
    return discretefieldblockcomponent;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<DiscreteFieldBlock> &discretefieldblock);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
    auto discretefieldblockcomponent =
        make_shared<DiscreteFieldBlockComponent>(hidden());
    discretefieldblockcomponent->read(rs, node, discretefieldblock);
    return discretefieldblockcomponent;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<DiscreteFieldBlock> &discretefieldblock);
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const Silo<DBfile> &file, const string &loc,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
    auto discretefieldblockcomponent =
        make_shared<DiscreteFieldBlockComponent>(hidden());
    discretefieldblockcomponent->read(file, loc, discretefieldblock);
    return discretefieldblockcomponent;
  }
  void read(const Silo<DBfile> &file, const string &loc,
            const shared_ptr<DiscreteFieldBlock> &discretefieldblock);
#endif

public:
  virtual ~DiscreteFieldBlockComponent() {}

  void merge(const shared_ptr<DiscreteFieldBlockComponent>
                 &discretefieldblockcomponent);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &
  operator<<(ostream &os,
             const DiscreteFieldBlockComponent &discretefieldblockcomponent) {
    return discretefieldblockcomponent.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &
  operator<<(ASDF::writer &w,
             const DiscreteFieldBlockComponent &discretefieldblockcomponent) {
    return discretefieldblockcomponent.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  virtual string silo_path() const;
  virtual void write(const Silo<DBfile> &file, const string &loc) const;
  string silo_varname() const;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif

  void unsetDataBlock();
  shared_ptr<DataRange> createDataRange(const WriteOptions &write_options,
                                        double origin,
                                        const vector<double> &delta);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<DataSet> createDataSet(const WriteOptions &write_options);
  shared_ptr<DataSet> createDataSet(const WriteOptions &write_options,
                                    const H5::DataType &type);
  template <typename T>
  shared_ptr<DataSet> createDataSet(const WriteOptions &write_options) {
    return createDataSet(write_options, H5::getType(T{}));
  }
  shared_ptr<DataBufferEntry>
  createDataBufferEntry(const WriteOptions &write_options,
                        const H5::DataType &type,
                        const shared_ptr<DataBuffer> &databuffer);
  shared_ptr<CopyObj> createCopyObj(const WriteOptions &write_options,
                                    const H5::Group &group, const string &name);
  shared_ptr<CopyObj> createCopyObj(const WriteOptions &write_options,
                                    const H5::H5File &file, const string &name);
  shared_ptr<ExtLink> createExtLink(const WriteOptions &write_options,
                                    const string &filename,
                                    const string &objname);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<ASDFData> createASDFData(const WriteOptions &write_options,
                                      const shared_ptr<ASDF::ndarray> &ndarray);
  shared_ptr<ASDFData>
  createASDFData(const WriteOptions &write_options,
                 const ASDF::memoized<ASDF::block_t> &mdata,
                 const shared_ptr<ASDF::datatype_t> &datatype);
  template <typename T>
  shared_ptr<ASDFData>
  createASDFData(const WriteOptions &write_options,
                 const ASDF::memoized<ASDF::block_t> &mdata) {
    return createASDFData(
        write_options, mdata,
        make_shared<ASDF::datatype_t>(ASDF::get_scalar_type_id<T>::value));
  }
  shared_ptr<ASDFData>
  createASDFData(const WriteOptions &write_options, const void *data,
                 size_t npoints, const box_t &memlayout,
                 const shared_ptr<ASDF::datatype_t> &datatype);
  template <typename T>
  shared_ptr<ASDFData> createASDFData(const WriteOptions &write_options,
                                      const vector<T> &data,
                                      const box_t &memlayout) {
    return createASDFData(
        write_options, data.data(), data.size(), memlayout,
        make_shared<ASDF::datatype_t>(ASDF::get_scalar_type_id<T>::value));
  }

  shared_ptr<ASDFRef> createASDFRef(const WriteOptions &write_options,
                                    const string &filename,
                                    const vector<string> &path);
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  shared_ptr<SiloVar> createSiloVar(const WriteOptions &write_options);
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  shared_ptr<TileDBData> createTileDBData(const WriteOptions &write_options);
#endif

  string getPath() const;
  string getName() const;
};

} // namespace SimulationIO

#define DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
