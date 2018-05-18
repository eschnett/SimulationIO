# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_ASDF_CXX QUIET asdf-cxx)

find_path(ASDF_CXX_INCLUDE_DIR asdf.hpp
  HINTS
    ${ASDF_CXX_ROOT} ENV ASDF_CXX_ROOT
    ${PC_ASDF_CXX_MINIMAL_INCLUDEDIR}
    ${PC_ASDF_CXX_MINIMAL_INCLUDE_DIRS}
    ${PC_ASDF_CXX_INCLUDEDIR}
    ${PC_ASDF_CXX_INCLUDE_DIRS}
  PATH_SUFFIXES include)

find_library(ASDF_CXX_LIBRARY NAMES asdf-cxx libasdf-cxx
  HINTS
    ${ASDF_CXX_ROOT} ENV ASDF_CXX_ROOT
    ${PC_ASDF_CXX_MINIMAL_LIBDIR}
    ${PC_ASDF_CXX_MINIMAL_LIBRARY_DIRS}
    ${PC_ASDF_CXX_LIBDIR}
    ${PC_ASDF_CXX_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64)

set(ASDF_CXX_LIBRARIES ${ASDF_CXX_LIBRARY})
set(ASDF_CXX_INCLUDE_DIRS ${ASDF_CXX_INCLUDE_DIR})

find_package_handle_standard_args(asdf-cxx DEFAULT_MSG
  ASDF_CXX_LIBRARY ASDF_CXX_INCLUDE_DIR)

get_property(_type CACHE ASDF_CXX_ROOT PROPERTY TYPE)
if(_type)
  set_property(CACHE ASDF_CXX_ROOT PROPERTY ADVANCED 1)
  if("x${_type}" STREQUAL "xUNINITIALIZED")
    set_property(CACHE ASDF_CXX_ROOT PROPERTY TYPE PATH)
  endif()
endif()

mark_as_advanced(ASDF_CXX_ROOT ASDF_CXX_LIBRARY ASDF_CXX_INCLUDE_DIR)
