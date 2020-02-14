# CMake setup

cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0078 OLD)   # requires cmake 3.13

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

project(SimulationIO VERSION 10.0.0 LANGUAGES C CXX)
set(PROJECT_DESCRIPTION
  "SimulationIO: Efficient and convenient I/O for large PDE simulations")
set(PROJECT_URL "https://github.com/eschnett/SimulationIO")

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")

# External dependencies

OPTION(ENABLE_ASDF_CXX "Enable asdf-cxx backend" ON)
if(ENABLE_ASDF_CXX)
  find_package(asdf-cxx 7.0.0)
endif()
if(ASDF_CXX_FOUND)
  if(ASDF_CXX_VERSION_STRING VERSION_LESS 7.0.0)
    message(ASDF_CXX_VERSION)
    message(SEND_ERROR "asdf-cxx version 7.0.0 or later is required")
  endif()
  include_directories(${ASDF_CXX_INCLUDE_DIR})
  set(LIBS ${LIBS} ${ASDF_CXX_LIBRARY})
  set(HAVE_ASDF_CXX 1)
else()
  set(HAVE_ASDF_CXX 0)
endif()
# TODO: Add cmake config file to asdf-cxx so that its dependencies are
# automatically required

if(ENABLE_ASDF_CXX)
  find_package(BZip2)
  if(BZIP2_FOUND)
    include_directories(${BZIP2_INCLUDE_DIR})
    set(LIBS ${LIBS} ${BZIP2_LIBRARIES})
  endif()

  find_package(OpenSSL)
  if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    set(LIBS ${LIBS} ${OPENSSL_CRYPTO_LIBRARY})
  endif()

  find_package(yaml-cpp REQUIRED)
  include_directories(${YAML_CPP_INCLUDE_DIR})
  set(LIBS ${LIBS} ${YAML_CPP_LIBRARIES})

  find_package(ZLIB)
  if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${ZLIB_LIBRARIES})
  endif()
endif()

OPTION(ENABLE_GTEST "Enable GoogleTest infrastructure" ON)
if(ENABLE_GTEST)
  find_package(GTest)
endif()
if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})
  set(HAVE_GTEST 1)
else()
  set(HAVE_GTEST 0)
endif()

OPTION(ENABLE_HDF5 "Enable HDF5 backend" ON)
if(ENABLE_HDF5)
  find_package(HDF5 1.10.1 COMPONENTS C CXX)
endif()
if(HDF5_FOUND)
  if(HDF5_VERSION VERSION_LESS 1.10.1)
    message(HDF5_VERSION)
    message(SEND_ERROR "HDF5 version 1.10.1 or later is required")
  endif()
  include_directories(${HDF5_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${HDF5_LIBRARIES})
  set(HAVE_HDF5 1)
else()
  set(HAVE_HDF5 0)
endif()

OPTION(ENABLE_MPI "Enable MPI library (if the HDF5 library needs MPI)" ON)
if(ENABLE_MPI)
  # Note: We only look for MPI since HDF5 might require MPI
  find_package(MPI)
  # It seems that MPI_INCLUDE_DIRS and MPI_LIBRARIES are old names, and
  # MPI_CXX_INCLUDE_PATH and MPI_CXX_LIBRARIES are newer names
  if(MPI_FOUND)
    include_directories(${MPI_INCLUDE_DIRS} ${MPI_CXX_INCLUDE_PATH})
    set(LIBS ${LIBS} ${MPI_LIBRARIES} ${MPI_CXX_LIBRARIES})
  endif()
endif()

OPTION(ENABLE_RNPL "Enable RNPL (SDF) backend" ON)
if(ENABLE_RNPL)
  find_package(RNPL)
endif()
if(RNPL_FOUND)
  include_directories(${RNPL_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${RNPL_LIBRARIES})
  set(HAVE_RNPL 1)
else()
  set(HAVE_RNPL 0)
endif()

OPTION(ENABLE_SILO "Enable Silo backend" ON)
if(ENABLE_SILO)
  find_package(Silo 4.10.2)
endif()
if(SILO_FOUND)
  if(SILO_VERSION_STRING VERSION_LESS 4.10.2)
    message(SILO_VERSION_STRING)
    message(SEND_ERROR "Silo version 4.10.2 or later is required")
  endif()
  include_directories(${SILO_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${SILO_LIBRARIES})
  if(HDF5_FOUND)
    set(LIBS ${LIBS} ${HDF5_LIBRARIES})
  endif()
  set(HAVE_SILO 1)
else()
  set(HAVE_SILO 0)
endif()

OPTION(ENABLE_TILEDB "Enable TileDB backend" ON)
if(ENABLE_TILEDB)
  find_package(TILEDB)
endif()
if(TILEDB_FOUND)
  include_directories(${TILEDB_INCLUDE_DIRS})
  set(LIBS ${LIBS} tiledb)
  set(HAVE_TILEDB 1)
else()
  set(HAVE_TILEDB 0)
endif()

set(Python_ADDITIONAL_VERSIONS 3.0)
# This sets PYTHONINTERP_FOUND
find_package(PythonInterp)
# This sets PYTHONLIBS_FOUND
find_package(PythonLibs)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  if(HDF5_FOUND)
    find_package(H5Py)
  endif()
  find_package(NumPy)
endif()

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND
    (NOT HDF5_FOUND OR H5PY_FOUND) AND NUMPY_FOUND)
  find_package(SWIG)
endif()
if(SWIG_FOUND)
  include(${SWIG_USE_FILE})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Main project

configure_file(
  "${PROJECT_SOURCE_DIR}/Config.hpp.in"
  "${PROJECT_BINARY_DIR}/Config.hpp"
  )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

string(REPLACE
  "-DNDEBUG" ""
  CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

set(SIO_SRCS
  Basis.cpp
  BasisVector.cpp
  Buffer.cpp
  Common.cpp
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
  H5Helpers.cpp
  Helpers.cpp
  Manifold.cpp
  Parameter.cpp
  ParameterValue.cpp
  Project.cpp
  SubDiscretization.cpp
  TangentSpace.cpp
  TensorComponent.cpp
  TensorType.cpp
  )

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND
  CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
  isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# list(FIND
#   CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${ASDF_CXX_INCLUDE_DIRS}/../lib"
#   isSystemDir)
# if("${isSystemDir}" STREQUAL "-1")
#   set(CMAKE_INSTALL_RPATH "${ASDF_CXX_INCLUDE_DIRS}/../lib")
# endif()
# 
# list(FIND
#   CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${HDF5_INCLUDE_DIRS}/../lib"
#   isSystemDir)
# if("${isSystemDir}" STREQUAL "-1")
#   set(CMAKE_INSTALL_RPATH "${HDF5_INCLUDE_DIRS}/../lib")
# endif()
# 
# list(FIND
#   CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${SILO_INCLUDE_DIRS}/../lib"
#   isSystemDir)
# if("${isSystemDir}" STREQUAL "-1")
#   set(CMAKE_INSTALL_RPATH "${SILO_INCLUDE_DIRS}/../lib")
# endif()
# 
# list(FIND
#   CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${TILEDB_INCLUDE_DIRS}/../lib"
#   isSystemDir)
# if("${isSystemDir}" STREQUAL "-1")
#   set(CMAKE_INSTALL_RPATH "${TILEDB_INCLUDE_DIRS}/../lib")
# endif()

# add_library(SimulationIO ${SIO_SRCS})
add_library(SimulationIO SHARED ${SIO_SRCS})
target_link_libraries(SimulationIO ${LIBS})
set_target_properties(SimulationIO PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(SimulationIO PROPERTIES VERSION ${PROJECT_VERSION})

# SWIG bindings

#  AND H5PY_FOUND AND ASDF_FOUND
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND SWIG_FOUND)
  set_source_files_properties(RegionCalculus.i PROPERTIES CPLUSPLUS ON)
  swig_add_library(RegionCalculus LANGUAGE python SOURCES RegionCalculus.i)
  swig_link_libraries(RegionCalculus ${LIBS} ${PYTHON_LIBRARIES})

  set_source_files_properties(SimulationIO.i PROPERTIES CPLUSPLUS ON)
  swig_add_library(SimulationIO LANGUAGE python SOURCES SimulationIO.i)
  swig_link_libraries(SimulationIO SimulationIO ${PYTHON_LIBRARIES})
endif()

# Tools

set(EXES)
if(HDF5_FOUND)
  list(APPEND EXES 
    sio-benchmark
    sio-convert-carpet-output
    sio-convert-to-carpet
    sio-copy
    sio-example
    sio-example-attach
    sio-list
    sio-merge
    sio-filter
    )
  if(RNPL_FOUND)
    list(APPEND EXES 
      sio-convert-from-rnpl-sdf
      )
  endif()
endif()
if(ASDF_CXX_FOUND)
  list(APPEND EXES 
    sio-copy-asdf
    sio-list-asdf
    sio-example-asdf
    )
endif()
if(HDF5_FOUND AND ASDF_CXX_FOUND)
  list(APPEND EXES 
    sio-convert-hdf5-to-asdf
    )
endif()
if(SILO_FOUND)
  list(APPEND EXES 
    sio-list-silo
    sio-example-attach-silo
    )
endif()
if(TILEDB_FOUND)
  list(APPEND EXES 
    sio-example-attach-tiledb
    )
endif()

if(HDF5_FOUND)
  add_executable(sio-benchmark benchmark.cpp)
  target_link_libraries(sio-benchmark SimulationIO)

  add_executable(sio-convert-carpet-output convert-carpet-output.cpp)
  target_link_libraries(sio-convert-carpet-output SimulationIO)

  add_executable(sio-convert-to-carpet convert-to-carpet.cpp)
  target_link_libraries(sio-convert-to-carpet SimulationIO)

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

  add_executable(sio-filter filter.cpp)
  target_link_libraries(sio-filter SimulationIO)

  if(RNPL_FOUND)
    add_executable(sio-convert-from-rnpl-sdf convert-from-rnpl-sdf.cpp)
    target_link_libraries(sio-convert-from-rnpl-sdf SimulationIO)
  endif()
endif()

if(ASDF_CXX_FOUND)
  add_executable(sio-copy-asdf copy-asdf.cpp)
  target_link_libraries(sio-copy-asdf SimulationIO)

  add_executable(sio-example-asdf example-asdf.cpp)
  target_link_libraries(sio-example-asdf SimulationIO)

  add_executable(sio-list-asdf list-asdf.cpp)
  target_link_libraries(sio-list-asdf SimulationIO)
endif()

if(HDF5_FOUND AND ASDF_CXX_FOUND)
  add_executable(sio-convert-hdf5-to-asdf convert-hdf5-to-asdf.cpp)
  target_link_libraries(sio-convert-hdf5-to-asdf SimulationIO)
endif()

if(SILO_FOUND)
  add_executable(silo-list silo-list.cpp)
  # This target really only needs silo and HDF5
  target_link_libraries(silo-list SimulationIO)

  add_executable(sio-example-attach-silo example-attach-silo.cpp)
  target_link_libraries(sio-example-attach-silo SimulationIO)

  add_executable(sio-list-silo list-silo.cpp)
  target_link_libraries(sio-list-silo SimulationIO)
endif()

if(TILEDB_FOUND)
  add_executable(sio-example-attach-tiledb example-attach-tiledb.cpp)
  target_link_libraries(sio-example-attach-tiledb SimulationIO)
endif()

# Tests

add_executable(test_RegionCalculus test_RegionCalculus.cpp)
target_link_libraries(test_RegionCalculus
  ${LIBS} GTest::GTest GTest::Main Threads::Threads)

add_executable(test_SimulationIO test_SimulationIO.cpp)
target_link_libraries(test_SimulationIO
  SimulationIO GTest::GTest GTest::Main Threads::Threads)

enable_testing()
add_test(NAME test_RegionCalculus COMMAND ./test_RegionCalculus)
add_test(NAME test_SimulationIO COMMAND ./test_SimulationIO)
if(HDF5_FOUND)
  add_test(NAME example COMMAND ./sio-example)
  add_test(NAME list COMMAND ./sio-list example.s5)
  add_test(NAME example-attach COMMAND ./sio-example-attach)
  add_test(NAME list-attach COMMAND ./sio-list example-attach.s5)
  add_test(NAME copy COMMAND ./sio-copy example.s5 example2.s5)
  add_test(NAME list2 COMMAND ./sio-list example2.s5)
endif()
if(ASDF_CXX_FOUND)
  add_test(NAME example-asdf COMMAND ./sio-example-asdf)
  add_test(NAME list-asdf COMMAND asdf-ls example.asdf)
  add_test(NAME list-asdf2 COMMAND ./sio-list-asdf example.asdf)
  add_test(NAME copy-asdf COMMAND ./sio-copy-asdf example.asdf example2.asdf)
  add_test(NAME list-asdf3 COMMAND ./sio-list-asdf example2.asdf)
  add_test(NAME compare-asdf
    COMMAND ${CMAKE_SOURCE_DIR}/diff-commands.sh
            "./sio-list-asdf example.asdf" "./sio-list-asdf example2.asdf")
endif()
if(HDF5_FOUND AND ASDF_CXX_FOUND)
  add_test(NAME convert-hdf5-asdf
    COMMAND ./sio-convert-hdf5-to-asdf example.s5 example3.asdf)
  add_test(NAME compare-hdf5-asdf
    COMMAND ${CMAKE_SOURCE_DIR}/diff-commands.sh
              "./sio-list-asdf example.asdf" "./sio-list-asdf example3.asdf")
endif()
if(SILO_FOUND)
  add_test(NAME example-attach-silo COMMAND ./sio-example-attach-silo)
  add_test(NAME list-silo COMMAND h5ls example-silo.silo)
  add_test(NAME list-silo2 COMMAND ./sio-list-silo example-silo.silo)
endif()
if(TILEDB_FOUND)
  add_test(NAME example-attach-tiledb COMMAND ./sio-example-attach-tiledb)
endif()
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND H5PY_FOUND AND
     NUMPY_FOUND AND SWIG_FOUND)
  if(HDF5_FOUND)
    add_test(NAME python-example
      COMMAND
      env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}"
      ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example.py")
    add_test(NAME python-example-attach
      COMMAND
      env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}"
      ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example-attach.py")
    add_test(NAME python-read
      COMMAND
      env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}"
      ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read.py")
    add_test(NAME h5format_convert
      COMMAND h5format_convert python-example.s5)
    if(H5PY_FOUND)
      add_test(NAME python-read-h5py
        COMMAND
        env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}"
        ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-read-h5py.py")
    endif()
  endif()
  if(ASDF_FOUND)
    add_test(NAME python-example-asdf
      COMMAND
      env "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}"
      ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/python-example-asdf.py")
  endif()
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
install(FILES "${PROJECT_BINARY_DIR}/Config.hpp"
  DESTINATION include/SimulationIO)
install(TARGETS SimulationIO DESTINATION lib)
install(TARGETS ${EXES} DESTINATION bin)

if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND
    H5PY_FOUND AND NUMPY_FOUND AND SWIG_FOUND)
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

install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc"
  DESTINATION lib/pkgconfig)
