#! /usr/bin/env python

import H5
from RegionCalculus import *
from SimulationIO import *

import numpy as np
import h5py

from math import *
import sys



# Read metadata via SimulationIO, then access data via path

# Read project
filename = "python-example.s5"
file = H5.H5File(filename, H5.H5F_ACC_RDONLY, H5.FileCreatPropList(),
                 H5.FileAccPropList())
project = readProject(file)
file = h5py.File(filename, 'r')

rsum = 0.0
rsum2 = 0.0
rmin = sys.float_info.max
rmax = -sys.float_info.max
rcount = 0.0
ngrids = 0

field = project.fields['rho']
discretefield = field.discretefields['rho']
for discretefieldblockname in discretefield.discretefieldblocks:
    discretefieldblock = discretefield.discretefieldblocks[discretefieldblockname]
    discretefieldblockcomponent = discretefieldblock.discretefieldblockcomponents['scalar']
    assert discretefieldblockcomponent.data_type == DiscreteFieldBlockComponent.type_dataset
    dataset = discretefieldblockcomponent.getData_dataset()
    path = dataset.path
    name = dataset.name



    # discretizationblock = discretefieldblock.discretizationblock
    # region = discretizationblock.region
    # print "region=", region
    # active = discretizationblock.active
    # print "active=", active



    # Note: Cannot pass HDF5 identifiers between H5 and h5py
    # data = h5py.Dataset(dataset.getId())
    # rsum = 0.0
    # rcount = 0.0
    # for val in np.nditer(data):
    #     rsum += val
    #     rcount += 1.0
    # ravg = rsum / rcount
    # print "Average radius: %g" % ravg

    data = file[path][name]
    ngrids += 1
    for val in np.nditer(data):
        rsum += val
        rsum2 += val*val
        rmin = min(rmin, val)
        rmax = max(rmax, val)
        rcount += 1.0

ravg = rsum / rcount
rnorm2 = sqrt(rsum2 / rcount)
print "rho: ngrids=%d npoints=%g min=%g max=%g avg=%g norm2=%g" % (ngrids, rcount, rmin, rmax, ravg, rnorm2)
