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

#============================================================================
# Copyright 2017 Erik Schnetter
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
# 
#============================================================================

# Finding H5Py involves calling the Python interpreter
if(H5Py_FIND_REQUIRED)
    find_package(PythonInterp REQUIRED)
else()
    find_package(PythonInterp)
endif()

if(NOT PYTHONINTERP_FOUND)
    set(H5PY_FOUND FALSE)
endif()

execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
    "import h5py as h; print(h.version.version);"
    RESULT_VARIABLE _H5PY_SEARCH_SUCCESS
    OUTPUT_VARIABLE _H5PY_VALUES
    ERROR_VARIABLE _H5PY_ERROR_VALUE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT _H5PY_SEARCH_SUCCESS MATCHES 0)
    if(H5Py_FIND_REQUIRED)
        message(FATAL_ERROR
            "H5Py import failure:\n${_H5PY_ERROR_VALUE}")
    endif()
    set(H5PY_FOUND FALSE)
endif()

# Convert the process output into a list
string(REGEX REPLACE ";" "\\\\;" _H5PY_VALUES ${_H5PY_VALUES})
string(REGEX REPLACE "\n" ";" _H5PY_VALUES ${_H5PY_VALUES})
list(GET _H5PY_VALUES 0 H5PY_VERSION)

# Get the major and minor version numbers
string(REGEX REPLACE "\\." ";" _H5PY_VERSION_LIST ${H5PY_VERSION})
list(GET _H5PY_VERSION_LIST 0 H5PY_VERSION_MAJOR)
list(GET _H5PY_VERSION_LIST 1 H5PY_VERSION_MINOR)
list(GET _H5PY_VERSION_LIST 2 H5PY_VERSION_PATCH)
string(REGEX MATCH "[0-9]*" H5PY_VERSION_PATCH ${H5PY_VERSION_PATCH})
math(EXPR H5PY_VERSION_DECIMAL
    "(${H5PY_VERSION_MAJOR} * 10000) + (${H5PY_VERSION_MINOR} * 100) + ${H5PY_VERSION_PATCH}")

find_package_message(H5PY
    "Found H5Py: version \"${H5PY_VERSION}\""
    "${H5PY_VERSION}")

set(H5PY_FOUND TRUE)
