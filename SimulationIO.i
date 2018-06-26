// SWIG interface file

%module SimulationIO

%{
  #include "Config.hpp"
%}
%include "Config.hpp"

%{
  #include "RegionCalculus.hpp"
  using namespace RegionCalculus;
%}
%include "RegionCalculus.i"

%{
  #include "SimulationIO.hpp"
  using namespace SimulationIO;
%}

#ifdef SIMULATIONIO_HAVE_HDF5
%{
  #include <H5Cpp.h>
%}
#endif

%include <std_map.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
template<typename T> class weak_ptr {
public:
  typedef T element_type;

  weak_ptr();
  weak_ptr(const weak_ptr&);
  weak_ptr(const std::shared_ptr<T>&);
  template<typename U> weak_ptr(const weak_ptr<U>&);
  template<typename U> weak_ptr(const std::shared_ptr<U>&);

  void swap(weak_ptr&);
  void reset();

  long use_count() const;
  bool expired() const;
  std::shared_ptr<T> lock() const;
};
}

// Note: SWIG's support for map and shared_ptr does not work with namespaces
// using std::map;
// using std::shared_ptr;
using std::string;

%shared_ptr(DataBlock);
%shared_ptr(DataRange);
#ifdef SIMULATIONIO_HAVE_HDF5
%shared_ptr(CopyObj);
%shared_ptr(DataSet);
%shared_ptr(ExtLink);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
%shared_ptr(ASDFData);
%shared_ptr(ASDFRef);
#endif

%shared_ptr(Basis);
%shared_ptr(BasisVector);
%shared_ptr(Configuration);
%shared_ptr(CoordinateField);
%shared_ptr(CoordinateSystem);
%shared_ptr(DiscreteField);
%shared_ptr(DiscreteFieldBlock);
%shared_ptr(DiscreteFieldBlockComponent);
%shared_ptr(Discretization);
%shared_ptr(DiscretizationBlock);
%shared_ptr(Field);
%shared_ptr(Manifold);
%shared_ptr(Parameter);
%shared_ptr(ParameterValue);
%shared_ptr(Project);
%shared_ptr(SubDiscretization);
%shared_ptr(TangentSpace);
%shared_ptr(TensorComponent);
%shared_ptr(TensorType);

struct DataBlock;
struct DataRange;
#ifdef SIMULATIONIO_HAVE_HDF5
struct CopyObj;
struct DataSet;
struct ExtLink;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
struct ASDFData;
struct ASDFRef;
#endif

struct Basis;
struct BasisVector;
struct CoordinateField;
struct CoordinateSystem;
struct Configuration;
struct DiscreteField;
struct DiscreteFieldBlock;
struct DiscreteFieldBlockComponent;
struct Discretization;
struct DiscretizationBlock;
struct Field;
struct Manifold;
struct Parameter;
struct ParameterValue;
struct Project;
struct SubDiscretization;
struct TangentSpace;
struct TensorComponent;
struct TensorType;

%template(map_int_BasisVector)
  std::map<int, std::shared_ptr<BasisVector> >;
%template(map_int_CoordinateField)
  std::map<int, std::shared_ptr<CoordinateField> >;
%template(map_int_DiscreteFieldBlockComponent)
  std::map<int, std::shared_ptr<DiscreteFieldBlockComponent> >;
%template(map_int_TensorComponent)
  std::map<int, std::shared_ptr<TensorComponent> >;

%template(map_string_Basis)
  std::map<string, std::shared_ptr<Basis> >;
%template(map_string_BasisVector)
  std::map<string, std::shared_ptr<BasisVector> >;
%template(map_string_CoordinateField)
  std::map<string, std::shared_ptr<CoordinateField> >;
%template(map_string_CoordinateSystem)
  std::map<string, std::shared_ptr<CoordinateSystem> >;
%template(map_string_Configuration)
  std::map<string, std::shared_ptr<Configuration> >;
%template(map_string_DiscreteField)
  std::map<string, std::shared_ptr<DiscreteField> >;
%template(map_string_DiscreteFieldBlock)
  std::map<string, std::shared_ptr<DiscreteFieldBlock> >;
%template(map_string_DiscreteFieldBlockComponent)
  std::map<string, std::shared_ptr<DiscreteFieldBlockComponent> >;
%template(map_string_Discretization)
  std::map<string, std::shared_ptr<Discretization> >;
%template(map_string_DiscretizationBlock)
  std::map<string, std::shared_ptr<DiscretizationBlock> >;
%template(map_string_Field)
  std::map<string, std::shared_ptr<Field> >;
%template(map_string_Manifold)
  std::map<string, std::shared_ptr<Manifold> >;
%template(map_string_Parameter)
  std::map<string, std::shared_ptr<Parameter> >;
%template(map_string_ParameterValue)
  std::map<string, std::shared_ptr<ParameterValue> >;
%template(map_string_Project)
  std::map<string, std::shared_ptr<Project> >;
%template(map_string_SubDiscretization)
  std::map<string, std::shared_ptr<SubDiscretization> >;
%template(map_string_TangentSpace)
  std::map<string, std::shared_ptr<TangentSpace> >;
%template(map_string_TensorComponent)
  std::map<string, std::shared_ptr<TensorComponent> >;
%template(map_string_TensorType)
  std::map<string, std::shared_ptr<TensorType> >;

%template(map_string_weak_ptr_Basis)
  std::map<string, std::weak_ptr<Basis> >;
%template(map_string_weak_ptr_BasisVector)
  std::map<string, std::weak_ptr<BasisVector> >;
%template(map_string_weak_ptr_CoordinateField)
  std::map<string, std::weak_ptr<CoordinateField> >;
%template(map_string_weak_ptr_CoordinateSystem)
  std::map<string, std::weak_ptr<CoordinateSystem> >;
%template(map_string_weak_ptr_Configuration)
  std::map<string, std::weak_ptr<Configuration> >;
%template(map_string_weak_ptr_DiscreteField)
  std::map<string, std::weak_ptr<DiscreteField> >;
%template(map_string_weak_ptr_DiscreteFieldBlock)
  std::map<string, std::weak_ptr<DiscreteFieldBlock> >;
%template(map_string_weak_ptr_DiscreteFieldBlockComponent)
  std::map<string, std::weak_ptr<DiscreteFieldBlockComponent> >;
%template(map_string_weak_ptr_Discretization)
  std::map<string, std::weak_ptr<Discretization> >;
%template(map_string_weak_ptr_DiscretizationBlock)
  std::map<string, std::weak_ptr<DiscretizationBlock> >;
%template(map_string_weak_ptr_Field)
  std::map<string, std::weak_ptr<Field> >;
%template(map_string_weak_ptr_Manifold)
  std::map<string, std::weak_ptr<Manifold> >;
%template(map_string_weak_ptr_Parameter)
  std::map<string, std::weak_ptr<Parameter> >;
%template(map_string_weak_ptr_ParameterValue)
  std::map<string, std::weak_ptr<ParameterValue> >;
%template(map_string_weak_ptr_Project)
  std::map<string, std::weak_ptr<Project> >;
%template(map_string_weak_ptr_SubDiscretization)
  std::map<string, std::weak_ptr<SubDiscretization> >;
%template(map_string_weak_ptr_TangentSpace)
  std::map<string, std::weak_ptr<TangentSpace> >;
%template(map_string_weak_ptr_TensorComponent)
  std::map<string, std::weak_ptr<TensorComponent> >;
%template(map_string_weak_ptr_TensorType)
  std::map<string, std::weak_ptr<TensorType> >;

%template(shared_ptr_DataBlock) std::shared_ptr<DataBlock>;
%template(shared_ptr_DataRange) std::shared_ptr<DataRange>;
#ifdef SIMULATIONIO_HAVE_HDF5
%template(shared_ptr_CopyObj) std::shared_ptr<CopyObj>;
%template(shared_ptr_DataSet) std::shared_ptr<DataSet>;
%template(shared_ptr_ExtLink) std::shared_ptr<ExtLink>;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
%template(shared_ptr_ASDFData) std::shared_ptr<ASDFData>;
%template(shared_ptr_ASDFRef) std::shared_ptr<ASDFRef>;
#endif

%template(shared_ptr_Basis)
  std::shared_ptr<Basis>;
%template(shared_ptr_BasisVector)
  std::shared_ptr<BasisVector>;
%template(shared_ptr_CoordinateField)
  std::shared_ptr<CoordinateField>;
%template(shared_ptr_CoordinateSystem)
  std::shared_ptr<CoordinateSystem>;
%template(shared_ptr_Configuration)
  std::shared_ptr<Configuration>;
%template(shared_ptr_DiscreteField)
  std::shared_ptr<DiscreteField>;
%template(shared_ptr_DiscreteFieldBlock)
  std::shared_ptr<DiscreteFieldBlock>;
%template(shared_ptr_DiscreteFieldBlockComponent)
  std::shared_ptr<DiscreteFieldBlockComponent>;
%template(shared_ptr_Discretization)
  std::shared_ptr<Discretization>;
%template(shared_ptr_DiscretizationBlock)
  std::shared_ptr<DiscretizationBlock>;
%template(shared_ptr_Field)
  std::shared_ptr<Field>;
%template(shared_ptr_Manifold)
  std::shared_ptr<Manifold>;
%template(shared_ptr_Parameter)
  std::shared_ptr<Parameter>;
%template(shared_ptr_ParameterValue)
  std::shared_ptr<ParameterValue>;
%template(shared_ptr_Project)
  std::shared_ptr<Project>;
%template(shared_ptr_SubDiscretization)
  std::shared_ptr<SubDiscretization>;
%template(shared_ptr_TangentSpace)
  std::shared_ptr<TangentSpace>;
%template(shared_ptr_TensorComponent)
  std::shared_ptr<TensorComponent>;
%template(shared_ptr_TensorType)
  std::shared_ptr<TensorType>;

%template(vector_double) std::vector<double>;
%template(vector_int) std::vector<int>;

%template(weak_ptr_Basis)
  std::weak_ptr<Basis>;
%template(weak_ptr_BasisVector)
  std::weak_ptr<BasisVector>;
%template(weak_ptr_CoordinateField)
  std::weak_ptr<CoordinateField>;
%template(weak_ptr_CoordinateSystem)
  std::weak_ptr<CoordinateSystem>;
%template(weak_ptr_Configuration)
  std::weak_ptr<Configuration>;
%template(weak_ptr_DiscreteField)
  std::weak_ptr<DiscreteField>;
%template(weak_ptr_DiscreteFieldBlock)
  std::weak_ptr<DiscreteFieldBlock>;
%template(weak_ptr_DiscreteFieldBlockComponent)
  std::weak_ptr<DiscreteFieldBlockComponent>;
%template(weak_ptr_Discretization)
  std::weak_ptr<Discretization>;
%template(weak_ptr_DiscretizationBlock)
  std::weak_ptr<DiscretizationBlock>;
%template(weak_ptr_Field)
  std::weak_ptr<Field>;
%template(weak_ptr_Manifold)
  std::weak_ptr<Manifold>;
%template(weak_ptr_Parameter)
  std::weak_ptr<Parameter>;
%template(weak_ptr_ParameterValue)
  std::weak_ptr<ParameterValue>;
%template(weak_ptr_Project)
  std::weak_ptr<Project>;
%template(weak_ptr_SubDiscretization)
  std::weak_ptr<SubDiscretization>;
%template(weak_ptr_TangentSpace)
  std::weak_ptr<TangentSpace>;
%template(weak_ptr_TensorComponent)
  std::weak_ptr<TensorComponent>;
%template(weak_ptr_TensorType)
  std::weak_ptr<TensorType>;



%nodefaultctor;

struct DataBlock {
  box_t box() const;
  int rank() const;
  point_t shape() const;
  long long size() const;
  bool invariant() const;
};

struct DataRange: DataBlock {
  double origin() const;
  std::vector<double> delta() const;
};

#ifdef SIMULATIONIO_HAVE_HDF5

struct DataSet: DataBlock {
  // H5::DataSpace dataspace() const;
  // H5::DataType datatype() const;
  // H5::DataSet dataset() const;
  %extend {
    void writeData_int(const std::vector<int>& data,
                       const box_t& databox) const {
      self->writeData(data, databox);
    }
    void writeData_double(const std::vector<double>& data,
                          const box_t& databox) const {
      self->writeData(data, databox);
    }
    void writeData_int(const std::vector<int>& data) const {
      self->writeData(data);
    }
    void writeData_double(const std::vector<double>& data) const {
      self->writeData(data);
    }
    void attachData_int(const std::vector<int>& data,
                        const box_t& databox) const {
      self->attachData(data, databox);
    }
    void attachData_double(const std::vector<double>& data,
                           const box_t& databox) const {
      self->attachData(data, databox);
    }
  }
};

struct CopyObj: DataBlock {
  string name() const;
  %extend {
    std::vector<int> readData_int(const box_t& databox) const {
      return self->readData<int>(databox);
    }
    std::vector<double> readData_double(const box_t& databox) const {
      return self->readData<double>(databox);
    }
    std::vector<int> readData_int() const {
      return self->readData<int>();
    }
    std::vector<double> readData_double() const {
      return self->readData<double>();
    }
  }
};

struct ExtLink: DataBlock {
  string filename() const;
  string objname() const;
};

#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX

struct ASDFData: DataBlock {
};

struct ASDFRef: DataBlock {
};

#endif



struct Basis {
  string name() const;
  std::shared_ptr<TangentSpace> tangentspace() const;
  std::shared_ptr<Configuration> configuration() const;
  const std::map<string, std::shared_ptr<BasisVector> >& basisvectors() const;
  const std::map<int, std::shared_ptr<BasisVector> >& directions() const;
  bool invariant() const;

  std::shared_ptr<BasisVector> createBasisVector(const string& name, int dim);
};

struct BasisVector {
  string name() const;
  std::shared_ptr<Basis> basis() const;
  int direction() const;
  bool invariant() const;
};

struct Configuration {
  string name() const;
  std::shared_ptr<Project> project() const;
  const std::map<string, std::shared_ptr<ParameterValue> >&
    parametervalues() const;
  const std::map<string, std::weak_ptr<Basis> >& bases() const;
  const std::map<string, std::weak_ptr<CoordinateSystem> >&
    coordinatesystems() const;
  const std::map<string, std::weak_ptr<DiscreteField> >& discretefields() const;
  const std::map<string, std::weak_ptr<Discretization> >&
    discretizations() const;
  const std::map<string, std::weak_ptr<Field> >& fields() const;
  const std::map<string, std::weak_ptr<Manifold> >& manifolds() const;
  const std::map<string, std::weak_ptr<TangentSpace> >& tangentspaces() const;
  bool invariant() const;

  void
    insertParameterValue(const std::shared_ptr<ParameterValue>& parametervalue);
};

struct CoordinateField {
  string name() const;
  std::shared_ptr<CoordinateSystem> coordinatesystem() const;
  int direction() const;
  std::shared_ptr<Field> field() const;
  bool invariant() const;
};

struct CoordinateSystem {
  string name() const;
  std::shared_ptr<Project> project() const;
  std::shared_ptr<Configuration> configuration() const;
  std::shared_ptr<Manifold> manifold() const;
  const std::map<string, std::shared_ptr<CoordinateField> >&
    coordinatefields() const;
  const std::map<int, std::shared_ptr<CoordinateField> >& directions() const;
  bool invariant() const;

  std::shared_ptr<CoordinateField>
    createCoordinateField(string name, int direction,
                          const std::shared_ptr<Field>& field);
};

struct DiscreteField {
  string name() const;
  std::shared_ptr<Field> field() const;
  std::shared_ptr<Configuration> configuration() const;
  std::shared_ptr<Discretization> discretization() const;
  std::shared_ptr<Basis> basis() const;
  const std::map<string, std::shared_ptr<DiscreteFieldBlock> >&
    discretefieldblocks() const;
  bool invariant() const;

  std::shared_ptr<DiscreteFieldBlock>
    createDiscreteFieldBlock(const string& name,
                             const std::shared_ptr<DiscretizationBlock>&
                               discretizationblock);
};

struct DiscreteFieldBlock {
  string name() const;
  std::shared_ptr<DiscreteField> discretefield() const;
  std::shared_ptr<DiscretizationBlock> discretizationblock() const;
  const std::map<string, std::shared_ptr<DiscreteFieldBlockComponent> >&
    discretefieldblockcomponents() const;
  const std::map<int, std::shared_ptr<DiscreteFieldBlockComponent> >&
    storage_indices() const;
  bool invariant() const;

  std::shared_ptr<DiscreteFieldBlockComponent>
    createDiscreteFieldBlockComponent(const string& name,
                                      const std::shared_ptr<TensorComponent>&
                                        tensorcomponent);
};

struct DiscreteFieldBlockComponent {
  string name() const;
  std::shared_ptr<DiscreteFieldBlock> discretefieldblock() const;
  std::shared_ptr<TensorComponent> tensorcomponent() const;
  std::shared_ptr<DataBlock> datablock() const;
  std::shared_ptr<DataRange> datarange() const;
#ifdef SIMULATIONIO_HAVE_HDF5
  std::shared_ptr<DataSet> dataset() const;
  std::shared_ptr<CopyObj> copyobj() const;
  std::shared_ptr<ExtLink> extlink() const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  std::shared_ptr<ASDFData> asdfdata() const;
  std::shared_ptr<ASDFRef> asdfref() const;
#endif
  bool invariant() const;
  void unsetDataBlock();
  std::shared_ptr<DataRange>
    createDataRange(double origin, const std::vector<double>& delta);
#ifdef SIMULATIONIO_HAVE_HDF5
  %extend {
    std::shared_ptr<DataSet> createDataSet_int() {
      return self->createDataSet<int>();
    }
    std::shared_ptr<DataSet> createDataSet_double() {
      return self->createDataSet<double>();
    }
  }
  std::shared_ptr<CopyObj>
    createCopyObj(const H5::Group& group, const string& name);
  std::shared_ptr<CopyObj>
    createCopyObj(const H5::H5File& file, const string& name);
  // %extend {
  //   std::shared_ptr<CopyObj>
  //     createCopyObInGroup(long group_id, const string& name) {
  //     return self->createCopyObj(H5::Group(group_id), name);
  //   }
  //   std::shared_ptr<CopyObj>
  //     createCopyObInFile(long file_id, const string& name) {
  //     return self->createCopyObj(H5::H5File(file_id), name);
  //   }
  // }
  std::shared_ptr<ExtLink>
  createExtLink(const string& filename, const string& objname);
#endif
  // TODO: Eliminate these (requires writing hyperslabs for the combiners)
  string getPath() const;
  string getName() const;
};

struct Discretization {
  string name() const;
  std::shared_ptr<Manifold> manifold() const;
  std::shared_ptr<Configuration> configuration() const;
  const std::map<string, std::shared_ptr<DiscretizationBlock> >&
    discretizationblocks() const;
  const std::map<string, std::weak_ptr<SubDiscretization> >&
    child_discretizations() const;
  const std::map<string, std::weak_ptr<SubDiscretization> >&
    parent_discretizations() const;
  bool invariant() const;

  std::shared_ptr<DiscretizationBlock>
    createDiscretizationBlock(const string& name);
};

struct DiscretizationBlock {
  string name() const;
  box_t box() const;
  region_t active() const;
  std::shared_ptr<Discretization> discretization() const;
  bool invariant() const;
  void setBox(const box_t& box);
  void setActive(const region_t& active);
};

struct Field {
  string name() const;
  std::shared_ptr<Project> project() const;
  std::shared_ptr<Configuration> configuration() const;
  std::shared_ptr<Manifold> manifold() const;
  std::shared_ptr<TangentSpace> tangentspace() const;
  std::shared_ptr<TensorType> tensortype() const;
  const std::map<string, std::shared_ptr<DiscreteField> >&
    discretefields() const;
  bool invariant() const;

  std::shared_ptr<DiscreteField>
    createDiscreteField(const string& name,
                        const std::shared_ptr<Configuration>& configuration,
                        const std::shared_ptr<Discretization>& discretization,
                        const std::shared_ptr<Basis>& basis);
};

struct Manifold {
  string name() const;
  std::shared_ptr<Project> project() const;
  std::shared_ptr<Configuration> configuration() const;
  int dimension() const;
  const std::map<string, std::shared_ptr<Discretization> >&
    discretizations() const;
  const std::map<string, std::weak_ptr<Field> >& fields() const;
  const std::map<string, std::weak_ptr<CoordinateSystem> >&
    coordinatesystems() const;
  bool invariant() const;

  std::shared_ptr<Discretization>
    createDiscretization(const string& name,
                         const std::shared_ptr<Configuration>& configuration);
  std::shared_ptr<SubDiscretization>
    createSubDiscretization(const string& name,
                            const std::shared_ptr<Discretization>&
                              parent_discretization,
                            const std::shared_ptr<Discretization>&
                              child_discretization,
                            const std::vector<double>& factor,
                            const std::vector<double>& offset);
};

struct Parameter {
  string name() const;
  std::shared_ptr<Project> project() const;
  const std::map<string, std::shared_ptr<ParameterValue> >&
    parametervalues() const;
  bool invariant() const;

  std::shared_ptr<ParameterValue> createParameterValue(const string& name);
};

struct ParameterValue {
  string name() const;
  std::shared_ptr<Parameter> parameter() const;
  const std::map<string, std::weak_ptr<Configuration> >& configurations() const;
  enum value_type_t {
    type_empty,
    type_int,
    type_double,
    type_string,
  } value_type;
  bool invariant() const;
  value_type_t getValueType() const;
  int getValueInt() const;
  double getValueDouble() const;
  string getValueString() const;
  %extend {
    void setValue_empty()
    {
      self->setValue();
    }
    void setValue_int(int i)
    {
      self->setValue(i);
    }
    void setValue_double(double d)
    {
      self->setValue(d);
    }
    void setValue_string(string s)
    {
      self->setValue(s);
    }
    // These are deprecated
    int getValue_int() const
    {
      assert(self->value_type == ParameterValue::type_int);
      return self->value_int;
    }
    double getValue_double() const
    {
      assert(self->value_type == ParameterValue::type_double);
      return self->value_double;
    }
    string getValue_string() const
    {
      assert(self->value_type == ParameterValue::type_string);
      return self->value_string;
    }
  }
};

struct Project {
  string name() const;
  const std::map<string, std::shared_ptr<Parameter> >& parameters() const;
  const std::map<string, std::shared_ptr<Configuration> >&
    configurations() const;
  const std::map<string, std::shared_ptr<TensorType> >& tensortypes() const;
  const std::map<string, std::shared_ptr<Manifold> >& manifolds() const;
  const std::map<string, std::shared_ptr<TangentSpace> >& tangentspaces() const;
  const std::map<string, std::shared_ptr<Field> >& fields() const;
  const std::map<string, std::shared_ptr<CoordinateSystem> >&
    coordinatesystems() const;
  bool invariant() const;

  void createStandardTensorTypes();
#ifdef SIMULATIONIO_HAVE_HDF5
  void write(const H5::H5Location& loc);
  void writeHDF5(const string& name);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF
  void writeASDF(const string& name);
#endif

  std::shared_ptr<Parameter> createParameter(const string& name);
  std::shared_ptr<Configuration> createConfiguration(const string& name);
  std::shared_ptr<TensorType> createTensorType(const string& name,
                                               int dimension, int rank);
  std::shared_ptr<Manifold>
    createManifold(const string& name,
                   const std::shared_ptr<Configuration>& configuration,
                   int dimension);
  std::shared_ptr<TangentSpace>
    createTangentSpace(const string& name,
                       const std::shared_ptr<Configuration>& configuration,
                       int dimension);
  std::shared_ptr<Field>
    createField(const string& name,
                const std::shared_ptr<Configuration>& configuration,
                const std::shared_ptr<Manifold>& manifold,
                const std::shared_ptr<TangentSpace>& tangentspace,
                const std::shared_ptr<TensorType>& tensortype);
  std::shared_ptr<CoordinateSystem>
    createCoordinateSystem(const string& name,
                           const std::shared_ptr<Configuration>& configuration,
                           const std::shared_ptr<Manifold>& manifold);
};
std::shared_ptr<Project> createProject(const string& name);
#ifdef SIMULATIONIO_HAVE_HDF5
std::shared_ptr<Project> readProject(const H5::H5Location& loc);
std::shared_ptr<Project> readProjectHDF5(const string& name);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF
std::shared_ptr<Project> readProjectASDF(const string& name);
#endif

struct SubDiscretization {
  string name() const;
  std::shared_ptr<Manifold> manifold() const;
  std::shared_ptr<Discretization> parent_discretization() const;
  std::shared_ptr<Discretization> child_discretization() const;
  std::vector<double> factor() const;
  std::vector<double> offset() const;
  std::vector<double> child2parent(const std::vector<double>& child_idx) const;
  std::vector<double> parent2child(const std::vector<double>& parent_idx) const;
  bool invariant() const;
};

struct TangentSpace {
  string name() const;
  std::shared_ptr<Project> project() const;
  std::shared_ptr<Configuration> configuration() const;
  int dimension() const;
  const std::map<string, std::shared_ptr<Basis> >& bases() const;
  const std::map<string, std::weak_ptr<Field> >& fields() const;
  bool invariant() const;

  std::shared_ptr<Basis>
    createBasis(const string& name,
                const std::shared_ptr<Configuration>& configuration);
};

struct TensorComponent {
  string name() const;
  std::shared_ptr<TensorType> tensortype() const;
  int storage_index() const;
  std::vector<int> indexvalues() const;
  bool invariant() const;
};

struct TensorType {
  string name() const;
  std::shared_ptr<Project> project() const;
  int dimension() const;
  int rank() const;
  const std::map<string, std::shared_ptr<TensorComponent> >&
    tensorcomponents() const;
  const std::map<int, std::shared_ptr<TensorComponent> >&
    storage_indices() const;

  std::shared_ptr<TensorComponent>
    createTensorComponent(const string& name, int storage_index,
                          const std::vector<int>& indexvalues);
};

%clearnodefaultctor;
