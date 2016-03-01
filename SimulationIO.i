// SWIG interface file

%module SimulationIO

%{
#include <H5Cpp.h>
#include "RegionCalculus.hpp"
#include "SimulationIO.hpp"
using namespace SimulationIO;
typedef RegionCalculus::dbox<int> ibox;
typedef RegionCalculus::dregion<int> iregion;
%}

%include <std_map.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

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

namespace std {
template<typename T> class weak_ptr {
public:
  typedef T element_type;

  weak_ptr();
  weak_ptr(const weak_ptr&);
  weak_ptr(const shared_ptr<T>&);
  template<typename U> weak_ptr(const weak_ptr<U>&);
  template<typename U> weak_ptr(const shared_ptr<U>&);

  void swap(weak_ptr&);
  void reset();

  long use_count() const;
  bool expired() const;
  shared_ptr<T> lock() const;
};
}

// Note: SWIG's support for map and shared_ptr does not work with namespaces
// using std::map;
// using std::shared_ptr;
using std::string;

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



// Use __getitem__ instead
//TODO %{
//TODO template<typename K, typename T>
//TODO const T& map_get(const std::map<K,T>& m, const K& k) { return m.at(k); }
//TODO %}
//TODO template<typename K, typename T>
//TODO const T& map_get(const std::map<K,T>& m, const K& k) { return m.at(k); }

//TODO %template(map_get_string_Parameter)
//TODO   map_get<string, std::shared_ptr<Parameter> >;
//TODO %template(map_get_string_ParameterValue)
//TODO   map_get<string, std::shared_ptr<ParameterValue> >;
//TODO %template(map_get_string_TensorType)
//TODO   map_get<string, std::shared_ptr<TensorType> >;

/*
template<typename K, typename T>
typename map<K,T>::const_iterator map_start(const std::map<K,T>& m) {
  return m.begin();
}
template<typename K, typename T>
bool map_done(const std::map<K,T>& m,
              const typename std::map<K,T>::const_iterator& mi) {
  return mi == m.end();
}
template<typename K, typename T>
std::pair<T, typename std::map<K,T>::const_iterator>
map_next(const std::map<K,T>& m,
         const typename std::map<K,T>::const_iterator& mi) {
  return std::make_pair(*mi, ++mi);
}
*/

%nodefaultctor;

struct Basis {
  string name;
  std::weak_ptr<TangentSpace> tangentspace;
  std::shared_ptr<Configuration> configuration;
  std::map<string, std::shared_ptr<BasisVector> > basisvectors;
  std::map<int, std::shared_ptr<BasisVector> > directions;
  bool invariant() const;

  std::shared_ptr<BasisVector> createBasisVector(const string& name, int dim);
};

struct BasisVector {
  string name;
  std::weak_ptr<Basis> basis;
  int direction;
  bool invariant() const;
};

struct Configuration {
  string name;
  std::weak_ptr<Project> project;
  std::map<string, std::shared_ptr<ParameterValue> > parametervalues;
  std::map<string, std::weak_ptr<Basis> > bases;
  std::map<string, std::weak_ptr<CoordinateSystem> > coordinatesystems;
  std::map<string, std::weak_ptr<DiscreteField> > discretefields;
  std::map<string, std::weak_ptr<Discretization> > discretizations;
  std::map<string, std::weak_ptr<Field> > fields;
  std::map<string, std::weak_ptr<Manifold> > manifolds;
  std::map<string, std::weak_ptr<TangentSpace> > tangentspaces;
  bool invariant() const;

  void
    insertParameterValue(const std::shared_ptr<ParameterValue>& parametervalue);
};

struct CoordinateField {
  string name;
  std::weak_ptr<CoordinateSystem> coordinatesystem;
  int direction;
  std::shared_ptr<Field> field;
  bool invariant();
};

struct CoordinateSystem {
  string name;
  std::weak_ptr<Project> project;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<Manifold> manifold;
  std::map<string, std::shared_ptr<CoordinateField> > coordinatefields;
  std::map<int, std::shared_ptr<CoordinateField> > directions;
  bool invariant();

  std::shared_ptr<CoordinateField>
    createCoordinateField(string name,
                          int direction,
                          std::shared_ptr<Field>& field);
};

struct DiscreteField {
  string name;
  std::weak_ptr<Field> field;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<Discretization> discretization;
  std::shared_ptr<Basis> basis;
  std::map<string, std::shared_ptr<DiscreteFieldBlock> > discretefieldblocks;
  bool invariant() const;

  std::shared_ptr<DiscreteFieldBlock>
    createDiscreteFieldBlock(const string& name,
                             const std::shared_ptr<DiscretizationBlock>&
                               discretizationblock);
};

struct DiscreteFieldBlock {
  string name;
  std::weak_ptr<DiscreteField> discretefield;
  std::shared_ptr<DiscretizationBlock> discretizationblock;
  std::map<string, std::shared_ptr<DiscreteFieldBlockComponent> >
    discretefieldblockcomponents;
  std::map<int, std::shared_ptr<DiscreteFieldBlockComponent> > storage_indices;
  bool invariant() const;

  std::shared_ptr<DiscreteFieldBlockComponent>
    createDiscreteFieldBlockComponent(const string& name,
                                      const std::shared_ptr<TensorComponent>&
                                        tensorcomponent);
};

struct DiscreteFieldBlockComponent {
  string name;
  std::weak_ptr<DiscreteFieldBlock> discretefieldblock;
  std::shared_ptr<TensorComponent> tensorcomponent;
  H5::DataSet data_dataset;
  bool invariant() const;
  void setData();
  void setData(const H5::DataType &datatype, const H5::DataSpace& dataspace);
  %extend {
    H5::DataSet getData_DataSet() const {
      return self->data_dataset;
    }
  }
  string getPath() const;
  string getName() const;
  %extend {
    void writeData_int(const std::vector<int>& data) const {
      self->writeData(data);
    }
    void writeData_double(const std::vector<double>& data) const {
      self->writeData(data);
    }
  }
};

struct Discretization {
  string name;
  std::weak_ptr<Manifold> manifold;
  std::shared_ptr<Configuration> configuration;
  std::map<string, std::shared_ptr<DiscretizationBlock> > discretizationblocks;
  std::map<string, std::weak_ptr<SubDiscretization> > child_discretizations;
  std::map<string, std::weak_ptr<SubDiscretization> > parent_discretizations;
  bool invariant() const;

  std::shared_ptr<DiscretizationBlock>
    createDiscretizationBlock(const string& name);
};

struct DiscretizationBlock {
  string name;
  ibox region;
  iregion active;
  std::weak_ptr<Discretization> discretization;
  bool invariant() const;
  %extend {
    void setRegion(const std::vector<int>& ioffset,
                   const std::vector<int>& ishape) {
      std::vector<hssize_t> hoffset(ioffset.size()), hshape(ishape.size());
      std::copy(ioffset.begin(), ioffset.end(), hoffset.begin());
      std::copy(ishape.begin(), ishape.end(), hshape.begin());
      self->setRegion(box_t(hoffset, point_t(hoffset) + hshape));
    }
  }
};

struct Field {
  string name;
  std::weak_ptr<Project> project;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<Manifold> manifold;
  std::shared_ptr<TangentSpace> tangentspace;
  std::shared_ptr<TensorType> tensortype;
  std::map<string, std::shared_ptr<DiscreteField> > discretefields;
  bool invariant() const;

  std::shared_ptr<DiscreteField>
    createDiscreteField(const string& name,
                        const std::shared_ptr<Configuration>& configuration,
                        const std::shared_ptr<Discretization>& discretization,
                        const std::shared_ptr<Basis>& basis);
};

struct Manifold {
  string name;
  std::weak_ptr<Project> project;
  std::shared_ptr<Configuration> configuration;
  int dimension;
  std::map<string, std::shared_ptr<Discretization> > discretizations;
  std::map<string, std::weak_ptr<Field> > fields;
  std::map<string, std::weak_ptr<CoordinateSystem> > coordinatesystems;
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
  string name;
  std::weak_ptr<Project> project;
  std::map<string, std::shared_ptr<ParameterValue> > parametervalues;
  bool invariant() const;

  std::shared_ptr<ParameterValue> createParameterValue(const string& name);
};

struct ParameterValue {
  string name;
  std::weak_ptr<Parameter> parameter;
  std::map<string, std::weak_ptr<Configuration> > configurations;
  bool invariant() const;
};

struct Project {
  string name;
  std::map<string, std::shared_ptr<Parameter> > parameters;
  std::map<string, std::shared_ptr<Configuration> > configurations;
  std::map<string, std::shared_ptr<TensorType> > tensortypes;
  std::map<string, std::shared_ptr<Manifold> > manifolds;
  std::map<string, std::shared_ptr<TangentSpace> > tangentspaces;
  std::map<string, std::shared_ptr<Field> > fields;
  std::map<string, std::shared_ptr<CoordinateSystem> > coordinatesystems;
  bool invariant() const;

  void createStandardTensorTypes();
  void write(const H5::CommonFG& loc);

  std::shared_ptr<Parameter> createParameter(const string& name);
  std::shared_ptr<Configuration> createConfiguration(const string& name);
  std::shared_ptr<TensorType>
    createTensorType(const string& name, int dimension, int rank);
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
std::shared_ptr<Project> readProject(const H5::CommonFG &loc);

struct SubDiscretization {
  string name;
  std::weak_ptr<Manifold> manifold;
  std::shared_ptr<Discretization> parent_discretization;
  std::shared_ptr<Discretization> child_discretization;
  std::vector<double> factor;
  std::vector<double> offset;
  vector<double> child2parent(const vector<double> &child_idx) const;
  vector<double> parent2child(const vector<double> &parent_idx) const;
  bool invariant() const;
};

struct TangentSpace {
  string name;
  std::weak_ptr<Project> project;
  std::shared_ptr<Configuration> configuration;
  int dimension;
  std::map<string, std::shared_ptr<Basis> > bases;
  std::map<string, std::weak_ptr<Field> > fields;
  bool invariant() const;

  std::shared_ptr<Basis>
    createBasis(const string& name,
                const std::shared_ptr<Configuration>& configuration);
};

struct TensorComponent {
  string name;
  std::weak_ptr<TensorType> tensortype;
  int storage_index;
  std::vector<int> indexvalues;
  bool invariant() const;
};

struct TensorType {
  string name;
  std::weak_ptr<Project> project;
  int dimension;
  int rank;
  std::map<string, std::shared_ptr<TensorComponent> > tensorcomponents;
  std::map<int, std::shared_ptr<TensorComponent> > storage_indices;

  std::shared_ptr<TensorComponent>
    createTensorComponent(const string& name,
                          int storage_index,
                          const std::vector<int>& indexvalues);
};

%clearnodefaultctor;
