# Thoughts on a rather generic AMR I/O file format and library
(That would be useful for the Einstein Toolkit and related projects)

## General Thoughts

### Uses

- generic file I/O, i.e. exporting/importing data
- checkpointing, recovery
- visualization
- post-processing
- long-term archival

### Necessary Properties

- needs to be based on HDF5, or an equivalent portrable, widely-supported file format
- needs to support efficient parallel I/O (including quick discovery of file contents, quick reading of individual datasets)
- needs to be portable
- should be useful for similar projects, e.g. Enzo
- must support unigrid, AMR, multi-block, DGFE, staggered grids
- must be usable from C, C++, Fortran, Python, Mathematica
- should be implemented in a portable stand-alone library that can easily be used in various projects
- must remain accessible without such a library

## Ideas

We introduce various concepts that abstract various properties. Data
are described in terms of these concepts instead of using ad-hoc
descriptions for datasets.

It seems useful that each concept has abstract properties and several
concrete realizations.

Concepts:
- project (consisting of simulations)
- manifold (consisting of discretizations)
- coordinate systems (various types)
- tangent spaces (what is the connection to a coordinate system?)
- tensors (described via bases) (should bases be related to tangent spaces?)
- generic variable types (such as "floating"), implemented via concrete types (such as float32, float64)
- fields, as defined by the user

Names should be illustrative only and don't matter; only the
connection between these concepts are relevant.

Describe these via category theory!

The file format should be described e.g. as YAML document, which is
equivalent e.g. to XML documents; C++ objects holding fields, arrays,
pointers; HDF5 files; etc.

project:
  - manifolds (with various symmetries)
  - coordinate systems (scalar fields living on a manifold)
  - tangent spaces (may be connected to a coordinate system)
  - tensors (over a tangent space, with various ranks and symmetries)
  - fields (living on a manifold, consisting of a tensor)

manifold:
  - belongs to a project
  - dimension (integer)
  - symmetries (none, periodic, reflecting, rotating90, rotating180, axial symmetry, spherical symmetry)
  - can have parent manifolds, e.g. a 2-sphere living in 3-space

coordinate system:
  - belongs to a manifold
  - number of coordinates (integer)
  - list of coordinate fields, each a scalar field living on the manifold
  - can have parent coordinate systems; if so, must be consitent with parent manifold relations

tangent space:
  - belongs to a manifold
  - number of dimensions (integer)
  - can have parent tangent spaces; if so, these must live on the same manifold (e.g. on a 2-sphere, can have 2-vectors and 3-vectors)

tensor:
  - lives on a manifold
  - lives in a tangent space
  - rank (integer)
  - can have two kinds of parent tensors; if so, must be compatible with parent manifold and parent tangent spaces, respectively

field:
  - lives on a manifold
  - has a tensor type
  - can have various parent relationships



simulation:
  - belongs to a project

tensor basis:
  - for a coordinate system

tensor representation:
  - tensor
  - tensor basis

field representation:
  - field
  - discretization
  - tensor basis
