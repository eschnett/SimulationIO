# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# Findasdf-cxx
# --------
#
# Find asdf-cxx
#
# Find the asdf-cxx headers and libraries.
#
# ::
#
#   ASDF_CXX_INCLUDE_DIRS   - where to find asdf.hpp, etc.
#   ASDF_CXX_LIBRARIES      - List of libraries when using asdf-cxx.
#   ASDF_CXX_FOUND          - True if asdf-cxx found.
#   ASDF_CXX_VERSION_STRING - the version of asdf-cxx found

# Look for the header file.
find_path(ASDF_CXX_INCLUDE_DIR NAMES asdf.hpp)
mark_as_advanced(ASDF_CXX_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(ASDF_CXX_LIBRARY NAMES asdf-cxx)
mark_as_advanced(ASDF_CXX_LIBRARY)

if(ASDF_CXX_INCLUDE_DIR)
  foreach(_asdf_cxx_version_header asdf_config.hpp)
    if(EXISTS "${ASDF_CXX_INCLUDE_DIR}/${_asdf_cxx_version_header}")
      file(STRINGS "${ASDF_CXX_INCLUDE_DIR}/${_asdf_cxx_version_header}" asdf_cxx_version_str REGEX "^#define[\t ]+ASDF_VERSION[\t ]+\".*\"")
      string(REGEX REPLACE "^#define[\t ]+ASDF_VERSION[\t ]+\"([^\"]*)\".*" "\\1" ASDF_CXX_VERSION_STRING "${asdf_cxx_version_str}")
      unset(asdf_cxx_version_str)
      break()
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASDF_CXX
                                  REQUIRED_VARS ASDF_CXX_LIBRARY ASDF_CXX_INCLUDE_DIR
                                  VERSION_VAR ASDF_CXX_VERSION_STRING)

if(ASDF_CXX_FOUND)
  set(ASDF_CXX_LIBRARIES ${ASDF_CXX_LIBRARY})
  set(ASDF_CXX_INCLUDE_DIRS ${ASDF_CXX_INCLUDE_DIR})
endif()
