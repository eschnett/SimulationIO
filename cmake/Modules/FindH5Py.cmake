# - Find the H5Py libraries
# This module finds if H5Py is installed, and sets the following variables
# indicating where it is.
#
#  H5PY_FOUND               - was H5Py found
#  H5PY_VERSION             - the version of H5Py found as a string
#  H5PY_VERSION_MAJOR       - the major version number of H5Py
#  H5PY_VERSION_MINOR       - the minor version number of H5Py
#  H5PY_VERSION_PATCH       - the patch version number of H5Py
#  H5PY_VERSION_DECIMAL     - e.g. version 1.6.1 is 10601
#  H5PY_INCLUDE_DIR         - path to the H5Py include files



# COPYRIGHT
# 
# All contributions by the University of California:
# Copyright (c) 2014-2017 The Regents of the University of California (Regents)
# All rights reserved.
# 
# All other contributions:
# Copyright (c) 2014-2017, the respective contributors
# All rights reserved.
# 
# Caffe uses a shared copyright model: each contributor holds
# copyright over their contributions to Caffe. The project versioning
# records all such contribution and copyright details. If a
# contributor wants to further mark their specific copyright on a
# particular contribution, they should indicate their copyright solely
# in the commit message of the change when it is committed.
# 
# LICENSE
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.



unset(H5PY_VERSION)
unset(H5PY_INCLUDE_DIR)

if(PYTHONINTERP_FOUND)
  execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
    "import numpy as n; print(n.__version__); print(n.get_include());"
    RESULT_VARIABLE __result
    OUTPUT_VARIABLE __output
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(__result MATCHES 0)
    string(REGEX REPLACE ";" "\\\\;" __values ${__output})
    string(REGEX REPLACE "\r?\n" ";"    __values ${__values})
    list(GET __values 0 H5PY_VERSION)
    list(GET __values 1 H5PY_INCLUDE_DIR)

    string(REGEX MATCH "^([0-9])+\\.([0-9])+\\.([0-9])+" __ver_check "${H5PY_VERSION}")
    if(NOT "${__ver_check}" STREQUAL "")
      set(H5PY_VERSION_MAJOR ${CMAKE_MATCH_1})
      set(H5PY_VERSION_MINOR ${CMAKE_MATCH_2})
      set(H5PY_VERSION_PATCH ${CMAKE_MATCH_3})
      math(EXPR H5PY_VERSION_DECIMAL
        "(${H5PY_VERSION_MAJOR} * 10000) + (${H5PY_VERSION_MINOR} * 100) + ${H5PY_VERSION_PATCH}")
      string(REGEX REPLACE "\\\\" "/"  H5PY_INCLUDE_DIR ${H5PY_INCLUDE_DIR})
    else()
     unset(H5PY_VERSION)
     unset(H5PY_INCLUDE_DIR)
     message(STATUS "Requested H5Py version and include path, but got instead:\n${__output}\n")
    endif()
  endif()
else()
  message(STATUS "To find H5Py Python interpretator is required to be found.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(H5Py REQUIRED_VARS H5PY_INCLUDE_DIR H5PY_VERSION
                                        VERSION_VAR   H5PY_VERSION)

if(H5PY_FOUND)
  message(STATUS "H5Py version ${H5PY_VERSION} found (include: ${H5PY_INCLUDE_DIR})")
endif()
