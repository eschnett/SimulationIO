# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindSilo
# --------
#
# Find Silo
#
# Find the Silo headers and libraries.
#
# ::
#
#   SILO_INCLUDE_DIRS   - where to find silo.h, etc.
#   SILO_LIBRARIES      - List of libraries when using Silo.
#   SILO_FOUND          - True if Silo found.
#   SILO_VERSION_STRING - the version of Silo found

# Look for the header file.
find_path(SILO_INCLUDE_DIR NAMES silo.h)
mark_as_advanced(SILO_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(SILO_LIBRARY NAMES siloh5 silo)
mark_as_advanced(SILO_LIBRARY)

if(SILO_INCLUDE_DIR)
  foreach(_silo_version_header silo.h)
    if(EXISTS "${SILO_INCLUDE_DIR}/${_silo_version_header}")
      file(STRINGS "${SILO_INCLUDE_DIR}/${_silo_version_header}" silo_version_major_str REGEX "^#define[\t ]+SILO_VERS_MAJ[\t ]+[0-9]+")
      file(STRINGS "${SILO_INCLUDE_DIR}/${_silo_version_header}" silo_version_minor_str REGEX "^#define[\t ]+SILO_VERS_MIN[\t ]+[0-9]+")
      file(STRINGS "${SILO_INCLUDE_DIR}/${_silo_version_header}" silo_version_patch_str REGEX "^#define[\t ]+SILO_VERS_PAT[\t ]+[0-9]+")
      string(REGEX REPLACE "^#define[\t ]+SILO_VERS_MAJ[\t ]+([0-9]+).*!#define[\t ]+SILO_VERS_MIN[\t ]+([0-9]+).*!#define[\t ]+SILO_VERS_PAT[\t ]+([0-9]+).*" "\\1.\\2.\\3" SILO_VERSION_STRING "${silo_version_major_str}!${silo_version_minor_str}!${silo_version_patch_str}")
      unset(silo_version_major_str)
      unset(silo_version_minor_str)
      unset(silo_version_pator_str)
      break()
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SILO
                                  REQUIRED_VARS SILO_LIBRARY SILO_INCLUDE_DIR
                                  VERSION_VAR SILO_VERSION_STRING)

if(SILO_FOUND)
  set(SILO_LIBRARIES ${SILO_LIBRARY})
  set(SILO_INCLUDE_DIRS ${SILO_INCLUDE_DIR})
endif()
