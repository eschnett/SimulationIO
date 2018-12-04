# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# Findrnpl
# --------
#
# Find RNPL
#
# Find the RNPL headers and libraries.
#
# ::
#
#   RNPL_INCLUDE_DIRS   - where to find bbhutil.h, etc.
#   RNPL_LIBRARIES      - List of libraries when using RNPL.
#   RNPL_FOUND          - True if RNPL found.

# Look for the header file.
find_path(RNPL_INCLUDE_DIR NAMES librnpl.h)
mark_as_advanced(RNPL_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(RNPL_LIBRARY NAMES
  rnpl
)
mark_as_advanced(RNPL_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RNPL
                                  REQUIRED_VARS RNPL_LIBRARY RNPL_INCLUDE_DIR)

if(RNPL_FOUND)
  set(RNPL_LIBRARIES ${RNPL_LIBRARY})
  set(RNPL_INCLUDE_DIRS ${RNPL_INCLUDE_DIR})
endif()
