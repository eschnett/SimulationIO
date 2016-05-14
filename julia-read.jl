#! /usr/bin/env julia

using PyCall

@pyimport H5
@pyimport SimulationIO as SIO

using HDF5



# Read metadata via SimulationIO, then access data via path

# Read project
filename = "julia-example.s5"
file = H5.H5File(filename, H5.H5F_ACC_RDONLY)
project = SIO.readProject(file)
file = h5open(filename, "r")

rsum = 0.0
rsum2 = 0.0
rmin = typemax(Float64)
rmax = typemin(Float64)
rcount = 0.0
ngrids = 0

field = get(project[:fields], "rho")
discretefield = get(field[:discretefields], "rho")
for discretefieldblockname in discretefield[:discretefieldblocks]
    discretefieldblock =
        get(discretefield[:discretefieldblocks], discretefieldblockname)
    discretefieldblockcomponent =
        get(discretefieldblock[:discretefieldblockcomponents], "scalar")
    @assert discretefieldblockcomponent[:data_type] ==
        SIO.DiscreteFieldBlockComponent[:type_dataset]
    dataset = discretefieldblockcomponent[:getData_dataset]()
    path = dataset[:path]
    name = dataset[:name]

    # Note: Cannot pass HDF5 identifiers between H5 and HDF5
    # data = h5py.Dataset(dataset.getId())
    # rsum = 0.0
    # rcount = 0.0
    # for val in np.nditer(data):
    #     rsum += val
    #     rcount += 1.0
    # ravg = rsum / rcount
    # print "Average radius: %g" % ravg

    data = read(file[path][name])
    ngrids += 1
    for val in data
        rsum += val
        rsum2 += val^2
        rmin = min(rmin, val)
        rmax = max(rmax, val)
        rcount += 1.0
    end
end

if rcount == 0.0
    ravg = 0.0
    rnorm2 = 0.0
else
    ravg = rsum / rcount
    rnorm2 = sqrt(rsum2 / rcount)
end
println("rho: ngrids=$ngrids npoints=$rcount min=$rmin max=$rmax avg=$ravg norm2=$rnorm2")
