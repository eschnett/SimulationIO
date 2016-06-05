# SimulationIO
[![Build Status](https://travis-ci.org/eschnett/SimulationIO.svg?branch=release-0.1)](https://travis-ci.org/eschnett/SimulationIO)
[![Coverage Status](https://coveralls.io/repos/eschnett/SimulationIO/badge.svg?branch=release-0.1&service=github)](https://coveralls.io/github/eschnett/SimulationIO?branch=release-0.1)
[![codecov.io](https://codecov.io/github/eschnett/SimulationIO/coverage.svg?branch=release-0.1)](https://codecov.io/github/eschnett/SimulationIO?branch=release-0.1)

SimulationIO is a library for efficient and convenient I/O for large
PDE simulations. The intent is to abstract away from details such as
files and formats and describe the simulation as one or more
discretizations of a manifold, complete with tangent space,
fiber-bundles, and sub-manifolds. The project is currently in a
working beta.

More information can be found in
[this presentation](https://github.com/Yurlungur/simulationio-and-yt)
on SimulationIO and related tools.

## Components

SimulationIO has several components:
- A collection of object files that can be linked against. For now,
  only static linking is supported.
- Several utility programs that can, for example, convert HDF5 output
  from the [Einstein Toolkit](http://einsteintoolkit.org/) into the
  SimulationIO format.
- A python library, pysimulationio that wraps the object files into a
  single shared python API.

## Using

- To convert output from the Einstein Toolkit:

```bash
sio-convert-carpet-output output_file.s5 /path/to/input_files/*.h5
```

- To look at the innards of a file:

```bash
sio-list filename.s5
```

- To generate an example file:

```bash
sio-example
```

- The repository also contains several python files which show how to
  use the python API.

## Requirements

SimulationIO relies on a modern version of HDF5 and a modern version
of MPI.

## Installing

The python library depends on but is nequired for the rest of the
library. Therefore, it is installed separately.

### Installng the core library

To install the utilities and object files, clone the repository. Then
open `Make.user` and change it to meet your needs. There are defaults
set, but they may not work for you. The supported settings (and
defaults) are below:

```bash
CXX=g++
```

You may use CXX to set a compiler other than g++, for example the
intel compilers.

```bash
MPI_NAME=mpich
MPI_DIR=/usr
MPI_LIBS="-lmpichcxx -lmpich"
```

This assumes that the mpi include directory is `${MPI_DIR}/include`
and lib directory is `${MPI_DIR}/lib/x86_64-gnu`. You may also
explicitly set these values:

```bash
MPI_INCDIR=${MPI_DIR}/include/${MPI_NAME}
MPI_CPPFLAGS=-I${MPI_INCDIR}
MPI_CXXFLAGS=
MPI_LIBDIR=${MPI_DIR}/lib/x86_64-gnu
MPI_LDFLAGS="-L${MPI_LIBDIR} -Wl,-rpath,${MPI_LIBDIR}"
```

Similarly, for HDF5:

```bash
HDF5_DIR=/usr/local/hdf5
HDF5_INCDIR=${HDF5_DIR}/include
HDF5_CPPFLAGS=-I{HDF5_INCDIR}
HDF5_CXXFLAGS=
HDF5_LIBDIR=${HDF5_DIR}/lib
HDF5_LDFLAGS="-L${HDF5_LIBDIR} -Wl,-rpath,${HDF5_LIBDIR}"
HDF5_LIBS="-lhdf5_cpp -lhdf5"
```

Once you've set your environment variables, run

```bash
make -j N
```

where `N` is the number of processors you'd like to use. This
generates the object files and utilities. If you'd like to install the
utilities in a global location you can set that location with the
shell variables

```bash
PREFIX=/usr/local
BIN_SUBDIR=bin
```

then type `make install` to install the to copy the utilities to
`${PREFIX}/${BIN_SUBDIR}`. You may need to use administrator priveleges.

### Installing the Python API

If you've run `make` all you need to do is run

```bash
python setup.py
```

as usual for python packages.


### Hangups

- If you need to install the python API locally, use

```bash
python setup.py --user
```

- If you have h5py installed with python, setup.py will probably try
  to link to the version of hdf5 it uses. The best solution to this is
  to manually tell SimulationIO to point to that directory. If you are
  using Anaconda python, this means:

```bash
HDF5_DIR=/path/to/anaconda/environment/root
```

