#include "SimulationIO.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

using std::ostringstream;
using std::remove;
using std::shared_ptr;
using std::string;

using namespace SimulationIO;

TEST(ipow, ipow) {
  EXPECT_EQ(1, ipow(0, 0)); // Yes, this needs to be 1
  EXPECT_EQ(0, ipow(0, 1));
  EXPECT_EQ(0, ipow(0, 2));
  EXPECT_EQ(1, ipow(1, 0));
  EXPECT_EQ(1, ipow(1, 1));
  EXPECT_EQ(1, ipow(1, 2));
  EXPECT_EQ(1, ipow(2, 0));
  EXPECT_EQ(2, ipow(2, 1));
  EXPECT_EQ(4, ipow(2, 2));
}

shared_ptr<Project> project;

TEST(Project, create) {
  project = createProject("p1");
  EXPECT_TRUE(project->invariant());
  ostringstream buf;
  buf << *project;
  EXPECT_EQ("Project \"p1\"\n", buf.str());
}

TEST(Project, HDF5) {
  auto filename = "project.h5";
  string orig;
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
    ostringstream buf;
    buf << *project;
    orig = buf.str();
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p2 = createProject(file);
    ostringstream buf;
    buf << *p2;
    EXPECT_EQ(orig, buf.str());
  }
  remove(filename);
}

// This test sets up the global variable "project", which is necessary for most
// of the following tests. It must be the last of the "Project" tests. TODO:
// Avoid this dependency.
TEST(Project, setup) {
  project->createStandardTensorTypes();
  EXPECT_EQ(3, project->tensortypes.size());
}

TEST(Parameter, create) {
  EXPECT_TRUE(project->parameters.empty());
  const auto &par1 = project->createParameter("par1");
  const auto &par2 = project->createParameter("par2");
  const auto &par3 = project->createParameter("par3");
  EXPECT_EQ(3, project->parameters.size());
  EXPECT_EQ(par1, project->parameters.at("par1"));
  EXPECT_EQ(par2, project->parameters.at("par2"));
  EXPECT_EQ(par3, project->parameters.at("par3"));
}

TEST(Parameter, HDF5) {
  auto filename = "parameter.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->parameters.at("par1");
    buf << *p1->parameters.at("par2");
    buf << *p1->parameters.at("par3");
    EXPECT_EQ("Parameter \"par1\"\n"
              "Parameter \"par2\"\n"
              "Parameter \"par3\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(ParameterValue, create) {
  const auto &par1 = project->parameters.at("par1");
  const auto &par2 = project->parameters.at("par2");
  const auto &par3 = project->parameters.at("par3");
  EXPECT_TRUE(par1->parametervalues.empty());
  EXPECT_TRUE(par2->parametervalues.empty());
  EXPECT_TRUE(par3->parametervalues.empty());
  auto val1 = par1->createParameterValue("val1");
  auto val2 = par2->createParameterValue("val2");
  auto val3 = par2->createParameterValue("val3");
  auto val4 = par3->createParameterValue("val4");
  val1->setValue(1);
  val2->setValue(2.0);
  val3->setValue(3.0);
  val4->setValue("four");
  EXPECT_EQ(1, par1->parametervalues.size());
  EXPECT_EQ(2, par2->parametervalues.size());
  EXPECT_EQ(1, par3->parametervalues.size());
  EXPECT_EQ(val1, par1->parametervalues.at("val1"));
  EXPECT_EQ(val2, par2->parametervalues.at("val2"));
  EXPECT_EQ(val3, par2->parametervalues.at("val3"));
  EXPECT_EQ(val4, par3->parametervalues.at("val4"));
}

TEST(ParameterValue, HDF5) {
  auto filename = "parametervalue.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->parameters.at("par1");
    buf << *p1->parameters.at("par2");
    buf << *p1->parameters.at("par3");
    EXPECT_EQ("Parameter \"par1\"\n"
              "  ParameterValue \"val1\": Parameter \"par1\"\n"
              "    value: int(1)\n"
              "Parameter \"par2\"\n"
              "  ParameterValue \"val2\": Parameter \"par2\"\n"
              "    value: double(2)\n"
              "  ParameterValue \"val3\": Parameter \"par2\"\n"
              "    value: double(3)\n"
              "Parameter \"par3\"\n"
              "  ParameterValue \"val4\": Parameter \"par3\"\n"
              "    value: string(\"four\")\n",
              buf.str());
  }
  remove(filename);
}

TEST(Configuration, create) {
  EXPECT_TRUE(project->configurations.empty());
  const auto &conf1 = project->createConfiguration("conf1");
  const auto &conf2 = project->createConfiguration("conf2");
  const auto &par1 = project->parameters.at("par1");
  const auto &par2 = project->parameters.at("par2");
  const auto &val1 = par1->parametervalues.at("val1");
  const auto &val2 = par2->parametervalues.at("val2");
  EXPECT_EQ(2, project->configurations.size());
  EXPECT_EQ(conf1, project->configurations.at("conf1"));
  EXPECT_EQ(conf2, project->configurations.at("conf2"));
  EXPECT_TRUE(conf1->parametervalues.empty());
  EXPECT_TRUE(conf2->parametervalues.empty());
  conf2->insertParameterValue(val1);
  conf2->insertParameterValue(val2);
  EXPECT_TRUE(conf1->parametervalues.empty());
  EXPECT_EQ(2, conf2->parametervalues.size());
  EXPECT_EQ(val1, conf2->parametervalues.at("val1"));
  EXPECT_EQ(val2, conf2->parametervalues.at("val2"));
}

TEST(Configuration, HDF5) {
  auto filename = "configuration.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->configurations.at("conf1");
    buf << *p1->configurations.at("conf2");
    EXPECT_EQ("Configuration \"conf1\"\n"
              "Configuration \"conf2\"\n"
              "  Parameter \"par1\" ParameterValue \"val1\"\n"
              "  Parameter \"par2\" ParameterValue \"val2\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(TensorTypes, Scalar3D) {
  const auto &tt = project->tensortypes.at("Scalar3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(0, tt->rank);
  EXPECT_EQ(1, tt->tensorcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents)
    EXPECT_TRUE(tc.second->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"Scalar3D\": dim=3 rank=0\n"
            "  TensorComponent \"scalar\": TensorType \"Scalar3D\" "
            "storage_index=0 indexvalues=[]\n",
            buf.str());
}

TEST(TensorTypes, Vector3D) {
  const auto &tt = project->tensortypes.at("Vector3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(1, tt->rank);
  EXPECT_EQ(3, tt->tensorcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents)
    EXPECT_TRUE(tc.second->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"Vector3D\": dim=3 rank=1\n"
            "  TensorComponent \"0\": TensorType \"Vector3D\" storage_index=0 "
            "indexvalues=[0]\n"
            "  TensorComponent \"1\": TensorType \"Vector3D\" storage_index=1 "
            "indexvalues=[1]\n"
            "  TensorComponent \"2\": TensorType \"Vector3D\" storage_index=2 "
            "indexvalues=[2]\n",
            buf.str());
}

TEST(TensorTypes, SymmetricTensor3D) {
  const auto &tt = project->tensortypes.at("SymmetricTensor3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(2, tt->rank);
  EXPECT_EQ(6, tt->tensorcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents)
    EXPECT_TRUE(tc.second->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"SymmetricTensor3D\": dim=3 rank=2\n"
            "  TensorComponent \"00\": TensorType \"SymmetricTensor3D\" "
            "storage_index=0 indexvalues=[0,0]\n"
            "  TensorComponent \"01\": TensorType \"SymmetricTensor3D\" "
            "storage_index=1 indexvalues=[0,1]\n"
            "  TensorComponent \"02\": TensorType \"SymmetricTensor3D\" "
            "storage_index=2 indexvalues=[0,2]\n"
            "  TensorComponent \"11\": TensorType \"SymmetricTensor3D\" "
            "storage_index=3 indexvalues=[1,1]\n"
            "  TensorComponent \"12\": TensorType \"SymmetricTensor3D\" "
            "storage_index=4 indexvalues=[1,2]\n"
            "  TensorComponent \"22\": TensorType \"SymmetricTensor3D\" "
            "storage_index=5 indexvalues=[2,2]\n",
            buf.str());
}

TEST(TensorTypes, HDF5) {
  auto filename = "tensortypes.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    for (const auto &tt : p1->tensortypes)
      buf << *tt.second;
    EXPECT_EQ("TensorType \"Scalar3D\": dim=3 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar3D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"SymmetricTensor3D\": dim=3 rank=2\n"
              "  TensorComponent \"00\": TensorType \"SymmetricTensor3D\" "
              "storage_index=0 indexvalues=[0,0]\n"
              "  TensorComponent \"01\": TensorType \"SymmetricTensor3D\" "
              "storage_index=1 indexvalues=[0,1]\n"
              "  TensorComponent \"02\": TensorType \"SymmetricTensor3D\" "
              "storage_index=2 indexvalues=[0,2]\n"
              "  TensorComponent \"11\": TensorType \"SymmetricTensor3D\" "
              "storage_index=3 indexvalues=[1,1]\n"
              "  TensorComponent \"12\": TensorType \"SymmetricTensor3D\" "
              "storage_index=4 indexvalues=[1,2]\n"
              "  TensorComponent \"22\": TensorType \"SymmetricTensor3D\" "
              "storage_index=5 indexvalues=[2,2]\n"
              "TensorType \"Vector3D\": dim=3 rank=1\n"
              "  TensorComponent \"0\": TensorType \"Vector3D\" "
              "storage_index=0 indexvalues=[0]\n"
              "  TensorComponent \"1\": TensorType \"Vector3D\" "
              "storage_index=1 indexvalues=[1]\n"
              "  TensorComponent \"2\": TensorType \"Vector3D\" "
              "storage_index=2 indexvalues=[2]\n",
              buf.str());
  }
  remove(filename);
}

TEST(Manifold, create) {
  EXPECT_TRUE(project->manifolds.empty());
  auto m1 = project->createManifold("m1", 3);
  EXPECT_EQ(1, project->manifolds.size());
  EXPECT_EQ(m1, project->manifolds.at("m1"));
}

TEST(Manifold, HDF5) {
  auto filename = "manifold.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->manifolds.at("m1");
    EXPECT_EQ("Manifold \"m1\": dim=3\n", buf.str());
  }
  remove(filename);
}

TEST(TangentSpace, create) {
  EXPECT_TRUE(project->tangentspaces.empty());
  const auto &s1 = project->createTangentSpace("s1", 3);
  EXPECT_EQ(1, project->tangentspaces.size());
  EXPECT_EQ(s1, project->tangentspaces.at("s1"));
}

TEST(TangentSpace, HDF5) {
  auto filename = "tangentspace.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces.at("s1");
    EXPECT_EQ("TangentSpace \"s1\": dim=3\n", buf.str());
  }
  remove(filename);
}

TEST(Field, create) {
  EXPECT_TRUE(project->fields.empty());
  const auto &m1 = project->manifolds.at("m1");
  const auto &s1 = project->tangentspaces.at("s1");
  const auto &s3d = project->tensortypes.at("Vector3D");
  auto f1 = project->createField("f1", m1, s1, s3d);
  EXPECT_EQ(1, project->fields.size());
  EXPECT_EQ(f1, project->fields.at("f1"));
}

TEST(Field, HDF5) {
  auto filename = "field.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->fields.at("f1");
    EXPECT_EQ("Field \"f1\": Manifold \"m1\" TangentSpace \"s1\" "
              "TensorType \"Vector3D\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(Discretization, create) {
  const auto &m1 = project->manifolds.at("m1");
  EXPECT_TRUE(m1->discretizations.empty());
  auto d1 = m1->createDiscretization("d1");
  EXPECT_EQ(1, m1->discretizations.size());
  EXPECT_EQ(d1, m1->discretizations.at("d1"));
}

TEST(Discretization, HDF5) {
  auto filename = "discretization.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->manifolds.at("m1")->discretizations.at("d1");
    EXPECT_EQ("Discretization \"d1\": Manifold \"m1\"\n", buf.str());
  }
  remove(filename);
}

TEST(DiscretizationBlock, create) {
  const auto &m1 = project->manifolds.at("m1");
  const auto &d1 = m1->discretizations.at("d1");
  EXPECT_TRUE(d1->discretizationblocks.empty());
  auto db1 = d1->createDiscretizationBlock("db1");
  EXPECT_EQ(1, d1->discretizationblocks.size());
  EXPECT_EQ(db1, d1->discretizationblocks.at("db1"));
}

TEST(DiscretizationBlock, HDF5) {
  auto filename = "discretizationblock.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->manifolds.at("m1")->discretizations.at("d1");
    EXPECT_EQ("Discretization \"d1\": Manifold \"m1\"\n"
              "  DiscretizationBlock \"db1\": Discretization \"d1\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(Basis, create) {
  const auto &s1 = project->tangentspaces.at("s1");
  EXPECT_TRUE(s1->bases.empty());
  auto b1 = s1->createBasis("b1");
  EXPECT_EQ(1, s1->bases.size());
  EXPECT_EQ(b1, s1->bases.at("b1"));
}

TEST(Basis, HDF5) {
  auto filename = "basis.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces.at("s1")->bases.at("b1");
    EXPECT_EQ("Basis \"b1\": TangentSpace \"s1\"\n", buf.str());
  }
  remove(filename);
}

TEST(BasisVector, create) {
  const auto &s1 = project->tangentspaces.at("s1");
  const auto &b1 = s1->bases.at("b1");
  EXPECT_TRUE(b1->basisvectors.empty());
  auto bx1 = b1->createBasisVector("x", 0);
  auto by1 = b1->createBasisVector("y", 1);
  auto bz1 = b1->createBasisVector("z", 2);
  EXPECT_EQ(0, bx1->direction);
  EXPECT_EQ(1, by1->direction);
  EXPECT_EQ(2, bz1->direction);
  EXPECT_EQ(3, b1->basisvectors.size());
  EXPECT_EQ(bx1, b1->basisvectors.at("x"));
  EXPECT_EQ(by1, b1->basisvectors.at("y"));
  EXPECT_EQ(bz1, b1->basisvectors.at("z"));
}

TEST(BasisVector, HDF5) {
  auto filename = "basisvector.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces.at("s1")->bases.at("b1");
    EXPECT_EQ("Basis \"b1\": TangentSpace \"s1\"\n"
              "  BasisVector \"x\": Basis \"b1\" direction=0\n"
              "  BasisVector \"y\": Basis \"b1\" direction=1\n"
              "  BasisVector \"z\": Basis \"b1\" direction=2\n",
              buf.str());
  }
  remove(filename);
}

TEST(DiscreteField, create) {
  const auto &f1 = project->fields.at("f1");
  const auto &m1 = f1->manifold;
  const auto &d1 = m1->discretizations.at("d1");
  const auto &s1 = f1->tangentspace;
  const auto &b1 = s1->bases.at("b1");
  EXPECT_TRUE(f1->discretefields.empty());
  auto df1 = f1->createDiscreteField("df1", d1, b1);
  EXPECT_EQ(1, f1->discretefields.size());
  EXPECT_EQ(df1, f1->discretefields.at("df1"));
}

TEST(DiscreteField, HDF5) {
  auto filename = "discretefield.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->fields.at("f1");
    EXPECT_EQ("Field \"f1\": Manifold \"m1\" TangentSpace \"s1\" TensorType "
              "\"Vector3D\"\n"
              "  DiscreteField \"df1\": Field \"f1\" Discretization \"d1\" "
              "Basis \"b1\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(DiscreteFieldBlock, create) {
  const auto &f1 = project->fields.at("f1");
  const auto &df1 = f1->discretefields.at("df1");
  const auto &d1 = df1->discretization;
  const auto &db1 = d1->discretizationblocks.at("db1");
  EXPECT_TRUE(df1->discretefieldblocks.empty());
  auto dfb1 = df1->createDiscreteFieldBlock("dfb1", db1);
  EXPECT_EQ(1, df1->discretefieldblocks.size());
  EXPECT_EQ(dfb1, df1->discretefieldblocks.at("dfb1"));
}

TEST(DiscreteFieldBlock, HDF5) {
  auto filename = "discretefieldblock.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->fields.at("f1")->discretefields.at("df1");
    EXPECT_EQ("DiscreteField \"df1\": Field \"f1\" Discretization \"d1\" Basis "
              "\"b1\"\n"
              "  DiscreteFieldBlock \"dfb1\": DiscreteField \"df1\" "
              "DiscretizationBlock \"db1\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(DiscreteFieldBlockData, create) {
  const auto &f1 = project->fields.at("f1");
  const auto &df1 = f1->discretefields.at("df1");
  const auto &dfb1 = df1->discretefieldblocks.at("dfb1");
  const auto &tt1 = f1->tensortype;
  const auto &bx1 = tt1->tensorcomponents.at("0");
  const auto &by1 = tt1->tensorcomponents.at("1");
  const auto &bz1 = tt1->tensorcomponents.at("2");
  EXPECT_TRUE(dfb1->discretefieldblockdata.empty());
  auto dfbd1 = dfb1->createDiscreteFieldBlockData("dfbd1", bx1);
  auto dfbd2 = dfb1->createDiscreteFieldBlockData("dfbd2", by1);
  auto dfbd3 = dfb1->createDiscreteFieldBlockData("dfbd3", bz1);
  EXPECT_EQ(3, dfb1->discretefieldblockdata.size());
  EXPECT_EQ(dfbd1, dfb1->discretefieldblockdata.at("dfbd1"));
  EXPECT_EQ(dfbd2, dfb1->discretefieldblockdata.at("dfbd2"));
  EXPECT_EQ(dfbd3, dfb1->discretefieldblockdata.at("dfbd3"));
  EXPECT_EQ(DiscreteFieldBlockData::type_empty, dfbd1->data_type);
  EXPECT_EQ(DiscreteFieldBlockData::type_empty, dfbd2->data_type);
  EXPECT_EQ(DiscreteFieldBlockData::type_empty, dfbd3->data_type);
  dfbd2->setData("discretizationfieldblockdata.h5",
                 project->name + "/tensortypes/Scalar3D");
  const auto datatype = H5::getType(0.0);
  const int rank = 1;
  const hsize_t dims[rank] = {10};
  auto dataspace = H5::DataSpace(rank, dims);
  dfbd3->setData(datatype, dataspace);
  EXPECT_EQ(DiscreteFieldBlockData::type_empty, dfbd1->data_type);
  EXPECT_EQ(DiscreteFieldBlockData::type_extlink, dfbd2->data_type);
  EXPECT_EQ(DiscreteFieldBlockData::type_dataset, dfbd3->data_type);
}

TEST(DiscreteFieldBlockData, HDF5) {
  auto filename = "discretizationfieldblockdata.h5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = createProject(file);
    ostringstream buf;
    buf << *p1->fields.at("f1")
                ->discretefields.at("df1")
                ->discretefieldblocks.at("dfb1");
    EXPECT_EQ("DiscreteFieldBlock \"dfb1\": DiscreteField \"df1\" "
              "DiscretizationBlock \"db1\"\n"
              "  DiscreteFieldBlockData \"dfbd1\": DiscreteFieldBlock \"dfb1\" "
              "TensorComponent \"0\"\n"
              "    data: empty\n"
              "  DiscreteFieldBlockData \"dfbd2\": DiscreteFieldBlock \"dfb1\" "
              "TensorComponent \"1\"\n"
              "    data: external link to "
              "\"discretizationfieldblockdata.h5\":\"p1/tensortypes/"
              "Scalar3D\"\n"
              "  DiscreteFieldBlockData \"dfbd3\": DiscreteFieldBlock \"dfb1\" "
              "TensorComponent \"2\"\n"
              "    data: dataset\n",
              buf.str());
  }
  remove(filename);
}

#include "src/gtest_main.cc"
