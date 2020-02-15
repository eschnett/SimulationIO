#! /usr/bin/env python

from __future__ import print_function
from math import *

import numpy as np

from SimulationIO import *

dim = 3
dirnames = ["x", "y", "z"]

nli,nlj,nlk = 10, 10, 10
npoints = nli * nlj * nlk
npi,npj,npk = 4, 4, 4
ngrids = npi * npj * npk
ni,nj,nk = npi * nli, npj * nlj, npk * nlk
xmin,ymin,zmin = -1.0, -1.0, -1.0
xmax,ymax,zmax = 1.0, 1.0, 1.0
def getcoords(i,j,k):
    assert i >= 0 and i < ni and j >= 0 and j < nj and k >= 0 and k < nk
    x = xmin + (i + 0.5) * (xmax - xmin) / ni
    y = ymin + (j + 0.5) * (ymax - ymin) / nj
    z = zmin + (k + 0.5) * (zmax - zmin) / nk
    return x, y, z

# Project
project = createProject("python-simulation")

# Configuration
configuration = project.createConfiguration("global")

# TensorTypes
project.createStandardTensorTypes()
scalar3d = project.tensortypes()["Scalar3D"]
vector3d = project.tensortypes()["Vector3D"]

# Manifold and TangentSpace, both 3D
manifold = project.createManifold("domain", configuration, dim)
tangentspace = project.createTangentSpace("space", configuration, dim)

# Discretization for Manifold
discretization = manifold.createDiscretization("uniform", configuration)
blocks = []
import RegionCalculus as RC
for pk in range(npk):
    for pj in range(npj):
        for pi in range(npi):
            p = pi + npi * (pj + npj * pk)
            block = discretization.createDiscretizationBlock("grid.%d" % p)
            lo = RC.point_t([nli*pi, nlj*pj, nlk*pk])
            hi = lo + RC.point_t([nli, nlj, nlk])
            box = RC.box_t(lo, hi)
            block.setBox(box)
            blocks.append(block)

# Basis for TangentSpace
basis = tangentspace.createBasis("Cartesian", configuration)
directions = []
for d in range(dim):
    directions.append(basis.createBasisVector(dirnames[d], d))

# Coordinate system
coordinatesystem = project.createCoordinateSystem(
    "Cartesian", configuration, manifold)
coordinates = []
for d in range(dim):
    field = project.createField(
        dirnames[d], configuration, manifold, tangentspace, scalar3d)
    discretefield = field.createDiscreteField(
        field.name(), configuration, discretization, basis)
    for p in range(ngrids):
        block = discretefield.createDiscreteFieldBlock(
            "%s-%s" % (discretefield.name(), blocks[p].name()), blocks[p])
        scalar3d_component = scalar3d.storage_indices()[0]
        component = block.createDiscreteFieldBlockComponent(
            "scalar", scalar3d_component)
        component.createDataSet(WriteOptions())
    coordinates.append(
        coordinatesystem.createCoordinateField(dirnames[d], d, field))

# Fields
rho = project.createField(
    "rho", configuration, manifold, tangentspace, scalar3d)
vel = project.createField(
    "vel", configuration, manifold, tangentspace, vector3d)
discretized_rho = rho.createDiscreteField(
    "rho", configuration, discretization, basis)
discretized_vel = vel.createDiscreteField(
    "vel", configuration, discretization, basis)
for p in range(ngrids):
    # Create discrete region
    rho_block = discretized_rho.createDiscreteFieldBlock(
        "%s-%s" % (rho.name(), blocks[p].name()), blocks[p])
    vel_block = discretized_vel.createDiscreteFieldBlock(
        "%s-%s" % (vel.name(), blocks[p].name()), blocks[p])
    # Create tensor components for this region
    scalar3d_component = scalar3d.storage_indices()[0]
    rho_component = rho_block.createDiscreteFieldBlockComponent(
        "scalar", scalar3d_component)
    rho_component.createDataSet(WriteOptions())
    for d in range(dim):
        vector3d_component = vector3d.storage_indices()[d]
        vel_component = vel_block.createDiscreteFieldBlockComponent(
            dirnames[d], vector3d_component)
        vel_component.createDataSet(WriteOptions())

# Attach data
for pk in range(npk):
    for pj in range(npj):
        for pi in range(npi):
            p = pi + npi * (pj + npj * pk)
            coordx = np.empty((nli,nlj,nlk), order='F')
            coordy = np.empty((nli,nlj,nlk), order='F')
            coordz = np.empty((nli,nlj,nlk), order='F')
            datarho = np.empty((nli,nlj,nlk), order='F')
            datavelx = np.empty((nli,nlj,nlk), order='F')
            datavely = np.empty((nli,nlj,nlk), order='F')
            datavelz = np.empty((nli,nlj,nlk), order='F')
            # lo = RC.point_t([nli * pi, nlj * pj, nlk * pk])
            # hi = lo + RC.point_t([nli, nlj, nlk])
            # box = RC.box_t(lo, hi)
            for lk in range(nlk):
                for lj in range(nlj):
                    for li in range(nli):
                        i = li + nli * pi
                        j = lj + nlj * pj
                        k = lk + nlk * pk
                        x,y,z = getcoords(i, j, k)
                        r = sqrt(x**2 + y**2 + z**2)
                        coordx[li,lj,lk] = x
                        coordy[li,lj,lk] = y
                        coordz[li,lj,lk] = z
                        datarho[li,lj,lk] = exp(-0.5 * r**2)
                        datavelx[li,lj,lk] = -y * r * exp(-0.5 * r**2)
                        datavely[li,lj,lk] = +x * r * exp(-0.5 * r**2)
                        datavelz[li,lj,lk] = 0.0
            # Write coordinates
            for d in range(dim):
                field = coordinates[d].field()
                discretefield = field.discretefields()[field.name()]
                block = discretefield.discretefieldblocks()[
                    "%s-%s" % (discretefield.name(), blocks[p].name())]
                component = block.discretefieldblockcomponents()["scalar"]
                box = block.discretizationblock().box()
                component.dataset().attachData_double(
                    np.reshape([coordx, coordy, coordz][d], npoints), box)
            # Write rho
            for d in range(1):
                field = rho
                discretefield = field.discretefields()[field.name()]
                block = discretefield.discretefieldblocks()[
                    "%s-%s" % (discretefield.name(), blocks[p].name())]
                component = block.discretefieldblockcomponents()["scalar"]
                box = block.discretizationblock().box()
                component.dataset().attachData_double(
                    np.reshape(datarho, npoints), box)
            # Write velocity
            for d in range(dim):
                field = vel
                discretefield = field.discretefields()[field.name()]
                block = discretefield.discretefieldblocks()[
                    "%s-%s" % (discretefield.name(), blocks[p].name())]
                component = block.discretefieldblockcomponents()[dirnames[d]]
                box = block.discretizationblock().box()
                component.dataset().attachData_double(
                    np.reshape([datavelx, datavely, datavelz][d], npoints), box)

# Write file
project.writeHDF5("python-example-attach.s5")
