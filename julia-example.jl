#! /usr/bin/env julia

using PyCall

@pyimport H5
@pyimport SimulationIO as SIO

const dim = 3
const dirnames = ("x", "y", "z")

const nli,nlj,nlk = 10, 10, 10
const npoints = nli * nlj * nlk
const npi,npj,npk = 4, 4, 4
const ngrids = npi * npj * npk
const ni,nj,nk = npi * nli, npj * nlj, npk * nlk
const xmin,ymin,zmin = -1.0, -1.0, -1.0
const xmax,ymax,zmax = 1.0, 1.0, 1.0
function getcoords(i,j,k)
    @assert 0 < i <= ni && 0 < j <= nj && 0 < k <= nk
    x = xmin + (i - 0.5) * (xmax - xmin) / ni
    y = ymin + (j - 0.5) * (ymax - ymin) / nj
    z = zmin + (k - 0.5) * (zmax - zmin) / nk
    x, y, z
end

# Project
project = SIO.createProject("julia-simulation")

# Configuration
configuration = project[:createConfiguration]("global")

# TensorTypes
project[:createStandardTensorTypes]()
scalar3d = get(project[:tensortypes], "Scalar3D")
vector3d = get(project[:tensortypes], "Vector3D")

# Manifold and TangentSpace, both 3D
manifold = project[:createManifold]("domain", configuration, dim)
tangentspace = project[:createTangentSpace]("space", configuration, dim)

# Discretization for Manifold
discretization = manifold[:createDiscretization]("uniform", configuration)
blocks = []
for pk in 1:npk, pj in 1:npj, pi in 1:npi
    p = pi-1 + npi * (pj-1 + npj * (pk-1)) + 1
    block = discretization[:createDiscretizationBlock]("grid.$(p-1)")
    block[:setBox]((nli*pi, nlj*pj, nlk*pk), (nli, nlj, nlk))
    push!(blocks, block)
end

# Basis for TangentSpace
basis = tangentspace[:createBasis]("Cartesian", configuration)
directions = []
for d in 1:dim
    push!(directions, basis[:createBasisVector](dirnames[d], d-1))
end

# Coordinate system
coordinatesystem = project[:createCoordinateSystem](
    "Cartesian", configuration, manifold)
coordinates = []
for d in 1:dim
    field = project[:createField](
        dirnames[d], configuration, manifold, tangentspace, scalar3d)
    discretefield = field[:createDiscreteField](
        field[:name], configuration, discretization, basis)
    for p in 1:ngrids
        block = discretefield[:createDiscreteFieldBlock](
            "$(discretefield[:name])-$(blocks[p][:name])", blocks[p])
        scalar3d_component = get(scalar3d[:storage_indices], 0)
        component = block[:createDiscreteFieldBlockComponent](
            "scalar", scalar3d_component)
        dataspace = H5.DataSpace[:make]((nli, nlj, nlk))
        datatype = H5.DataType(H5.PredType[:NATIVE_DOUBLE])
        component[:setData](datatype, dataspace)
    end
    push!(coordinates,
        coordinatesystem[:createCoordinateField](dirnames[d], d-1, field))
end

# Fields
rho = project[:createField](
    "rho", configuration, manifold, tangentspace, scalar3d)
vel = project[:createField](
    "vel", configuration, manifold, tangentspace, vector3d)
discretized_rho = rho[:createDiscreteField](
    "rho", configuration, discretization, basis)
discretized_vel = vel[:createDiscreteField](
    "vel", configuration, discretization, basis)
for p in 1:ngrids
    dataspace = H5.DataSpace[:make]((nli, nlj, nlk))
    datatype = H5.DataType(H5.PredType[:NATIVE_DOUBLE])
    # Create discrete region
    rho_block = discretized_rho[:createDiscreteFieldBlock](
        "$(rho[:name])-$(blocks[p][:name])", blocks[p])
    vel_block = discretized_vel[:createDiscreteFieldBlock](
        "$(vel[:name])-$(blocks[p][:name])", blocks[p])
    # Create tensor components for this region
    scalar3d_component = get(scalar3d[:storage_indices], 0)
    rho_component = rho_block[:createDiscreteFieldBlockComponent](
        "scalar", scalar3d_component)
    rho_component[:setData](datatype, dataspace)
    for d in 1:dim
        vector3d_component = get(vector3d[:storage_indices], d-1)
        vel_component = vel_block[:createDiscreteFieldBlockComponent](
            dirnames[d], vector3d_component)
        vel_component[:setData](datatype, dataspace)
    end
end

# Write file
filename = "julia-example.s5"
file = H5.H5File(filename, H5.H5F_ACC_TRUNC)
project[:write](file)

# Write data
for pk in 1:npk, pj in 1:npj, pi in 1:npi
    p = pi-1 + npi * (pj-1 + npj * (pk-1)) + 1
    coordx = Array{Float64}(nli,nlj,nlk)
    coordy = Array{Float64}(nli,nlj,nlk)
    coordz = Array{Float64}(nli,nlj,nlk)
    datarho = Array{Float64}(nli,nlj,nlk)
    datavelx = Array{Float64}(nli,nlj,nlk)
    datavely = Array{Float64}(nli,nlj,nlk)
    datavelz = Array{Float64}(nli,nlj,nlk)
    for lk in 1:nlk, lj in 1:nlj, li in 1:nli
        i = li + nli * (pi-1)
        j = lj + nlj * (pj-1)
        k = lk + nlk * (pk-1)
        x,y,z = getcoords(i, j, k)
        r = sqrt(x^2 + y^2 + z^2)
        coordx[li,lj,lk] = x
        coordy[li,lj,lk] = y
        coordz[li,lj,lk] = z
        datarho[li,lj,lk] = exp(-0.5 * r^2)
        datavelx[li,lj,lk] = -y * r * exp(-0.5 * r^2)
        datavely[li,lj,lk] = +x * r * exp(-0.5 * r^2)
        datavelz[li,lj,lk] = 0.0
    end
    # Write coordinates
    for d in 1:dim
        field = coordinates[d][:field]
        discretefield = get(field[:discretefields], field[:name])
        block = get(discretefield[:discretefieldblocks],
            "$(discretefield[:name])-$(blocks[p][:name])")
        component = get(block[:discretefieldblockcomponents], "scalar")
        component[:writeData_double](
            reshape((coordx, coordy, coordz)[d], npoints))
    end
    # Write rho
    for d in 1:1
        field = rho
        discretefield = get(field[:discretefields], field[:name])
        block = get(discretefield[:discretefieldblocks],
            "$(discretefield[:name])-$(blocks[p][:name])")
        component = get(block[:discretefieldblockcomponents], "scalar")
        component[:writeData_double](reshape(datarho, npoints))
    end
    # Write velocity
    for d in 1:dim
        field = vel
        discretefield = get(field[:discretefields], field[:name])
        block = get(discretefield[:discretefieldblocks],
            "$(discretefield[:name])-$(blocks[p][:name])")
        component = get(block[:discretefieldblockcomponents], dirnames[d])
        component[:writeData_double](
            reshape((datavelx, datavely, datavelz)[d], npoints))
    end
end
