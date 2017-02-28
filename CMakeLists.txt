# CMake setup

cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

project(SimulationIO VERSION 0.1.0 LANGUAGES C CXX)
set(PROJECT_DESCRIPTION "SimulationIO: Efficient and convenient I/O for large PDE simulations")
set(PROJECT_URL "https://github.com/eschnett/SimulationIO")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")

# External dependencies

find_package(HDF5 COMPONENTS C CXX REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})
set(LIBS ${LIBS} ${HDF5_LIBRARIES})

# Note: We only require MPI since HDF5 might require MPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_DIRS})
set(LIBS ${LIBS} ${MPI_LIBRARIES})

set(Python_ADDITIONAL_VERSIONS 2.7)
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

find_package(H5Py REQUIRED)
find_package(NumPy REQUIRED)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# set(LIBS ${LIBS} Threads::Threads)

# Main project

configure_file(
  "${PROJECT_SOURCE_DIR}/Config.hpp.in"
  "${PROJECT_BINARY_DIR}/Config.hpp"
  )

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

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
set_property(TARGET SimulationIO PROPERTY CXX_STANDARD 11)
set_property(TARGET SimulationIO PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# SWIG bindings

set_source_files_properties(H5.i PROPERTIES CPLUSPLUS ON)
swig_add_module(H5 python H5.i)
swig_link_libraries(H5 SimulationIO ${PYTHON_LIBRARIES})

set_source_files_properties(RegionCalculus.i PROPERTIES CPLUSPLUS ON)
swig_add_module(RegionCalculus python RegionCalculus.i)
swig_link_libraries(RegionCalculus SimulationIO ${PYTHON_LIBRARIES})

set_source_files_properties(SimulationIO.i PROPERTIES CPLUSPLUS ON)
swig_add_module(SimulationIO python SimulationIO.i)
swig_link_libraries(SimulationIO SimulationIO ${PYTHON_LIBRARIES})

# Tools

set(EXES
  sio-benchmark
  sio-convert-carpet-output
  sio-copy
  sio-example
  sio-list
  sio-merge
  )

add_executable(sio-benchmark benchmark.cpp)
set_property(TARGET sio-benchmark PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-benchmark SimulationIO)

add_executable(sio-convert-carpet-output convert-carpet-output.cpp)
set_property(TARGET sio-convert-carpet-output PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-convert-carpet-output SimulationIO)

add_executable(sio-copy copy.cpp)
set_property(TARGET sio-copy PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-copy SimulationIO)

add_executable(sio-example example.cpp)
set_property(TARGET sio-example PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-example SimulationIO)

add_executable(sio-list list.cpp)
set_property(TARGET sio-list PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-list SimulationIO)

add_executable(sio-merge merge.cpp)
set_property(TARGET sio-merge PROPERTY CXX_STANDARD 11)
target_link_libraries(sio-merge SimulationIO)

# Tests

set(GOOGLETEST_DIR "googletest-release-1.8.0/googletest")
include_directories("${GOOGLETEST_DIR}/include" "${GOOGLETEST_DIR}")

add_executable(test_RegionCalculus test_RegionCalculus.cpp "${GOOGLETEST_DIR}/src/gtest-all.cc")
set_property(TARGET test_RegionCalculus PROPERTY CXX_STANDARD 11)
target_link_libraries(test_RegionCalculus SimulationIO Threads::Threads)

add_executable(test_SimulationIO test_SimulationIO.cpp "${GOOGLETEST_DIR}/src/gtest-all.cc")
set_property(TARGET test_SimulationIO PROPERTY CXX_STANDARD 11)
target_link_libraries(test_SimulationIO SimulationIO Threads::Threads)

enable_testing()
add_test(NAME test_RegionCalculus COMMAND ./test_RegionCalculus)
add_test(NAME test_SimulationIO COMMAND ./test_SimulationIO)
add_test(NAME example COMMAND ./sio-example)
add_test(NAME list COMMAND ./sio-list example.s5)
add_test(NAME copy COMMAND ./sio-copy example.s5 example2.s5)
add_test(NAME list2 COMMAND ./sio-list example2.s5)
add_test(NAME python-example COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example.py")
add_test(NAME python-read COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read.py")
# add_test(NAME h5format_convert COMMAND ${HDF5_DIR}/bin/h5format_convert python-example.s5)
add_test(NAME python-read-direct COMMAND env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read-direct.py")

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

install(TARGETS ${SWIG_MODULE_H5_REAL_NAME} DESTINATION lib)
install(TARGETS ${SWIG_MODULE_RegionCalculus_REAL_NAME} DESTINATION lib)
install(TARGETS ${SWIG_MODULE_SimulationIO_REAL_NAME} DESTINATION lib)

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
