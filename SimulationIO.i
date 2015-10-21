// SWIG interface file

%module SimulationIO

%{
  #include <H5Cpp.h>
  #include "SimulationIO.hpp"
  using namespace SimulationIO;
%}

%include <std_map.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

%shared_ptr(Basis);
%shared_ptr(BasisVector);
%shared_ptr(Configuration);
%shared_ptr(DiscreteField);
%shared_ptr(DiscreteFieldBlock);
%shared_ptr(DiscreteFieldBlockData);
%shared_ptr(Discretization);
%shared_ptr(DiscretizationBlock);
%shared_ptr(Field);
%shared_ptr(Manifold);
%shared_ptr(Parameter);
%shared_ptr(ParaemterValue);
%shared_ptr(Project);
%shared_ptr(TangentSpace);
%shared_ptr(TensorComponent);
%shared_ptr(TensorType);

// Note: SWIG's support for map and shared_ptr does not work with namespaces
// using std::map;
// using std::shared_ptr;
using std::string;

%nodefaultctor;

struct Basis;
struct BasisVector;
struct Configuration;
struct DiscreteField;
struct DiscreteFieldBlock;
struct DiscreteFieldBlockData;
struct Discretization;
struct DiscretizationBlock;
struct Field;
struct Manifold;
struct Parameter;
struct ParameterValue;
struct Project;
struct TangentSpace;
struct TensorComponent;
struct TensorType;

%template(map_string_Basis) std::map<string, std::shared_ptr<Basis> >;
%template(map_string_BasisVector)
  std::map<string, std::shared_ptr<BasisVector> >;
%template(map_string_Configuration)
  std::map<string, std::shared_ptr<Configuration> >;
%template(map_string_DiscreteField)
  std::map<string, std::shared_ptr<DiscreteField> >;
%template(map_string_DiscreteFieldBlock)
  std::map<string, std::shared_ptr<DiscreteFieldBlock> >;
%template(map_string_DiscreteFieldBlockData)
  std::map<string, std::shared_ptr<DiscreteFieldBlockData> >;
%template(map_string_Discretization)
  std::map<string, std::shared_ptr<Discretization> >;
%template(map_string_DiscretizationBlock)
  std::map<string, std::shared_ptr<DiscretizationBlock> >;
%template(map_string_Field) std::map<string, std::shared_ptr<Field> >;
%template(map_string_Manifold) std::map<string, std::shared_ptr<Manifold> >;
%template(map_string_Parameter) std::map<string, std::shared_ptr<Parameter> >;
%template(map_string_ParameterValue)
  std::map<string, std::shared_ptr<ParameterValue> >;
%template(map_string_Project) std::map<string, std::shared_ptr<Project> >;
%template(map_string_TangentSpace)
  std::map<string, std::shared_ptr<TangentSpace> >;
%template(map_string_TensorComponent)
  std::map<string, std::shared_ptr<TensorComponent> >;
%template(map_string_TensorType) std::map<string, std::shared_ptr<TensorType> >;

%template(map_int_BasisVector) std::map<int, std::shared_ptr<BasisVector> >;
%template(map_int_TensorComponent)
  std::map<int, std::shared_ptr<TensorComponent> >;
%template(vector_int) std::vector<int>;

struct Basis {
  string name;
  std::weak_ptr<TangentSpace> tangentspace;
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
  bool invariant() const;

  void
    insertParameterValue(const std::shared_ptr<ParameterValue>& parametervalue);
};

struct DiscreteField {
  string name;
  std::weak_ptr<Field> field;
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
  std::map<string, std::shared_ptr<DiscreteFieldBlockData> >
    discretefieldblockdata;
  bool invariant() const;

  std::shared_ptr<DiscreteFieldBlockData>
    createDiscreteFieldBlockData(const string& name,
                                 const std::shared_ptr<TensorComponent>&
                                   tensorcomponent);
};

struct DiscreteFieldBlockData {
  string name;
  std::weak_ptr<DiscreteFieldBlock> discretefieldblock;
  std::shared_ptr<TensorComponent> tensorcomponent;
  bool invariant() const;
};

struct Discretization {
  string name;
  std::weak_ptr<Manifold> manifold;
  std::map<string, std::shared_ptr<DiscretizationBlock> > discretizationblocks;
  bool invariant() const;

  std::shared_ptr<DiscretizationBlock>
    createDiscretizationBlock(const string &name);
};

struct DiscretizationBlock {
  string name;
  std::weak_ptr<Discretization> discretization;
  bool invariant() const;
};

struct Field {
  string name;
  std::weak_ptr<Project> project;
  std::shared_ptr<Manifold> manifold;
  std::shared_ptr<TangentSpace> tangentspace;
  std::shared_ptr<TensorType> tensortype;
  std::map<string, std::shared_ptr<DiscreteField> > discretefields;
  bool invariant() const;

  std::shared_ptr<DiscreteField>
    createDiscreteField(const string&name,
                        const std::shared_ptr<Discretization>& discretization,
                        const std::shared_ptr<Basis>& basis);
};

struct Manifold {
  string name;
  std::weak_ptr<Project> project;
  int dimension;
  std::map<string, std::shared_ptr<Discretization> > discretizations;
  std::map<string, std::weak_ptr<Field> > fields;
  bool invariant() const;

  std::shared_ptr<Discretization> createDiscretization(const string &name);
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
  bool invariant() const;

  void createStandardTensorTypes();
  void write(const H5::CommonFG& loc);

  std::shared_ptr<Parameter> createParameter(const string &name);
  std::shared_ptr<Configuration> createConfiguration(const string &name);
  std::shared_ptr<TensorType>
    createTensorType(const string &name, int dimension, int rank);
  std::shared_ptr<Manifold> createManifold(const string &name, int dimension);
  std::shared_ptr<TangentSpace>
    createTangentSpace(const string &name, int dimension);
  std::shared_ptr<Field>
    createField(const string& name,
                const std::shared_ptr<Manifold>& manifold,
                const std::shared_ptr<TangentSpace>& tangentspace,
                const std::shared_ptr<TensorType>& tensortype);
};
std::shared_ptr<Project> createProject(const string& name);

struct TangentSpace {
  string name;
  std::weak_ptr<Project> project;
  int dimension;
  std::map<string, std::shared_ptr<Basis> > bases;
  std::map<string, std::weak_ptr<Field> > fields;
  bool invariant() const;

  std::shared_ptr<Basis> createBasis(const string& name);
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