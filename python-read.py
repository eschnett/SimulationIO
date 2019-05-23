#! /usr/bin/env python

from __future__ import print_function
from math import *
import sys

import numpy as np

from RegionCalculus import *
from SimulationIO import *



# Read metadata via SimulationIO, then access data via path

# Read project
filename = "python-example.s5"
project = readProjectHDF5("python-example.s5")

rsum = 0.0
rsum2 = 0.0
rmin = sys.float_info.max
rmax = -sys.float_info.max
rcount = 0.0
ngrids = 0

field = project.fields()['rho']
discretefield = field.discretefields()['rho']
for discretefieldblockname in discretefield.discretefieldblocks():
    discretefieldblock = \
        discretefield.discretefieldblocks()[discretefieldblockname]
    discretefieldblockcomponent = \
        discretefieldblock.discretefieldblockcomponents()['scalar']
    dataset = discretefieldblockcomponent.copyobj()
    assert dataset
    data0 = dataset.readData_double()
    data = np.asarray(data0)

    ngrids += 1
    for val in np.nditer(data):
        rsum += val
        rsum2 += val*val
        rmin = min(rmin, val)
        rmax = max(rmax, val)
        rcount += 1.0

ravg = rsum / rcount
rnorm2 = sqrt(rsum2 / rcount)
print("""\
rho:
    ngrids=%d
    npoints=%g
    min=%g
    max=%g
    avg=%g
    norm2=%g
""" % (ngrids, rcount, rmin, rmax, ravg, rnorm2))
