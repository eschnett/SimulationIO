"""
Concepts for describing simulations
"""
module SimulationIO

using HDF5

import Base: push!
export push!



# Project? Simulation? Iteration?



abstract Concept



# Tensor types

abstract TensorType_ <: Concept
abstract TensorComponent_ <: Concept

const tensortypes = Set{TensorType_}()

type TensorType <: TensorType_
    name::UTF8String
    dimension::Int
    rank::Int
    storedcomponents::Vector{TensorComponent_}
    function TensorType(n::AbstractString, d::Integer, r::Integer)
        tt = new(n, d, r, TensorComponent_[])
        @assert invariant(tt)
        push!(tensortypes, tt)
        tt
    end
end
function invariant(tt::TensorType)
    inv = tt.dimension>=0 && tt.rank>=0 &&
        length(tt.storedcomponents) <= tt.dimension^tt.rank
    for i in eachindex(tt.storedcomponents)
        inv &= (tt.storedcomponents[i].tensortype === tt &&
                tt.storedcomponents[i].storedcomponent == i)
    end
    inv
end
function push!(tt::TensorType, tc::TensorComponent_)
    push!(tt.storedcomponents, tc)
    @assert invariant(tt)
    tt
end

type TensorComponent <: TensorComponent_
    name::UTF8String
    tensortype::TensorType
    storedcomponent::Int
    indexvalues::Vector{Int}
    function TensorComponent{I<:Integer}(n::AbstractString,
                                            tt::TensorType, sc::Integer,
                                            ivs::Vector{I})
        tc = new(n, tt, sc, Vector{Int}(ivs))
        @assert invariant(tc)
        push!(tt, tc)
        tc
    end
end
function invariant(tc::TensorComponent)
    inv = tc.storedcomponent>=0
    for iv in tc.indexvalues
        inv &= iv>=0 && iv<tc.tensortype.dimension
    end
    inv
end

const Scalar3D = TensorType("Scalar3D", 3, 0)
TensorComponent("scalar", Scalar3D, 1, Int[])

const Vector3D = TensorType("Vector3D", 3, 1)
TensorComponent("0", Vector3D, 1, [0])
TensorComponent("1", Vector3D, 2, [1])
TensorComponent("2", Vector3D, 3, [2])

const SymmetricTensor3D = TensorType("Vector3D", 3, 2)
TensorComponent("00", SymmetricTensor3D, 1, [0,0])
TensorComponent("01", SymmetricTensor3D, 2, [0,1])
TensorComponent("02", SymmetricTensor3D, 3, [0,2])
TensorComponent("11", SymmetricTensor3D, 4, [1,1])
TensorComponent("12", SymmetricTensor3D, 5, [1,2])
TensorComponent("22", SymmetricTensor3D, 6, [2,2])



# High-level continuum concepts

abstract Manifold_ <: Concept
abstract TangentSpace_ <: Concept
abstract Field_ <: Concept
abstract Discretization_ <: Concept
abstract Basis_ <: Concept
abstract DiscreteField_ <: Concept

const manifolds = Set{Manifold_}()
const tangentspaces = Set{TangentSpace_}()
const fields = Set{Field_}()

type Manifold <: Manifold_
    name::UTF8String
    dimension::Int
    discretizations::Set{Discretization_}
    fields::Set{Field_}
    function Manifold(n::AbstractString, d::Integer)
        m = new(n, d, Set{Discretization_}(), Set{Field_}())
        @assert invariant(m)
        push!(manifolds, m)
        m
    end
end
function invariant(m::Manifold)
    inv = m.dimension>=0
    for d in m.discretizations
        inv &= d.manifold === m
    end
    for f in m.fields
        inv &= f.manifold === m
    end
    inv
end
function push!(m::Manifold, f::Field_)
    push!(m.fields, f)
    @assert invariant(m)
    m
end

type TangentSpace <: TangentSpace_
    name::UTF8String
    dimension::Int
    bases::Set{Basis_}
    fields::Set{Field_}
    function TangentSpace(n::AbstractString, d::Integer)
        ts = new(n, d, Set{Basis_}(), Set{Field_}())
        @assert invariant(ts)
        push!(tangentspaces, ts)
        ts
    end
end
function invariant(ts::TangentSpace)
    inv = ts.dimension>=0
    for b in ts.bases
        inv &= b.tangentspace === ts
    end
    for f in ts.fields
        inv &= f.tangentspace === ts
    end
    inv
end
function push!(ts::TangentSpace, f::Field_)
    push!(ts.fields, f)
    @assert invariant(ts)
    ts
end

type Field <: Field_
    name::UTF8String
    manifold::Manifold
    tangentspace::TangentSpace
    tensortype::TensorType
    discretefields::Set{DiscreteField_}
    function Field(n::AbstractString, m::Manifold, ts::TangentSpace,
        tt::TensorType)
        f = new(n, m, ts, tt, Set{DiscreteField_}())
        @assert invariant(f)
        push!(m, f)
        push!(ts, f)
        # push!(tt, f)
        push!(fields, f)
        f
    end
end
function invariant(f::Field)
    inv = f.tangentspace.dimension == f.tensortype.dimension
    for df in f.discretefields
        inv &= df.field === f
    end
    inv
end



# Manifold discretization

abstract DiscretizationBlock_ <: Concept

type Discretization <: Discretization_
    name::UTF8String
    manifold::Manifold
    discretizationblocks::Set{DiscretizationBlock_}
end
invariant(d::Discretization) = true

type DiscretizationBlock <: DiscretizationBlock_
    name::UTF8String
    discretization::Discretization
    # bounding box? in terms of coordinates?
    # connectivity? neighbouring blocks?
    # overlaps?
end
invariant(db::DiscretizationBlock) = true



# Tangent space basis

abstract BasisVector_ <: Concept
abstract CoordinateBasis_ <: Concept
abstract CoordinateBasisElement_ <: Concept

type Basis <: Basis_
    name::UTF8String
    tangentspace::TangentSpace
    basisvectors::Vector{BasisVector_}
    coordinatebases::Set{CoordinateBasis_}
end
function invariant(b::Basis)
    length(b.basisvectors) == tangentspace.dimension
end

type BasisVector <: BasisVector_
    name::UTF8String
    basis::Basis
    direction::Int
    coordinatebasiselements::Set{CoordinateBasisElement_}
end
function invariant(bv::BasisVector)
    (direction>0 && direction <= length(basis.basisvectors) &&
        basis.basisvectors[direction] === bv)
end



# Discrete field

abstract DiscreteFieldBlock_ <: Concept
abstract DiscreteFieldBlockData_ <: Concept

type DiscreteField <: DiscreteField_
    name::UTF8String
    field::Field
    discretization::Discretization
    basis::Basis
    discretefieldblocks::Set{DiscreteFieldBlock_}
end
function invariant(df::DiscreteField)
    field.manifold == discretemanifold.manifold
end

type DiscreteFieldBlock <: DiscreteFieldBlock_
    name::UTF8String
    discretefield::DiscreteField
    discretizationblock::DiscretizationBlock
    discretefieldblockdata::Set{DiscreteFieldBlockData_}
end
invariant(dfb::DiscreteFieldBlock) = true

type DiscreteFieldBlockData <: DiscreteFieldBlockData_
    name::UTF8String
    discretefieldblock::DiscreteFieldBlock
    tensorcomponent::TensorComponent
    dataset::HDF5Dataset
end
function invariant(dfbd::DiscreteFieldBlockData)
    (dfbd.discretefieldblock.discretefield.field.tensortype ===
        dfbd.tensorcomponent.tensortype)
end



# Coordinates

abstract CoordinateField_ <: Concept
abstract CoordinateBasis_ <: Concept
abstract CoordinateBasisElement_ <: Concept

type CoordinateSystem <: Concept
    manifold::Manifold
    coordinatefields::Vector{CoordinateField_}
    coordinatebases::Set{CoordinateBasis_}
end
invariant(cs::CoordinateSystem) = true

type CoordinateField <: CoordinateField_
    coordinatesystem::CoordinateSystem
    direction::Int
    field::Field
end
function invariant(cf::CoordinateField)
    (cf.direction>0 &&
        cf.direction <= length(cf.coordinatesystem.coordinatefields) &&
        cfcoordinatesystem.coordinatefields[direction] === cf)
end

type CoordinateBasis <: Concept
    coordinatesystem::CoordinateSystem
    basis::Basis
    coordinatebasiselements::Vector{CoordinateBasisElement_}
end
invariant(cb::CoordinateBasis) = true

type CoordinateBasisElement <: CoordinateBasisElement_
    coordinatebasis::CoordinateBasis
    coordinatefield::CoordinateField
    basisvector::BasisVector
end
function invariant(cbe::CoordinateBasisElement)
    cbe.coordinatefield.direction == cbe.basisvector.direction
end

const coordinatesystems = Set{CoordinateSystem}()

end
