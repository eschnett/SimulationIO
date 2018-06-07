#include "Config.hpp"
#include "SimulationIO.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#include <gtest/gtest.h>

#include <array>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>

using std::array;
using std::complex;
using std::ifstream;
using std::int64_t;
using std::ios;
using std::numeric_limits;
using std::ofstream;
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

#ifdef SIMULATIONIO_HAVE_HDF5
TEST(HDF5, types) {
  auto filename = "types.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    createAttribute(file, "char", numeric_limits<char>::max());
    createAttribute(file, "signed char", numeric_limits<signed char>::max());
    createAttribute(file, "unsigned char",
                    numeric_limits<unsigned char>::max());
    createAttribute(file, "short", numeric_limits<short>::max());
    createAttribute(file, "unsigned short",
                    numeric_limits<unsigned short>::max());
    createAttribute(file, "int", numeric_limits<int>::max());
    createAttribute(file, "unsigned int", numeric_limits<unsigned int>::max());
    createAttribute(file, "long", numeric_limits<long>::max());
    createAttribute(file, "unsigned long",
                    numeric_limits<unsigned long>::max());
    createAttribute(file, "long long", numeric_limits<long long>::max());
    createAttribute(file, "unsigned long long",
                    numeric_limits<unsigned long long>::max());
    createAttribute(file, "float", numeric_limits<float>::min());
    createAttribute(file, "double", numeric_limits<double>::min());
    createAttribute(file, "long double", numeric_limits<long double>::min());
    createAttribute(file, "complex<signed char>",
                    complex<signed char>(numeric_limits<signed char>::min(),
                                         numeric_limits<signed char>::max()));
    createAttribute(file, "complex<short>",
                    complex<short>(numeric_limits<short>::min(),
                                   numeric_limits<short>::max()));
    createAttribute(
        file, "complex<int>",
        complex<int>(numeric_limits<int>::min(), numeric_limits<int>::max()));
    createAttribute(file, "complex<long>",
                    complex<long>(numeric_limits<long>::min(),
                                  numeric_limits<long>::max()));
    createAttribute(file, "complex<long long>",
                    complex<long long>(numeric_limits<long long>::min(),
                                       numeric_limits<long long>::max()));
    createAttribute(file, "complex<float>",
                    complex<float>(numeric_limits<float>::min(),
                                   numeric_limits<float>::max()));
    createAttribute(file, "complex<double>",
                    complex<double>(numeric_limits<double>::min(),
                                    numeric_limits<double>::max()));
    createAttribute(file, "complex<long double>",
                    complex<long double>(numeric_limits<long double>::min(),
                                         numeric_limits<long double>::max()));
  }
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    EXPECT_EQ(numeric_limits<char>::max(),
              H5::getAttribute<char>(file, "char"));
    EXPECT_EQ(numeric_limits<signed char>::max(),
              H5::getAttribute<signed char>(file, "signed char"));
    EXPECT_EQ(numeric_limits<unsigned char>::max(),
              H5::getAttribute<unsigned char>(file, "unsigned char"));
    EXPECT_EQ(numeric_limits<short>::max(),
              H5::getAttribute<short>(file, "short"));
    EXPECT_EQ(numeric_limits<unsigned short>::max(),
              H5::getAttribute<unsigned short>(file, "unsigned short"));
    EXPECT_EQ(numeric_limits<int>::max(), H5::getAttribute<int>(file, "int"));
    EXPECT_EQ(numeric_limits<unsigned int>::max(),
              H5::getAttribute<unsigned int>(file, "unsigned int"));
    EXPECT_EQ(numeric_limits<long>::max(),
              H5::getAttribute<long>(file, "long"));
    EXPECT_EQ(numeric_limits<unsigned long>::max(),
              H5::getAttribute<unsigned long>(file, "unsigned long"));
    EXPECT_EQ(numeric_limits<long long>::max(),
              H5::getAttribute<long long>(file, "long long"));
    EXPECT_EQ(numeric_limits<unsigned long long>::max(),
              H5::getAttribute<unsigned long long>(file, "unsigned long long"));
    EXPECT_EQ(numeric_limits<float>::min(),
              H5::getAttribute<float>(file, "float"));
    EXPECT_EQ(numeric_limits<double>::min(),
              H5::getAttribute<double>(file, "double"));
    EXPECT_EQ(numeric_limits<long double>::min(),
              H5::getAttribute<long double>(file, "long double"));
    EXPECT_EQ(
        complex<signed char>(numeric_limits<signed char>::min(),
                             numeric_limits<signed char>::max()),
        H5::getAttribute<complex<signed char>>(file, "complex<signed char>"));
    EXPECT_EQ(complex<short>(numeric_limits<short>::min(),
                             numeric_limits<short>::max()),
              H5::getAttribute<complex<short>>(file, "complex<short>"));
    EXPECT_EQ(
        complex<int>(numeric_limits<int>::min(), numeric_limits<int>::max()),
        H5::getAttribute<complex<int>>(file, "complex<int>"));
    EXPECT_EQ(
        complex<long>(numeric_limits<long>::min(), numeric_limits<long>::max()),
        H5::getAttribute<complex<long>>(file, "complex<long>"));
    EXPECT_EQ(complex<long long>(numeric_limits<long long>::min(),
                                 numeric_limits<long long>::max()),
              H5::getAttribute<complex<long long>>(file, "complex<long long>"));
    EXPECT_EQ(complex<float>(numeric_limits<float>::min(),
                             numeric_limits<float>::max()),
              H5::getAttribute<complex<float>>(file, "complex<float>"));
    EXPECT_EQ(complex<double>(numeric_limits<double>::min(),
                              numeric_limits<double>::max()),
              H5::getAttribute<complex<double>>(file, "complex<double>"));
    EXPECT_EQ(
        complex<long double>(numeric_limits<long double>::min(),
                             numeric_limits<long double>::max()),
        H5::getAttribute<complex<long double>>(file, "complex<long double>"));
  }
  remove(filename);
}
#endif

shared_ptr<Project> project;

TEST(Project, create) {
  project = createProject("p1");
  EXPECT_TRUE(project->invariant());
  ostringstream buf;
  buf << *project;
  EXPECT_EQ("Project \"p1\"\n", buf.str());
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Project, ASDF) {
  auto filename = "project.asdf";
  string orig;
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
    ostringstream buf;
    buf << *project;
    orig = buf.str();
  }
  shared_ptr<Project> p1;
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    p1 = readProjectASDF(pfile);
    ostringstream buf;
    buf << *p1;
    EXPECT_EQ(orig, buf.str());
  }
  remove(filename);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Parameter, ASDF) {
  auto filename = "parameter.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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
  EXPECT_EQ(ParameterValue::type_int, val1->getValueType());
  EXPECT_EQ(ParameterValue::type_double, val2->getValueType());
  EXPECT_EQ(ParameterValue::type_double, val3->getValueType());
  EXPECT_EQ(ParameterValue::type_string, val4->getValueType());
  EXPECT_EQ(ParameterValue::type_string, val5->getValueType());
  EXPECT_EQ(1, val1->getValueInt());
  EXPECT_EQ(2.0, val2->getValueDouble());
  EXPECT_EQ(3.0, val3->getValueDouble());
  EXPECT_EQ("four", val4->getValueString());
  EXPECT_EQ(x5, val5->getValueString());
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(ParameterValue, ASDF) {
  auto filename = "parametervalue.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Configuration, ASDF) {
  auto filename = "configuration.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(TensorTypes, ASDF) {
  auto filename = "tensortypes.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Manifold, ASDF) {
  auto filename = "manifold.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
    ostringstream buf;
    buf << *p1->manifolds().at("m1");
    EXPECT_EQ("Manifold \"m1\": Configuration \"conf1\" dim=3\n", buf.str());
    ostringstream buf0;
    buf0 << *p1->manifolds().at("m0");
    EXPECT_EQ("Manifold \"m0\": Configuration \"conf1\" dim=0\n", buf0.str());
  }
  remove(filename);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(TangentSpace, ASDF) {
  auto filename = "tangentspace.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Field, ASDF) {
  auto filename = "field.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

TEST(CoordinateSystem, create) {
  EXPECT_TRUE(project->coordinatesystems().empty());
  auto conf1 = project->configurations().at("conf1");
  auto m1 = project->manifolds().at("m1");
  auto cs1 = project->createCoordinateSystem("cs1", conf1, m1);
  EXPECT_EQ(1, project->coordinatesystems().size());
  EXPECT_EQ(cs1, project->coordinatesystems().at("cs1"));
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(CoordinateSystem, ASDF) {
  auto filename = "coordinatesystem.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
    ostringstream buf;
    buf << *p1->coordinatesystems().at("cs1");
    EXPECT_EQ(
        "CoordinateSystem \"cs1\": Configuration \"conf1\" Manifold \"m1\"\n",
        buf.str());
  }
  remove(filename);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Discretization, ASDF) {
  auto filename = "discretization.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(SubDiscretization, ASDF) {
  auto filename = "subdiscretization.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
    ostringstream buf;
    buf << *p1->manifolds().at("m1")->subdiscretizations().at("sd1");
    EXPECT_EQ("SubDiscretization \"sd1\": Manifold \"m1\" parent "
              "Discretization \"d1\" child Discretization \"d2\" "
              "factor=[1,1,1] offset=[0.5,0.5,0.5]\n",
              buf.str());
  }
  remove(filename);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(DiscretizationBlock, ASDF) {
  auto filename = "discretizationblock.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(Basis, ASDF) {
  auto filename = "basis.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(BasisVector, ASDF) {
  auto filename = "basisvector.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(DiscreteField, ASDF) {
  auto filename = "discretefield.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

TEST(CoordinateField, create) {
  auto cs1 = project->coordinatesystems().at("cs1");
  auto f1 = project->fields().at("f1");
  EXPECT_TRUE(cs1->coordinatefields().empty());
  auto csf1 = cs1->createCoordinateField("csf1", 0, f1);
  EXPECT_EQ(1, cs1->coordinatefields().size());
  EXPECT_EQ(csf1, cs1->coordinatefields().at("csf1"));
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(CoordinateField, ASDF) {
  auto filename = "coordinatefield.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
    ostringstream buf;
    buf << *p1->coordinatesystems().at("cs1")->coordinatefields().at("csf1");
    EXPECT_EQ("CoordinateField \"csf1\": CoordinateSystem \"cs1\" "
              "direction=0 Field "
              "\"f1\"\n",

              buf.str());
  }
  remove(filename);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(DiscreteFieldBlock, ASDF) {
  auto filename = "discretefieldblock.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
#endif

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
}

#ifdef SIMULATIONIO_HAVE_HDF5
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
              "  DiscreteFieldBlockComponent \"dfbd3\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"02\"\n"
              "  DiscreteFieldBlockComponent \"dfbd4\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"11\"\n",
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
              "\"dfb0\" TensorComponent \"scalar\"\n",
              buf0.str());
  }
  remove(filename);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(DiscreteFieldBlockComponent, ASDF) {
  auto filename = "discretizationfieldblockcomponent.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
              "  DiscreteFieldBlockComponent \"dfbd3\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"02\"\n"
              "  DiscreteFieldBlockComponent \"dfbd4\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"11\"\n",
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
              "\"dfb0\" TensorComponent \"scalar\"\n",
              buf0.str());
  }
  remove(filename);
}
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
TEST(DataBlock, create) {
  auto f1 = project->fields().at("f1");
  auto df1 = f1->discretefields().at("df1");
  auto dfb1 = df1->discretefieldblocks().at("dfb1");
  auto tt1 = f1->tensortype();
  auto bxx1 = tt1->tensorcomponents().at("00");
  auto bxy1 = tt1->tensorcomponents().at("01");
  auto bxz1 = tt1->tensorcomponents().at("02");
  auto byy1 = tt1->tensorcomponents().at("11");
  auto byz1 = tt1->tensorcomponents().at("12");
  auto dfbd1 = dfb1->discretefieldblockcomponents().at("dfbd1");
  auto dfbd2 = dfb1->discretefieldblockcomponents().at("dfbd2");
  auto dfbd3 = dfb1->discretefieldblockcomponents().at("dfbd3");
  auto dfbd4 = dfb1->discretefieldblockcomponents().at("dfbd4");
  EXPECT_FALSE(bool(dfbd1->datablock()));
  EXPECT_FALSE(bool(dfbd2->datablock()));
  EXPECT_FALSE(bool(dfbd3->datablock()));
  EXPECT_FALSE(bool(dfbd4->datablock()));
  dfbd1->createExtLink("discretizationfieldblockcomponent.s5",
                       project->name() + "/tensortypes/Scalar3D");
  dfbd1->unsetDataBlock();
  dfbd2->createExtLink("discretizationfieldblockcomponent.s5",
                       project->name() + "/tensortypes/Scalar3D");
  dfbd3->createDataSet<double>();
  int rank =
      dfb1->discretizationblock()->discretization()->manifold()->dimension();
  auto dims = dfb1->discretizationblock()->box().shape();
  double origin{-1.0};
  auto delta = dpoint<double>(rank, 2.0) / dims;
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
  auto dfbd0 = dfb0->discretefieldblockcomponents().at("dfbd0");
  EXPECT_FALSE(bool(dfbd0->datablock()));
  int rank0 =
      dfb0->discretizationblock()->discretization()->manifold()->dimension();
  double origin0{-1.0};
  dpoint<double> delta0(rank0);
  dfbd0->createDataRange(origin0, delta0);
  EXPECT_TRUE(bool(dfbd0->datarange()));
  dfbd0->unsetDataBlock();
  EXPECT_FALSE(bool(dfbd0->datarange()));
  dfbd0->createDataSet<int>();
  EXPECT_TRUE(bool(dfbd0->dataset()));
}

TEST(DataBlock, HDF5) {
  auto filename = "datablock.s5";
  {
    auto file = H5::H5File(filename, H5F_ACC_TRUNC);
    project->write(file);
    auto f1 = project->fields().at("f1");
    auto df1 = f1->discretefields().at("df1");
    auto dfb1 = df1->discretefieldblocks().at("dfb1");
    auto dfbd3 = dfb1->discretefieldblockcomponents().at("dfbd3");
    auto ds = dfbd3->dataset();
    auto shape = box_t(ds->box().lower(), point_t(vector<int>{10, 11, 12}));
    auto box = ds->box();
    vector<double> data(shape.size());
    for (int n = 0; n < shape.size(); ++n)
      data[n] = 42 + n;
    ds->writeData(data, shape, box);
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
              "    DataRange: origin=-1 delta=[0.333333,0.285714,0.25]\n",
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
    auto ds = p1->fields()
                  .at("f1")
                  ->discretefields()
                  .at("df1")
                  ->discretefieldblocks()
                  .at("dfb1")
                  ->discretefieldblockcomponents()
                  .at("dfbd3")
                  ->copyobj();
    EXPECT_TRUE(bool(ds));
    auto data = ds->readData<double>();
    EXPECT_EQ(6 * 7 * 8, data.size());
    ostringstream bufds;
    using namespace Output;
    bufds << data;
    EXPECT_EQ("[42,43,44,45,46,47,49,50,51,52,53,54,56,57,58,59,60,61,63,64,65,"
              "66,67,68,70,71,72,73,74,75,77,78,79,80,81,82,84,85,86,87,88,89,"
              "98,99,100,101,102,103,105,106,107,108,109,110,112,113,114,115,"
              "116,117,119,120,121,122,123,124,126,127,128,129,130,131,133,134,"
              "135,136,137,138,140,141,142,143,144,145,154,155,156,157,158,159,"
              "161,162,163,164,165,166,168,169,170,171,172,173,175,176,177,178,"
              "179,180,182,183,184,185,186,187,189,190,191,192,193,194,196,197,"
              "198,199,200,201,210,211,212,213,214,215,217,218,219,220,221,222,"
              "224,225,226,227,228,229,231,232,233,234,235,236,238,239,240,241,"
              "242,243,245,246,247,248,249,250,252,253,254,255,256,257,266,267,"
              "268,269,270,271,273,274,275,276,277,278,280,281,282,283,284,285,"
              "287,288,289,290,291,292,294,295,296,297,298,299,301,302,303,304,"
              "305,306,308,309,310,311,312,313,322,323,324,325,326,327,329,330,"
              "331,332,333,334,336,337,338,339,340,341,343,344,345,346,347,348,"
              "350,351,352,353,354,355,357,358,359,360,361,362,364,365,366,367,"
              "368,369,378,379,380,381,382,383,385,386,387,388,389,390,392,393,"
              "394,395,396,397,399,400,401,402,403,404,406,407,408,409,410,411,"
              "413,414,415,416,417,418,420,421,422,423,424,425,434,435,436,437,"
              "438,439,441,442,443,444,445,446,448,449,450,451,452,453,455,456,"
              "457,458,459,460,462,463,464,465,466,467,469,470,471,472,473,474,"
              "476,477,478,479,480,481]",
              bufds.str());
  }
  remove(filename);
}

TEST(DataBlock, cleanup) {
  // Remove data blocks again
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
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
TEST(DataBlock, create2) {
  auto f1 = project->fields().at("f1");
  auto df1 = f1->discretefields().at("df1");
  auto dfb1 = df1->discretefieldblocks().at("dfb1");
  auto db1 = dfb1->discretizationblock();
  auto tt1 = f1->tensortype();
  auto bxx1 = tt1->tensorcomponents().at("00");
  auto bxy1 = tt1->tensorcomponents().at("01");
  auto bxz1 = tt1->tensorcomponents().at("02");
  auto byy1 = tt1->tensorcomponents().at("11");
  auto byz1 = tt1->tensorcomponents().at("12");
  auto dfbd1 = dfb1->discretefieldblockcomponents().at("dfbd1");
  auto dfbd2 = dfb1->discretefieldblockcomponents().at("dfbd2");
  auto dfbd3 = dfb1->discretefieldblockcomponents().at("dfbd3");
  auto dfbd4 = dfb1->discretefieldblockcomponents().at("dfbd4");
  EXPECT_FALSE(bool(dfbd1->datablock()));
  EXPECT_FALSE(bool(dfbd2->datablock()));
  EXPECT_FALSE(bool(dfbd3->datablock()));
  EXPECT_FALSE(bool(dfbd4->datablock()));
  dfbd1->createASDFRef("discretizationfieldblockcomponent.asdf",
                       {project->name(), "tensortypes", "Scalar3D"});
  dfbd1->unsetDataBlock();
  dfbd2->createASDFRef("discretizationfieldblockcomponent.asdf",
                       {project->name(), "tensortypes", "Scalar3D"});
  auto box = db1->box();
  auto layout = box_t(box.lower(), point_t(vector<int>{10, 11, 12}));
  vector<double> data(layout.size());
  for (int n = 0; n < layout.size(); ++n)
    data[n] = 42 + n;
  dfbd3->createASDFData(move(data), layout);
  int rank = db1->discretization()->manifold()->dimension();
  auto dims = db1->box().shape();
  double origin{-1.0};
  auto delta = dpoint<double>(rank, 2.0) / dims;
  dfbd4->createDataRange(origin, delta);
  EXPECT_FALSE(bool(dfbd1->datablock()));
  EXPECT_TRUE(bool(dfbd2->asdfref()));
  EXPECT_TRUE(bool(dfbd3->asdfdata()));
  EXPECT_TRUE(bool(dfbd4->datarange()));
  auto f0 = project->fields().at("f0");
  auto df0 = f0->discretefields().at("df0");
  auto dfb0 = df0->discretefieldblocks().at("dfb0");
  auto tt0 = f0->tensortype();
  auto b0 = tt0->tensorcomponents().at("scalar");
  auto dfbd0 = dfb0->discretefieldblockcomponents().at("dfbd0");
  EXPECT_FALSE(bool(dfbd0->datablock()));
  int rank0 =
      dfb0->discretizationblock()->discretization()->manifold()->dimension();
  double origin0{-1.0};
  dpoint<double> delta0(rank0);
  dfbd0->createDataRange(origin0, delta0);
  EXPECT_TRUE(bool(dfbd0->datarange()));
  dfbd0->unsetDataBlock();
  EXPECT_FALSE(bool(dfbd0->datarange()));
}

TEST(DataBlock, ASDF) {
  auto filename = "datablock.asdf";
  {
    ofstream file(filename, ios::binary | ios::trunc | ios::out);
    project->writeASDF(file);
  }
  {
    auto pfile = make_shared<ifstream>(filename, ios::binary | ios::in);
    auto p1 = readProjectASDF(pfile);
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
              "    ASDFRef\n"
              "  DiscreteFieldBlockComponent \"dfbd3\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"02\"\n"
              "    ASDFArray\n"
              "  DiscreteFieldBlockComponent \"dfbd4\": DiscreteFieldBlock "
              "\"dfb1\" TensorComponent \"11\"\n"
              "    DataRange: origin=-1 delta=[0.333333,0.285714,0.25]\n",
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
              "\"dfb0\" TensorComponent \"scalar\"\n",
              buf0.str());
    auto ds = p1->fields()
                  .at("f1")
                  ->discretefields()
                  .at("df1")
                  ->discretefieldblocks()
                  .at("dfb1")
                  ->discretefieldblockcomponents()
                  .at("dfbd3")
                  ->asdfarray();
    EXPECT_TRUE(bool(ds));
    auto arr = ds->ndarray();
    auto type_size = arr->get_datatype()->type_size();
    ASSERT_EQ(sizeof(double), type_size);
    auto shape = arr->get_shape();
    EXPECT_EQ((vector<int64_t>{6, 7, 8}), shape);
    auto data = arr->get_data()->ptr();
    ostringstream bufds;
    bufds << "[";
    bool needcomma = false;
    for (ptrdiff_t k = 0; k < shape[2]; ++k) {
      for (ptrdiff_t j = 0; j < shape[1]; ++j) {
        for (ptrdiff_t i = 0; i < shape[0]; ++i) {
          ptrdiff_t lin = arr->linear_index(array<int64_t, 3>{i, j, k});
          double val = *reinterpret_cast<const double *>(
              reinterpret_cast<const unsigned char *>(data) + lin);
          if (needcomma)
            bufds << ",";
          needcomma = true;
          bufds << val;
        }
      }
    }
    bufds << "]";
    EXPECT_EQ("[42,43,44,45,46,47,49,50,51,52,53,54,56,57,58,59,60,61,63,64,65,"
              "66,67,68,70,71,72,73,74,75,77,78,79,80,81,82,84,85,86,87,88,89,"
              "98,99,100,101,102,103,105,106,107,108,109,110,112,113,114,115,"
              "116,117,119,120,121,122,123,124,126,127,128,129,130,131,133,134,"
              "135,136,137,138,140,141,142,143,144,145,154,155,156,157,158,159,"
              "161,162,163,164,165,166,168,169,170,171,172,173,175,176,177,178,"
              "179,180,182,183,184,185,186,187,189,190,191,192,193,194,196,197,"
              "198,199,200,201,210,211,212,213,214,215,217,218,219,220,221,222,"
              "224,225,226,227,228,229,231,232,233,234,235,236,238,239,240,241,"
              "242,243,245,246,247,248,249,250,252,253,254,255,256,257,266,267,"
              "268,269,270,271,273,274,275,276,277,278,280,281,282,283,284,285,"
              "287,288,289,290,291,292,294,295,296,297,298,299,301,302,303,304,"
              "305,306,308,309,310,311,312,313,322,323,324,325,326,327,329,330,"
              "331,332,333,334,336,337,338,339,340,341,343,344,345,346,347,348,"
              "350,351,352,353,354,355,357,358,359,360,361,362,364,365,366,367,"
              "368,369,378,379,380,381,382,383,385,386,387,388,389,390,392,393,"
              "394,395,396,397,399,400,401,402,403,404,406,407,408,409,410,411,"
              "413,414,415,416,417,418,420,421,422,423,424,425,434,435,436,437,"
              "438,439,441,442,443,444,445,446,448,449,450,451,452,453,455,456,"
              "457,458,459,460,462,463,464,465,466,467,469,470,471,472,473,474,"
              "476,477,478,479,480,481]",
              bufds.str());
  }
  remove(filename);
}

TEST(DataBlock, cleanup2) {
  // Remove data blocks again
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
}
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
TEST(ProjectMerge, merge) {
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
#endif

#include "src/gtest_main.cc"
