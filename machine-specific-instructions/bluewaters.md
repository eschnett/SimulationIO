# Instructions for installing SimulationIO on Blue Waters

* The users' guide for Blue Waters can be found
  [here](https://bluewaters.ncsa.illinois.edu/user-guide).

* This guide assumes the user will need many Python packages for the
  Python wrappers. Therefore we include instructions for installing
  Anaconda Python and linking against it.

## Module settings:

Run the following shell commands to setup the modules correctly. You
may wish to place this in your `.bashrc`.

```bash
module unload PrgEnv-cray
module load PrgEnv-gnu
module load cray-hdf5/1.8.16
```

## Install Anaconda Python

Run the following commands to install Anaconda and a modern version of swig.

```bash
wget http://repo.continuum.io/archive/Anaconda2-4.0.0-Linux-x86_64.sh
bash Anaconda2-4.0.0-Linux-x86_64.sh
# you may wish to add this to your .bashrc
export PATH=/u/sciteam/${USER}/anaconda2/bin:${PATH}
conda install swig
```

## Install SimulationIO

This section assumes that you have git setup for github , either via ssh key or https.


* First clone SimulationIO:

```bash
git clone git@github.com:eschnett/SimulationIO.git
```

* Then modify the SimulationIO makefile as follows. The HDF5 flags should be set as:

```bash
HDF5_DIR = /opt/local
HDF5_CPPFLAGS = 
HDF5_CXXFLAGS =
HDF5_LDFLAGS = -Wl,-rpath
HDF5_LIBS = -lhdf5_cpp -lhdf5
```

* The MPI flags should be set as:

```bash
MPI_DIR	=
MPI_CPPFLAGS =
MPI_CXXFLAGS =
MPI_LDFLAGS = -Wl,-rpath
MPI_LIBS =
```

* The Python flags should be set as:

```bash
PYTHON_DIR = /u/sciteam/${USER}/anaconda2
PYTHON_CPPFLAGS = -I$(PYTHON_DIR)/include/python2.7
PYTHON_CXXFLAGS =
PYTHON_LDFLAGS = -L$(PYTHON_DIR)/lib/python2.7 -Wl,-rpath,$(PYTHON_DIR)/lib/python2.7
PYTHON_LIBS =
```

* Add the `-k` flag to curl so that the curl line reads:

```bash
curl -kLv -o $(GTEST_DIR).tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
```

* Finally, you should be able to build SimulationIO with `make -j8`

