#! /usr/bin/env python

from __future__ import print_function
import pysimulationio
import numpy as np
import h5py
from math import *
import sys

# Read metadata via SimulationIO, then access data via path

# Read project
try:
    filename = sys.argv[1]
except:
    filename = "example.s5"
project,file_handle = pysimulationio.readProject(filename)
f = h5py.File(filename, 'r')

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
    dataset = discretefieldblockcomponent.data_dataset

    path = discretefieldblockcomponent.getPath()
    name = discretefieldblockcomponent.getName()

    # Note: Cannot pass HDF5 identifiers between H5 and h5py
    # data = h5py.Dataset(dataset.getId())
    # rsum = 0.0
    # rcount = 0.0
    # for val in np.nditer(data):
    #     rsum += val
    #     rcount += 1.0
    # ravg = rsum / rcount
    # print "Average radius: %g" % ravg

    data = f[path][name]
    ngrids += 1
    for val in np.nditer(data):
        rsum += val
        rsum2 += val*val
        rmin = min(rmin, val)
        rmax = max(rmax, val)
        rcount += 1.0

ravg = rsum / rcount
rnorm2 = sqrt(rsum2 / rcount)
print("rho: ngrids=%d npoints=%g min=%g max=%g avg=%g norm2=%g"
      % (ngrids, rcount, rmin, rmax, ravg, rnorm2))
f.close()
file_handle.close()
