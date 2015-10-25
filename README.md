# SimulationIO
[![Build Status](https://travis-ci.org/eschnett/SimulationIO.svg?branch=master)](https://travis-ci.org/eschnett/SimulationIO)
[![Coverage Status](https://coveralls.io/repos/eschnett/SimulationIO/badge.svg?branch=master&service=github)](https://coveralls.io/github/eschnett/SimulationIO?branch=master)

Efficient and convenient I/O for large PDE simulations

## Current state

This repository contains a working prototype of a library that can be used to write and read simulation output, as would be necessary e.g. for the Cactus framework. There are substantially complete test cases, a simple example, an `h5ls` like utility, and a converter from the current Cactus format.

## Outdated sketches
This repository contains a few sketches describing brainstorming results in various forms in the subdirectory `sketches`:
- `AMR IO Library.md`: Original document
- `simulations.{gliffy,svg,png}`: Graphical representation
- `SimulationIO.hpp`: Description as C++ header file
- `SimulationIO.jl`: Begin of an implementation in Julia

## Ideas
- Add a UUID to every object
  - can't just create UUIDs
  - probably need to handle set of UUIDs?
- Run benchmarks with large datasets
- Add parallelism
- Add sub-manifolds, sub-tangentspaces, etc.
- Write a reader for some visualization toolkit
  - may want to use Python wrappers for this
- Allow writing just part of a project, or adding to / modifying a project
  - idea: whenever creating an HDF5 object, check whether it already exists; if so, check whether it looks as expected
- Allow removing parts of a project, deleting it from a file?
- Implement (value) equality comparison operators for our objects
- Replace H5 SWIG interface with standard Python HDF5 library
- Allow coordinates that are not fields, but are e.g. uniform, or uniform per dimension
