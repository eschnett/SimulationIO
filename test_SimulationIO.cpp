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
  auto filename = "project.s5";
  string orig;
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
    ostringstream buf;
    buf << *project;
    orig = buf.str();
  }
  shared_ptr<Project> p1;
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    p1 = readProject(file);
    ostringstream buf;
    buf << *p1;
    EXPECT_EQ(orig, buf.str());
  }
  remove(filename);
}

// This test sets up the global variable "project", which is necessary for most
// of the following tests. This must be the last of the "Project" tests.
// TODO: Avoid this dependency.
TEST(Project, setup) {
  project->createStandardTensorTypes();
  EXPECT_EQ(13, project->tensortypes().size());
}

TEST(Parameter, create) {
  EXPECT_TRUE(project->parameters().empty());
  auto par1 = project->createParameter("par1");
  auto par2 = project->createParameter("par2");
  auto par3 = project->createParameter("par3");
  EXPECT_EQ(3, project->parameters().size());
  EXPECT_EQ(par1, project->parameters().at("par1"));
  EXPECT_EQ(par2, project->parameters().at("par2"));
  EXPECT_EQ(par3, project->parameters().at("par3"));
}

TEST(Parameter, HDF5) {
  auto filename = "parameter.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->parameters().at("par1");
    buf << *p1->parameters().at("par2");
    buf << *p1->parameters().at("par3");
    EXPECT_EQ("Parameter \"par1\"\n"
              "Parameter \"par2\"\n"
              "Parameter \"par3\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(ParameterValue, create) {
  auto par1 = project->parameters().at("par1");
  auto par2 = project->parameters().at("par2");
  auto par3 = project->parameters().at("par3");
  EXPECT_TRUE(par1->parametervalues().empty());
  EXPECT_TRUE(par2->parametervalues().empty());
  EXPECT_TRUE(par3->parametervalues().empty());
  auto val1 = par1->createParameterValue("val1");
  auto val2 = par2->createParameterValue("val2");
  auto val3 = par2->createParameterValue("val3");
  auto val4 = par3->createParameterValue("val4");
  auto val5 = par3->createParameterValue("val5");
  val1->setValue(1);
  val2->setValue(2.0);
  val3->setValue(3.0);
  val4->setValue("four");
  string x5(100000, 'x');
  val5->setValue(x5);
  EXPECT_EQ(1, par1->parametervalues().size());
  EXPECT_EQ(2, par2->parametervalues().size());
  EXPECT_EQ(2, par3->parametervalues().size());
  EXPECT_EQ(val1, par1->parametervalues().at("val1"));
  EXPECT_EQ(val2, par2->parametervalues().at("val2"));
  EXPECT_EQ(val3, par2->parametervalues().at("val3"));
  EXPECT_EQ(val4, par3->parametervalues().at("val4"));
  EXPECT_EQ(val5, par3->parametervalues().at("val5"));
}

TEST(ParameterValue, HDF5) {
  auto filename = "parametervalue.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->parameters().at("par1");
    buf << *p1->parameters().at("par2");
    buf << *p1->parameters().at("par3");
    string x5(100000, 'x');
    string expected = "Parameter \"par1\"\n"
                      "  ParameterValue \"val1\": Parameter \"par1\"\n"
                      "    value: int(1)\n"
                      "Parameter \"par2\"\n"
                      "  ParameterValue \"val2\": Parameter \"par2\"\n"
                      "    value: double(2)\n"
                      "  ParameterValue \"val3\": Parameter \"par2\"\n"
                      "    value: double(3)\n"
                      "Parameter \"par3\"\n"
                      "  ParameterValue \"val4\": Parameter \"par3\"\n"
                      "    value: string(\"four\")\n"
                      "  ParameterValue \"val5\": Parameter \"par3\"\n"
                      "    value: string(\"" +
                      x5 + "\")\n";
    EXPECT_EQ(expected, buf.str());
  }
  remove(filename);
}

TEST(Configuration, create) {
  EXPECT_TRUE(project->configurations().empty());
  auto conf1 = project->createConfiguration("conf1");
  auto conf2 = project->createConfiguration("conf2");
  auto par1 = project->parameters().at("par1");
  auto par2 = project->parameters().at("par2");
  auto val1 = par1->parametervalues().at("val1");
  auto val2 = par2->parametervalues().at("val2");
  EXPECT_EQ(2, project->configurations().size());
  EXPECT_EQ(conf1, project->configurations().at("conf1"));
  EXPECT_EQ(conf2, project->configurations().at("conf2"));
  EXPECT_TRUE(conf1->parametervalues().empty());
  EXPECT_TRUE(conf2->parametervalues().empty());
  conf2->insertParameterValue(val1);
  conf2->insertParameterValue(val2);
  EXPECT_TRUE(conf1->parametervalues().empty());
  EXPECT_EQ(2, conf2->parametervalues().size());
  EXPECT_EQ(val1, conf2->parametervalues().at("val1"));
  EXPECT_EQ(val2, conf2->parametervalues().at("val2"));
}

TEST(Configuration, HDF5) {
  auto filename = "configuration.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->configurations().at("conf1");
    buf << *p1->configurations().at("conf2");
    EXPECT_EQ("Configuration \"conf1\"\n"
              "Configuration \"conf2\"\n"
              "  Parameter \"par1\" ParameterValue \"val1\"\n"
              "  Parameter \"par2\" ParameterValue \"val2\"\n",
              buf.str());
  }
  remove(filename);
}

TEST(TensorTypes, Scalar3D) {
  auto tt = project->tensortypes().at("Scalar3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension());
  EXPECT_EQ(0, tt->rank());
  EXPECT_EQ(1, tt->tensorcomponents().size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents())
    EXPECT_TRUE(tc.second->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"Scalar3D\": dim=3 rank=0\n"
            "  TensorComponent \"scalar\": TensorType \"Scalar3D\" "
            "storage_index=0 indexvalues=[]\n",
            buf.str());
}

TEST(TensorTypes, Vector3D) {
  auto tt = project->tensortypes().at("Vector3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension());
  EXPECT_EQ(1, tt->rank());
  EXPECT_EQ(3, tt->tensorcomponents().size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents())
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
  auto tt = project->tensortypes().at("SymmetricTensor3D");
  EXPECT_TRUE(bool(tt));
  EXPECT_EQ(3, tt->dimension());
  EXPECT_EQ(2, tt->rank());
  EXPECT_EQ(6, tt->tensorcomponents().size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->tensorcomponents())
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
  auto filename = "tensortypes.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    for (const auto &tt : p1->tensortypes())
      buf << *tt.second;
    EXPECT_EQ("TensorType \"Scalar0D\": dim=0 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar0D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"Scalar1D\": dim=1 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar1D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"Scalar2D\": dim=2 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar2D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"Scalar3D\": dim=3 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar3D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"Scalar4D\": dim=4 rank=0\n"
              "  TensorComponent \"scalar\": TensorType \"Scalar4D\" "
              "storage_index=0 indexvalues=[]\n"
              "TensorType \"SymmetricTensor2D\": dim=2 rank=2\n"
              "  TensorComponent \"00\": TensorType \"SymmetricTensor2D\" "
              "storage_index=0 indexvalues=[0,0]\n"
              "  TensorComponent \"01\": TensorType \"SymmetricTensor2D\" "
              "storage_index=1 indexvalues=[0,1]\n"
              "  TensorComponent \"11\": TensorType \"SymmetricTensor2D\" "
              "storage_index=2 indexvalues=[1,1]\n"
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
              "TensorType \"SymmetricTensor4D\": dim=4 rank=2\n"
              "  TensorComponent \"00\": TensorType \"SymmetricTensor4D\" "
              "storage_index=0 indexvalues=[0,0]\n"
              "  TensorComponent \"01\": TensorType \"SymmetricTensor4D\" "
              "storage_index=1 indexvalues=[0,1]\n"
              "  TensorComponent \"02\": TensorType \"SymmetricTensor4D\" "
              "storage_index=2 indexvalues=[0,2]\n"
              "  TensorComponent \"03\": TensorType \"SymmetricTensor4D\" "
              "storage_index=3 indexvalues=[0,3]\n"
              "  TensorComponent \"11\": TensorType \"SymmetricTensor4D\" "
              "storage_index=4 indexvalues=[1,1]\n"
              "  TensorComponent \"12\": TensorType \"SymmetricTensor4D\" "
              "storage_index=5 indexvalues=[1,2]\n"
              "  TensorComponent \"13\": TensorType \"SymmetricTensor4D\" "
              "storage_index=6 indexvalues=[1,3]\n"
              "  TensorComponent \"22\": TensorType \"SymmetricTensor4D\" "
              "storage_index=7 indexvalues=[2,2]\n"
              "  TensorComponent \"23\": TensorType \"SymmetricTensor4D\" "
              "storage_index=8 indexvalues=[2,3]\n"
              "  TensorComponent \"33\": TensorType \"SymmetricTensor4D\" "
              "storage_index=9 indexvalues=[3,3]\n"
              "TensorType \"Vector0D\": dim=0 rank=1\n"
              "TensorType \"Vector1D\": dim=1 rank=1\n"
              "  TensorComponent \"0\": TensorType \"Vector1D\" "
              "storage_index=0 indexvalues=[0]\n"
              "TensorType \"Vector2D\": dim=2 rank=1\n"
              "  TensorComponent \"0\": TensorType \"Vector2D\" "
              "storage_index=0 indexvalues=[0]\n"
              "  TensorComponent \"1\": TensorType \"Vector2D\" "
              "storage_index=1 indexvalues=[1]\n"
              "TensorType \"Vector3D\": dim=3 rank=1\n"
              "  TensorComponent \"0\": TensorType \"Vector3D\" "
              "storage_index=0 indexvalues=[0]\n"
              "  TensorComponent \"1\": TensorType \"Vector3D\" "
              "storage_index=1 indexvalues=[1]\n"
              "  TensorComponent \"2\": TensorType \"Vector3D\" "
              "storage_index=2 indexvalues=[2]\n"
              "TensorType \"Vector4D\": dim=4 rank=1\n"
              "  TensorComponent \"0\": TensorType \"Vector4D\" "
              "storage_index=0 indexvalues=[0]\n"
              "  TensorComponent \"1\": TensorType \"Vector4D\" "
              "storage_index=1 indexvalues=[1]\n"
              "  TensorComponent \"2\": TensorType \"Vector4D\" "
              "storage_index=2 indexvalues=[2]\n"
              "  TensorComponent \"3\": TensorType \"Vector4D\" "
              "storage_index=3 indexvalues=[3]\n",
              buf.str());
  }
  remove(filename);
}

TEST(Manifold, create) {
  EXPECT_TRUE(project->manifolds().empty());
  auto conf1 = project->configurations().at("conf1");
  auto m1 = project->createManifold("m1", conf1, 3);
  EXPECT_EQ(1, project->manifolds().size());
  EXPECT_EQ(m1, project->manifolds().at("m1"));
  auto m0 = project->createManifold("m0", conf1, 0);
  EXPECT_EQ(2, project->manifolds().size());
  EXPECT_EQ(m0, project->manifolds().at("m0"));
}

TEST(Manifold, HDF5) {
  auto filename = "manifold.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->manifolds().at("m1");
    EXPECT_EQ("Manifold \"m1\": Configuration \"conf1\" dim=3\n", buf.str());
    ostringstream buf0;
    buf0 << *p1->manifolds().at("m0");
    EXPECT_EQ("Manifold \"m0\": Configuration \"conf1\" dim=0\n", buf0.str());
  }
  remove(filename);
}

TEST(TangentSpace, create) {
  EXPECT_TRUE(project->tangentspaces().empty());
  auto conf1 = project->configurations().at("conf1");
  auto s1 = project->createTangentSpace("s1", conf1, 3);
  EXPECT_EQ(1, project->tangentspaces().size());
  EXPECT_EQ(s1, project->tangentspaces().at("s1"));
  auto s0 = project->createTangentSpace("s0", conf1, 0);
  EXPECT_EQ(2, project->tangentspaces().size());
  EXPECT_EQ(s0, project->tangentspaces().at("s0"));
}

TEST(TangentSpace, HDF5) {
  auto filename = "tangentspace.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces().at("s1");
    EXPECT_EQ("TangentSpace \"s1\": Configuration \"conf1\" dim=3\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->tangentspaces().at("s0");
    EXPECT_EQ("TangentSpace \"s0\": Configuration \"conf1\" dim=0\n",
              buf0.str());
  }
  remove(filename);
}

TEST(Field, create) {
  EXPECT_TRUE(project->fields().empty());
  auto conf1 = project->configurations().at("conf1");
  auto m1 = project->manifolds().at("m1");
  auto s1 = project->tangentspaces().at("s1");
  auto s3d = project->tensortypes().at("SymmetricTensor3D");
  auto f1 = project->createField("f1", conf1, m1, s1, s3d);
  EXPECT_EQ(1, project->fields().size());
  EXPECT_EQ(&*f1, &*project->fields().at("f1"));
  auto m0 = project->manifolds().at("m0");
  auto s0 = project->tangentspaces().at("s0");
  auto s0d = project->tensortypes().at("Scalar0D");
  auto f0 = project->createField("f0", conf1, m0, s0, s0d);
  EXPECT_EQ(2, project->fields().size());
  EXPECT_EQ(&*f0, &*project->fields().at("f0"));
}

TEST(Field, HDF5) {
  auto filename = "field.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->fields().at("f1");
    EXPECT_EQ("Field \"f1\": Configuration \"conf1\" Manifold \"m1\" "
              "TangentSpace \"s1\" TensorType \"SymmetricTensor3D\"\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->fields().at("f0");
    EXPECT_EQ("Field \"f0\": Configuration \"conf1\" Manifold \"m0\" "
              "TangentSpace \"s0\" TensorType \"Scalar0D\"\n",
              buf0.str());
  }
  remove(filename);
}

TEST(CoordinateSystem, create) {
  EXPECT_TRUE(project->coordinatesystems().empty());
  auto conf1 = project->configurations().at("conf1");
  auto m1 = project->manifolds().at("m1");
  auto cs1 = project->createCoordinateSystem("cs1", conf1, m1);
  EXPECT_EQ(1, project->coordinatesystems().size());
  EXPECT_EQ(cs1, project->coordinatesystems().at("cs1"));
}

TEST(CoordinateSystem, HDF5) {
  auto filename = "coordinatesystem.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->coordinatesystems().at("cs1");
    EXPECT_EQ(
        "CoordinateSystem \"cs1\": Configuration \"conf1\" Manifold \"m1\"\n",
        buf.str());
  }
  remove(filename);
}

TEST(Discretization, create) {
  auto m1 = project->manifolds().at("m1");
  EXPECT_TRUE(m1->discretizations().empty());
  auto conf1 = project->configurations().at("conf1");
  auto d1 = m1->createDiscretization("d1", conf1);
  auto d2 = m1->createDiscretization("d2", conf1);
  EXPECT_EQ(2, m1->discretizations().size());
  EXPECT_EQ(d1, m1->discretizations().at("d1"));
  EXPECT_EQ(d2, m1->discretizations().at("d2"));
  auto m0 = project->manifolds().at("m0");
  EXPECT_TRUE(m0->discretizations().empty());
  auto d0 = m0->createDiscretization("d0", conf1);
  EXPECT_EQ(1, m0->discretizations().size());
  EXPECT_EQ(d0, m0->discretizations().at("d0"));
}

TEST(Discretization, HDF5) {
  auto filename = "discretization.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->manifolds().at("m1")->discretizations().at("d1");
    buf << *p1->manifolds().at("m1")->discretizations().at("d2");
    EXPECT_EQ(
        "Discretization \"d1\": Configuration \"conf1\" Manifold \"m1\"\n"
        "Discretization \"d2\": Configuration \"conf1\" Manifold \"m1\"\n",
        buf.str());
    ostringstream buf0;
    buf0 << *p1->manifolds().at("m0")->discretizations().at("d0");
    EXPECT_EQ(
        "Discretization \"d0\": Configuration \"conf1\" Manifold \"m0\"\n",
        buf0.str());
  }
  remove(filename);
}

TEST(SubDiscretization, create) {
  auto m1 = project->manifolds().at("m1");
  auto d1 = m1->discretizations().at("d1");
  auto d2 = m1->discretizations().at("d2");
  EXPECT_TRUE(m1->subdiscretizations().empty());
  vector<double> factor(m1->dimension(), 1.0), offset(m1->dimension(), 0.5);
  auto sd1 = m1->createSubDiscretization("sd1", d1, d2, factor, offset);
  EXPECT_EQ(1, m1->subdiscretizations().size());
  EXPECT_EQ(sd1, m1->subdiscretizations().at("sd1"));
}

TEST(SubDiscretization, HDF5) {
  auto filename = "subdiscretization.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->manifolds().at("m1")->subdiscretizations().at("sd1");
    EXPECT_EQ("SubDiscretization \"sd1\": Manifold \"m1\" parent "
              "Discretization \"d1\" child Discretization \"d2\" "
              "factor=[1,1,1] offset=[0.5,0.5,0.5]\n",
              buf.str());
  }
  remove(filename);
}

TEST(DiscretizationBlock, create) {
  auto m1 = project->manifolds().at("m1");
  auto d1 = m1->discretizations().at("d1");
  EXPECT_TRUE(d1->discretizationblocks().empty());
  auto db1 = d1->createDiscretizationBlock("db1");
  vector<long long> offset = {3, 3, 3};
  vector<long long> shape = {9, 10, 11};
  db1->setBox(box_t(offset, shape));
  EXPECT_EQ(1, d1->discretizationblocks().size());
  EXPECT_EQ(db1, d1->discretizationblocks().at("db1"));
  auto m0 = project->manifolds().at("m0");
  auto d0 = m0->discretizations().at("d0");
  EXPECT_TRUE(d0->discretizationblocks().empty());
  auto db0 = d0->createDiscretizationBlock("db0");
  vector<long long> offset0 = {};
  vector<long long> shape0 = {};
  db0->setBox(box_t(offset0, shape0));
  EXPECT_EQ(1, d0->discretizationblocks().size());
  EXPECT_EQ(db0, d0->discretizationblocks().at("db0"));
}

TEST(DiscretizationBlock, HDF5) {
  auto filename = "discretizationblock.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->manifolds().at("m1")->discretizations().at("d1");
    EXPECT_EQ("Discretization \"d1\": Configuration \"conf1\" Manifold \"m1\"\n"
              "  DiscretizationBlock \"db1\": Discretization \"d1\" "
              "box=([3,3,3]:[9,10,11])\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->manifolds().at("m0")->discretizations().at("d0");
    EXPECT_EQ("Discretization \"d0\": Configuration \"conf1\" Manifold \"m0\"\n"
              "  DiscretizationBlock \"db0\": Discretization \"d0\" box=(1)\n",
              buf0.str());
  }
  remove(filename);
}

TEST(Basis, create) {
  auto conf1 = project->configurations().at("conf1");
  auto s1 = project->tangentspaces().at("s1");
  EXPECT_TRUE(s1->bases().empty());
  auto b1 = s1->createBasis("b1", conf1);
  EXPECT_EQ(1, s1->bases().size());
  EXPECT_EQ(b1, s1->bases().at("b1"));
  auto s0 = project->tangentspaces().at("s0");
  EXPECT_TRUE(s0->bases().empty());
  auto b0 = s0->createBasis("b0", conf1);
  EXPECT_EQ(1, s0->bases().size());
  EXPECT_EQ(b0, s0->bases().at("b0"));
}

TEST(Basis, HDF5) {
  auto filename = "basis.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces().at("s1")->bases().at("b1");
    EXPECT_EQ("Basis \"b1\": Configuration \"conf1\" TangentSpace \"s1\"\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->tangentspaces().at("s0")->bases().at("b0");
    EXPECT_EQ("Basis \"b0\": Configuration \"conf1\" TangentSpace \"s0\"\n",
              buf0.str());
  }
  remove(filename);
}

TEST(BasisVector, create) {
  auto s1 = project->tangentspaces().at("s1");
  auto b1 = s1->bases().at("b1");
  EXPECT_TRUE(b1->basisvectors().empty());
  auto bx1 = b1->createBasisVector("x", 0);
  auto by1 = b1->createBasisVector("y", 1);
  auto bz1 = b1->createBasisVector("z", 2);
  EXPECT_EQ(0, bx1->direction());
  EXPECT_EQ(1, by1->direction());
  EXPECT_EQ(2, bz1->direction());
  EXPECT_EQ(3, b1->basisvectors().size());
  EXPECT_EQ(bx1, b1->basisvectors().at("x"));
  EXPECT_EQ(by1, b1->basisvectors().at("y"));
  EXPECT_EQ(bz1, b1->basisvectors().at("z"));
}

TEST(BasisVector, HDF5) {
  auto filename = "basisvector.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->tangentspaces().at("s1")->bases().at("b1");
    EXPECT_EQ("Basis \"b1\": Configuration \"conf1\" TangentSpace \"s1\"\n"
              "  BasisVector \"x\": Basis \"b1\" direction=0\n"
              "  BasisVector \"y\": Basis \"b1\" direction=1\n"
              "  BasisVector \"z\": Basis \"b1\" direction=2\n",
              buf.str());
  }
  remove(filename);
}

TEST(DiscreteField, create) {
  auto f1 = project->fields().at("f1");
  auto conf1 = project->configurations().at("conf1");
  auto m1 = f1->manifold();
  auto d1 = m1->discretizations().at("d1");
  auto s1 = f1->tangentspace();
  auto b1 = s1->bases().at("b1");
  EXPECT_TRUE(f1->discretefields().empty());
  auto df1 = f1->createDiscreteField("df1", conf1, d1, b1);
  EXPECT_EQ(1, f1->discretefields().size());
  EXPECT_EQ(df1, f1->discretefields().at("df1"));
  auto f0 = project->fields().at("f0");
  auto m0 = f0->manifold();
  auto d0 = m0->discretizations().at("d0");
  auto s0 = f0->tangentspace();
  auto b0 = s0->bases().at("b0");
  EXPECT_TRUE(f0->discretefields().empty());
  auto df0 = f0->createDiscreteField("df0", conf1, d0, b0);
  EXPECT_EQ(1, f0->discretefields().size());
  EXPECT_EQ(df0, f0->discretefields().at("df0"));
}

TEST(DiscreteField, HDF5) {
  auto filename = "discretefield.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->fields().at("f1");
    EXPECT_EQ("Field \"f1\": Configuration \"conf1\" Manifold \"m1\" "
              "TangentSpace \"s1\" TensorType \"SymmetricTensor3D\"\n"
              "  DiscreteField \"df1\": Configuration \"conf1\" Field \"f1\" "
              "Discretization \"d1\" Basis \"b1\"\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->fields().at("f0");
    EXPECT_EQ("Field \"f0\": Configuration \"conf1\" Manifold \"m0\" "
              "TangentSpace \"s0\" TensorType \"Scalar0D\"\n"
              "  DiscreteField \"df0\": Configuration \"conf1\" Field \"f0\" "
              "Discretization \"d0\" Basis \"b0\"\n",
              buf0.str());
  }
  remove(filename);
}

TEST(CoordinateField, create) {
  auto cs1 = project->coordinatesystems().at("cs1");
  auto f1 = project->fields().at("f1");
  EXPECT_TRUE(cs1->coordinatefields().empty());
  auto csf1 = cs1->createCoordinateField("csf1", 0, f1);
  EXPECT_EQ(1, cs1->coordinatefields().size());
  EXPECT_EQ(csf1, cs1->coordinatefields().at("csf1"));
}

TEST(CoordinateField, HDF5) {
  auto filename = "coordinatefield.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->coordinatesystems().at("cs1")->coordinatefields().at("csf1");
    EXPECT_EQ("CoordinateField \"csf1\": CoordinateSystem \"cs1\" "
              "direction=0 Field "
              "\"f1\"\n",

              buf.str());
  }
  remove(filename);
}

TEST(DiscreteFieldBlock, create) {
  auto f1 = project->fields().at("f1");
  auto df1 = f1->discretefields().at("df1");
  auto d1 = df1->discretization();
  auto db1 = d1->discretizationblocks().at("db1");
  EXPECT_TRUE(df1->discretefieldblocks().empty());
  auto dfb1 = df1->createDiscreteFieldBlock("dfb1", db1);
  EXPECT_EQ(1, df1->discretefieldblocks().size());
  EXPECT_EQ(dfb1, df1->discretefieldblocks().at("dfb1"));
  auto f0 = project->fields().at("f0");
  auto df0 = f0->discretefields().at("df0");
  auto d0 = df0->discretization();
  auto db0 = d0->discretizationblocks().at("db0");
  EXPECT_TRUE(df0->discretefieldblocks().empty());
  auto dfb0 = df0->createDiscreteFieldBlock("dfb0", db0);
  EXPECT_EQ(1, df0->discretefieldblocks().size());
  EXPECT_EQ(dfb0, df0->discretefieldblocks().at("dfb0"));
}

TEST(DiscreteFieldBlock, HDF5) {
  auto filename = "discretefieldblock.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->fields().at("f1")->discretefields().at("df1");
    EXPECT_EQ("DiscreteField \"df1\": Configuration \"conf1\" Field \"f1\" "
              "Discretization \"d1\" Basis \"b1\"\n"
              "  DiscreteFieldBlock \"dfb1\": DiscreteField \"df1\" "
              "DiscretizationBlock \"db1\"\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->fields().at("f0")->discretefields().at("df0");
    EXPECT_EQ("DiscreteField \"df0\": Configuration \"conf1\" Field \"f0\" "
              "Discretization \"d0\" Basis \"b0\"\n"
              "  DiscreteFieldBlock \"dfb0\": DiscreteField \"df0\" "
              "DiscretizationBlock \"db0\"\n",
              buf0.str());
  }
  remove(filename);
}

TEST(DiscreteFieldBlockComponent, create) {
  auto f1 = project->fields().at("f1");
  auto df1 = f1->discretefields().at("df1");
  auto dfb1 = df1->discretefieldblocks().at("dfb1");
  auto tt1 = f1->tensortype();
  auto bxx1 = tt1->tensorcomponents().at("00");
  auto bxy1 = tt1->tensorcomponents().at("01");
  auto bxz1 = tt1->tensorcomponents().at("02");
  auto byy1 = tt1->tensorcomponents().at("11");
  auto byz1 = tt1->tensorcomponents().at("12");
  EXPECT_TRUE(dfb1->discretefieldblockcomponents().empty());
  auto dfbd1 = dfb1->createDiscreteFieldBlockComponent("dfbd1", bxx1);
  auto dfbd2 = dfb1->createDiscreteFieldBlockComponent("dfbd2", bxy1);
  auto dfbd3 = dfb1->createDiscreteFieldBlockComponent("dfbd3", bxz1);
  auto dfbd4 = dfb1->createDiscreteFieldBlockComponent("dfbd4", byy1);
  EXPECT_EQ(4, dfb1->discretefieldblockcomponents().size());
  EXPECT_EQ(dfbd1, dfb1->discretefieldblockcomponents().at("dfbd1"));
  EXPECT_EQ(dfbd2, dfb1->discretefieldblockcomponents().at("dfbd2"));
  EXPECT_EQ(dfbd3, dfb1->discretefieldblockcomponents().at("dfbd3"));
  EXPECT_EQ(dfbd4, dfb1->discretefieldblockcomponents().at("dfbd4"));
  EXPECT_EQ(dfbd1, dfb1->storage_indices().at(bxx1->storage_index()));
  EXPECT_EQ(dfbd2, dfb1->storage_indices().at(bxy1->storage_index()));
  EXPECT_EQ(dfbd3, dfb1->storage_indices().at(bxz1->storage_index()));
  EXPECT_EQ(dfbd4, dfb1->storage_indices().at(byy1->storage_index()));
  EXPECT_EQ(dfb1->storage_indices().end(),
            dfb1->storage_indices().find(byz1->storage_index()));
  EXPECT_FALSE(bool(dfbd1->datablock()));
  EXPECT_FALSE(bool(dfbd2->datablock()));
  EXPECT_FALSE(bool(dfbd3->datablock()));
  EXPECT_FALSE(bool(dfbd4->datablock()));
  dfbd1->createExtLink("discretizationfieldblockcomponent.s5",
                       project->name() + "/tensortypes/Scalar3D");
  dfbd1->unsetDataBlock();
  dfbd2->createExtLink("discretizationfieldblockcomponent.s5",
                       project->name() + "/tensortypes/Scalar3D");
  // TODO: get rank and shape from manifold and discretization block
  const int rank = 3;
  const hsize_t dims[rank] = {11, 10, 9};
  dfbd3->createDataSet<double>();
  double origin = -1.0;
  vector<double> delta(rank);
  for (int d = 0; d < rank; ++d)
    delta.at(d) = 2.0 / dims[rank - 1 - d];
  dfbd4->createDataRange(origin, delta);
  EXPECT_FALSE(bool(dfbd1->datablock()));
  EXPECT_TRUE(bool(dfbd2->extlink()));
  EXPECT_TRUE(bool(dfbd3->dataset()));
  EXPECT_TRUE(bool(dfbd4->datarange()));

  auto f0 = project->fields().at("f0");
  auto df0 = f0->discretefields().at("df0");
  auto dfb0 = df0->discretefieldblocks().at("dfb0");
  auto tt0 = f0->tensortype();
  auto b0 = tt0->tensorcomponents().at("scalar");
  EXPECT_TRUE(dfb0->discretefieldblockcomponents().empty());
  auto dfbd0 = dfb0->createDiscreteFieldBlockComponent("dfbd0", b0);
  EXPECT_EQ(1, dfb0->discretefieldblockcomponents().size());
  EXPECT_EQ(dfbd0, dfb0->discretefieldblockcomponents().at("dfbd0"));
  EXPECT_EQ(dfbd0, dfb0->storage_indices().at(b0->storage_index()));
  EXPECT_FALSE(bool(dfbd0->datablock()));
  // TODO: get rank and shape from manifold and discretization block
  const int rank0 = 0;
  double origin0 = -1.0;
  vector<double> delta0(rank0);
  dfbd0->createDataRange(origin0, delta0);
  EXPECT_TRUE(bool(dfbd0->datarange()));
  dfbd0->unsetDataBlock();
  EXPECT_FALSE(bool(dfbd0->datarange()));
  dfbd0->createDataSet<double>();
  EXPECT_TRUE(bool(dfbd0->dataset()));
}

TEST(DiscreteFieldBlockComponent, HDF5) {
  auto filename = "discretizationfieldblockcomponent.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto p1 = readProject(file);
    ostringstream buf;
    buf << *p1->fields()
                .at("f1")
                ->discretefields()
                .at("df1")
                ->discretefieldblocks()
                .at("dfb1");
    EXPECT_EQ("DiscreteFieldBlock \"dfb1\": DiscreteField \"df1\" "
              "DiscretizationBlock \"db1\"\n"
              "  DiscreteFieldBlockComponent \"dfbd1\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"00\"\n"
              "  DiscreteFieldBlockComponent \"dfbd2\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"01\"\n"
              "    CopyObj: ???:\"data\"\n"
              "  DiscreteFieldBlockComponent \"dfbd3\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"02\"\n"
              "    CopyObj: ???:\"data\"\n"
              "  DiscreteFieldBlockComponent \"dfbd4\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"11\"\n"
              "    DataRange: origin=-1 delta=[0.222222,0.2,0.181818]\n",
              buf.str());
    ostringstream buf0;
    buf0 << *p1->fields()
                 .at("f0")
                 ->discretefields()
                 .at("df0")
                 ->discretefieldblocks()
                 .at("dfb0");
    EXPECT_EQ("DiscreteFieldBlock \"dfb0\": DiscreteField \"df0\" "
              "DiscretizationBlock \"db0\"\n"
              "  DiscreteFieldBlockComponent \"dfbd0\": DiscreteFieldBlock "
              "\"dfb0\" TensorComponent \"scalar\"\n"
              "    CopyObj: ???:\"data\"\n",
              buf0.str());
  }
  remove(filename);
}

TEST(ProjectMerge, merge) {
  // Clean up project to make merging possible
  auto f0 = project->fields().at("f0");
  auto df0 = f0->discretefields().at("df0");
  auto dfb0 = df0->discretefieldblocks().at("dfb0");
  auto dfbd0 = dfb0->discretefieldblockcomponents().at("dfbd0");
  dfbd0->unsetDataBlock();
  auto f1 = project->fields().at("f1");
  auto df1 = f1->discretefields().at("df1");
  auto dfb1 = df1->discretefieldblocks().at("dfb1");
  auto dfbd1 = dfb1->discretefieldblockcomponents().at("dfbd1");
  dfbd1->unsetDataBlock();
  auto dfbd2 = dfb1->discretefieldblockcomponents().at("dfbd2");
  dfbd2->unsetDataBlock();
  auto dfbd3 = dfb1->discretefieldblockcomponents().at("dfbd3");
  dfbd3->unsetDataBlock();
  auto dfbd4 = dfb1->discretefieldblockcomponents().at("dfbd4");
  dfbd4->unsetDataBlock();

  auto filename = "project.s5";
  string orig = [&] {
    ostringstream buf;
    buf << *project;
    return buf.str();
  }();
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
  }
  auto project2 = [&] {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    return readProject(file);
  }();
  project2->merge(project);
  string curr = [&] {
    ostringstream buf;
    buf << *project2;
    return buf.str();
  }();
  EXPECT_EQ(orig, curr);
  remove(filename);
}

#include "src/gtest_main.cc"
