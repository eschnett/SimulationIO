# SimulationIO
[![Build Status](https://travis-ci.org/eschnett/SimulationIO.svg?branch=master)](https://travis-ci.org/eschnett/SimulationIO)
[![Coverage Status](https://coveralls.io/repos/eschnett/SimulationIO/badge.svg?branch=master&service=github)](https://coveralls.io/github/eschnett/SimulationIO?branch=master)

Efficient and convenient I/O for large PDE simulations

## Sketches
This repository contains a few sketches describing brain storming
results in various forms in the subdirectory `sketches`:
- `AMR IO Library.md`: Original document
- `simulations.{gliffy,svg,png}`: Graphical representation
- `SimulationIO.hpp`: Description as C++ header file
- `SimulationIO.jl`: Begin of an implementation in Julia

## Ideas
- Define datatypes instead of groups with attributes to represent the various objects; avoid the "type" attributes
- Add a UUID to every object

## Handling multiple simulations, iterations
- group parameters: set of parameters
- parameter: name, type, range?, description?
- configuration: set of parameters and their values
- manifold, tangentspace, field, etc. (all?) can optionally specify a configuration
- instead of "optionally": introduce a hierarchy of configurations, and point to top-level configuration
- note: also want hierarchy of manifolds, tangent spaces
- value: parameter, data
