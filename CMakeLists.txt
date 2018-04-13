# CMake setup

cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

project(SimulationIO VERSION 2.4.0 LANGUAGES C CXX)
set(PROJECT_DESCRIPTION "SimulationIO: Efficient and convenient I/O for large PDE simulations")
set(PROJECT_URL "https://github.com/eschnett/SimulationIO")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")

# External dependencies

find_package(HDF5 1.10.1 COMPONENTS C CXX REQUIRED)
if(HDF5_VERSION VERSION_LESS 1.10.1)
  message(HDF5_VERSION)
  message(SEND_ERROR "HDF5 version 1.10.1 or later is required")
endif()
include_directories(${HDF5_INCLUDE_DIRS})
set(LIBS ${LIBS} ${HDF5_LIBRARIES})

# Note: We only look for MPI since HDF5 might require MPI
find_package(MPI REQUIRED)
# It seems that MPI_INCLUDE_DIRS and MPI_LIBRARIES are old names, and
# MPI_CXX_INCLUDE_PATH and MPI_CXX_LIBRARIES are newer names
include_directories(${MPI_INCLUDE_DIRS} ${MPI_CXX_INCLUDE_PATH})
set(LIBS ${LIBS} ${MPI_LIBRARIES} ${MPI_CXX_LIBRARIES})

set(Python_ADDITIONAL_VERSIONS 2.7)
# This sets PYTHONINTERP_FOUND
find_package(PythonInterp)
# This sets PYTHONLIBS_FOUND
find_package(PythonLibs)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  find_package(H5Py)
  find_package(NumPy)
endif()

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND H5PY_FOUND AND NUMPY_FOUND)
  find_package(SWIG)
endif()
if(SWIG_FOUND)
  include(${SWIG_USE_FILE})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# set(LIBS ${LIBS} Threads::Threads)

# Main project

configure_file(
  "${PROJECT_SOURCE_DIR}/Config.hpp.in"
  "${PROJECT_BINARY_DIR}/Config.hpp"
  )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

set(SIO_SRCS
  Basis.cpp
  BasisVector.cpp
  Buffer.cpp
  Config.cpp
  Configuration.cpp
  CoordinateField.cpp
  CoordinateSystem.cpp
  DataBlock.cpp
  DiscreteField.cpp
  DiscreteFieldBlock.cpp
  DiscreteFieldBlockComponent.cpp
  Discretization.cpp
  DiscretizationBlock.cpp
  Field.cpp
  Manifold.cpp
  Parameter.cpp
  ParameterValue.cpp
  Project.cpp
  SubDiscretization.cpp
  TangentSpace.cpp
  TensorComponent.cpp
  TensorType.cpp
  )

add_library(SimulationIO ${SIO_SRCS})
target_link_libraries(SimulationIO ${LIBS})
set_property(TARGET SimulationIO PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# SWIG bindings

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND H5PY_FOUND AND NUMPY_FOUND AND SWIG_FOUND)
  set_source_files_properties(H5.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(H5 python H5.i)
  # set_property(TARGET ${SWIG_MODULE_H5_REAL_NAME} PROPERTY CXX_STANDARD 11
  # set_property(TARGET ${SWIG_MODULE_H5_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  swig_link_libraries(H5 SimulationIO ${PYTHON_LIBRARIES})

  set_source_files_properties(RegionCalculus.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(RegionCalculus python RegionCalculus.i)
  # set_property(TARGET ${SWIG_MODULE_RegionCalculus_REAL_NAME} PROPERTY CXX_STANDARD 11)
  # set_property(TARGET ${SWIG_MODULE_RegionCalculus_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  swig_link_libraries(RegionCalculus SimulationIO ${PYTHON_LIBRARIES})

  set_source_files_properties(SimulationIO.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(SimulationIO python SimulationIO.i)
  # set_property(TARGET ${SWIG_MODULE_SimulationIO_REAL_NAME} PROPERTY CXX_STANDARD 11)
  # set_property(TARGET ${SWIG_MODULE_SimulationIO_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  swig_link_libraries(SimulationIO SimulationIO ${PYTHON_LIBRARIES})
endif()

# Tools

set(EXES
  sio-benchmark
  sio-convert-carpet-output
  sio-copy
  sio-example
  sio-example-attach
  sio-list
  sio-merge
  )

add_executable(sio-benchmark benchmark.cpp)
target_link_libraries(sio-benchmark SimulationIO)

add_executable(sio-convert-carpet-output convert-carpet-output.cpp)
target_link_libraries(sio-convert-carpet-output SimulationIO)

add_executable(sio-copy copy.cpp)
target_link_libraries(sio-copy SimulationIO)

add_executable(sio-example example.cpp)
target_link_libraries(sio-example SimulationIO)

add_executable(sio-example-attach example-attach.cpp)
target_link_libraries(sio-example-attach SimulationIO)

add_executable(sio-list list.cpp)
target_link_libraries(sio-list SimulationIO)

add_executable(sio-merge merge.cpp)
target_link_libraries(sio-merge SimulationIO)

# Tests

set(GOOGLETEST_DIR "googletest-release-1.8.0/googletest")
include_directories("${GOOGLETEST_DIR}/include" "${GOOGLETEST_DIR}")

add_executable(test_RegionCalculus test_RegionCalculus.cpp "${GOOGLETEST_DIR}/src/gtest-all.cc")
target_link_libraries(test_RegionCalculus SimulationIO Threads::Threads)

add_executable(test_SimulationIO test_SimulationIO.cpp "${GOOGLETEST_DIR}/src/gtest-all.cc")
target_link_libraries(test_SimulationIO SimulationIO Threads::Threads)

enable_testing()
add_test(NAME test_RegionCalculus COMMAND ./test_RegionCalculus)
add_test(NAME test_SimulationIO COMMAND ./test_SimulationIO)
add_test(NAME example COMMAND ./sio-example)
add_test(NAME list COMMAND ./sio-list example.s5)
add_test(NAME example-attach COMMAND ./sio-example-attach)
add_test(NAME list-attach COMMAND ./sio-list example-attach.s5)
add_test(NAME copy COMMAND ./sio-copy example.s5 example2.s5)
add_test(NAME list2 COMMAND ./sio-list example2.s5)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND H5PY_FOUND AND NUMPY_FOUND AND SWIG_FOUND)
  add_test(NAME python-example COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example.py")
  add_test(NAME python-example-attach COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example-attach.py")
  add_test(NAME python-read COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read.py")
  add_test(NAME h5format_convert COMMAND h5format_convert python-example.s5)
  add_test(NAME python-read-direct COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read-direct.py")
endif()

# Install

set(HEADERS
  Basis.hpp
  BasisVector.hpp
  Buffer.hpp
  Common.hpp
  Configuration.hpp
  CoordinateField.hpp
  CoordinateSystem.hpp
  DataBlock.hpp
  DiscreteField.hpp
  DiscreteFieldBlock.hpp
  DiscreteFieldBlockComponent.hpp
  Discretization.hpp
  DiscretizationBlock.hpp
  Field.hpp
  H5Helpers.hpp
  Helpers.hpp
  Manifold.hpp
  Parameter.hpp
  ParameterValue.hpp
  Project.hpp
  RegionCalculus.hpp
  SimulationIO.hpp
  SubDiscretization.hpp
  TangentSpace.hpp
  TensorComponent.hpp
  TensorType.hpp
  )

install(FILES ${HEADERS} DESTINATION include/SimulationIO)
install(FILES "${PROJECT_BINARY_DIR}/Config.hpp" DESTINATION include/SimulationIO)
install(TARGETS SimulationIO DESTINATION lib)
install(TARGETS ${EXES} DESTINATION bin)

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND H5PY_FOUND AND NUMPY_FOUND AND SWIG_FOUND)
  install(TARGETS ${SWIG_MODULE_H5_REAL_NAME} DESTINATION lib)
  install(TARGETS ${SWIG_MODULE_RegionCalculus_REAL_NAME} DESTINATION lib)
  install(TARGETS ${SWIG_MODULE_SimulationIO_REAL_NAME} DESTINATION lib)
endif()

set(PKG_CONFIG_REQUIRES "hdf5 mpi")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/include/SimulationIO")
set(PKG_CONFIG_LIBDIR "\${prefix}/lib")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")
set(PKG_CONFIG_LIBS "-L\${libdir} -lSimulationIO")

configure_file(
  "${PROJECT_SOURCE_DIR}/pkg-config.pc.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc" DESTINATION lib/pkgconfig)
