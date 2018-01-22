# - Try to find HighFive
# Once done this will define
#  HIGHFIVE_FOUND - System has HighFive
#  HIGHFIVE_INCLUDE_DIRS - The HighFive include directories
# HighFive is a header-only library and does not provide any libraries.

find_path(HIGHFIVE_INCLUDE_DIR highfive/H5File.hpp PATH_SUFFIXES highfive)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set HIGHFIVE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(HighFive DEFAULT_MSG HIGHFIVE_INCLUDE_DIR)

mark_as_advanced(HIGHFIVE_INCLUDE_DIR)

set(HIGHFIVE_INCLUDE_DIRS ${HIGHFIVE_INCLUDE_DIR})
