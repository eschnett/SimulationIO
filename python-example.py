#! /usr/bin/env python

import H5
from SimulationIO import *

# Project
project = createProject("python-simulation")

# Configuration
configuration = project.createConfiguration("global")


# TensorTypes
project.createStandardTensorTypes()
scalar3d = project.tensortypes["Scalar3D"]
vector3d = project.tensortypes["Vector3D"]

# Manifold and TangentSpace, both 3D
dim = 3
manifold = project.createManifold("domain", dim)
tangentspace = project.createTangentSpace("space", dim)

# Discretization for Manifold
discretization = manifold.createDiscretization("uniform")
ngrids = 10
blocks = []
for i in range(ngrids):
    blocks.append(discretization.createDiscretizationBlock("grid.%d" % i))

# Basis for TangentSpace
basis = tangentspace.createBasis("Cartesian")
dirnames = ["x", "y", "z"]
directions = []
for d in range(dim):
    directions.append(basis.createBasisVector(dirnames[d], d))

# Fields
rho = project.createField("rho", manifold, tangentspace, scalar3d)
vel = project.createField("vel", manifold, tangentspace, vector3d)
discretized_rho = rho.createDiscreteField("rho", discretization, basis)
discretized_vel = vel.createDiscreteField("vel", discretization, basis)
for i in range(ngrids):
    # Create discrete region
    rho_block = discretized_rho.createDiscreteFieldBlock(
        "%s-%s" % (rho.name, blocks[i].name), blocks[i])
    vel_block = discretized_vel.createDiscreteFieldBlock(
        "%s-%s" % (vel.name, blocks[i].name), blocks[i])
    # Create tensor components for this region
    scalar3d_component = scalar3d.storage_indices[0]
    rho_component = rho_block.createDiscreteFieldBlockData(
        rho_block.name, scalar3d_component)
    for d in range(dim):
        vector3d_component = vector3d.storage_indices[d]
        vel_component = vel_block.createDiscreteFieldBlockData(
            "%s-%s" % (vel_block.name, dirnames[d]), vector3d_component)

# Write file
filename = "python-example.h5"
file = H5.H5File(filename, H5.H5F_ACC_TRUNC)
project.write(file)
