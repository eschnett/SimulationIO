#! /usr/bin/env python

import H5
from SimulationIO import *

import numpy as np
import h5py



# Method 1:
# Read metadata via SimulationIO, then access data via path

# Read project
filename = "cactus/iof5-uniform.h5"
file = H5.H5File(filename, H5.H5F_ACC_RDONLY)
project = createProject(file)

field = project.fields['GRID::r']
discretefield = field.discretefields['GRID::r']
discretefieldblock =
    discretefield.discretefieldblocks['iteration.0-timelevel.0-m.0-rl.0']
discretefieldblockcomponent =
    discretefieldblock.discretefieldblockcomponent['scalar']
dataset = discretefieldblockcomponent.data_dataset
path = discretefieldblockdata.getPath()
name = discretefieldblockdata.getName()

# Note: Cannot pass HDF5 identifiers between H5 and h5py
# data = h5py.Dataset(dataset.getId())
# rsum = 0.0
# rcount = 0.0
# for val in np.nditer(data):
#     rsum += val
#     rcount += 1.0
# ravg = rsum / rcount
# print "Average radius: %g" % ravg

file = h5py.File(filename, 'r')
data = file[path][name]
rsum = 0.0
rcount = 0.0
for val in np.nditer(data):
    rsum += val
    rcount += 1.0
ravg = rsum / rcount
print "Average radius: %g" % ravg



# Method 2:
# Read metadata and data directly

file = h5py.File(filename, 'r')
field = file['fields']['GRID::r']
discretefield = field['discretefields']['GRID::r']
discretefieldblock =
    discretefield['discretefieldblocks']['iteration.0-timelevel.0-m.0-rl.0']
discretefieldblockcomponent =
    discretefieldblock['discretefieldblockcomponent']['scalar']
data = discretefieldblockcomponentdata']

rsum = 0.0
rcount = 0.0
for val in np.nditer(data):
    rsum += val
    rcount += 1.0
ravg = rsum / rcount
print "Average radius: %g" % ravg
