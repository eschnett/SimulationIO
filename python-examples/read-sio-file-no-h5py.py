#! /usr/bin/env python

from __future__ import print_function
import pysimulationio
import numpy as np
from math import *
import sys

# Read metadata via SimulationIO, then access data via path

# Read project
try:
    filename = sys.argv[1]
except:
    filename = "example.s5"
project,file_handle = pysimulationio.readProject(filename)

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
    dataset = discretefieldblockcomponent.getData_DataSet()
    shape = discretefieldblock.discretizationblock.region.shape()
    data = np.array(dataset.read_double()).reshape(shape,order='F')

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
file_handle.close()
