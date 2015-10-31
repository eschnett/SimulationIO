#! /usr/bin/env python

import numpy as np
import h5py

from math import *
import sys



# Read metadata and data directly

filename = "example.h5"
file = h5py.File(filename, 'r')

rsum = 0.0
rsum2 = 0.0
rmin = sys.float_info.max
rmax = -sys.float_info.max
rcount = 0.0
ngrids = 0

field = file['fields']['rho']
discretefield = field['discretefields']['rho']
for discretefieldblockname in discretefield['discretefieldblocks']:
    discretefieldblock = discretefield['discretefieldblocks'][discretefieldblockname]
    discretefieldblockcomponent = discretefieldblock['discretefieldblockcomponents']['scalar']

    data = discretefieldblockcomponent['data']
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
