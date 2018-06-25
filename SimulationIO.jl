# TODO: Split this file (and its tests) into separate files, one for
# each module

using Compat
import Compat.String

module PyCall2

export PyWrapped, PyMap, PyWeakPtr
export @wrap_type, @wrap_ctor, @wrap_field, @wrap_memfun

using PyCall

abstract PyWrapped

immutable PyMap{K,V}
    pyobj::PyObject
end
import Base: isempty, length
isempty(map::PyMap) = isempty(map.pyobj)
length(map::PyMap) = length(map.pyobj)
import Base: getindex, haskey
getindex{K,V}(map::PyMap{K,V}, key) = V(map.pyobj[:__getitem__](K(key)))
#TODO haskey(map::PyMap, key) = map.pyobj[:count](K(key)) != 0
#TODO haskey(map::PyMap, key) = map[:has_key](K(key))

immutable PyWeakPtr{T}
    pyobj::PyObject
end
import Base: isnull, get
isnull{T}(ptr::PyWeakPtr{T}) = ptr.pyobj[:expired]()::Bool
function getindex{T}(ptr::PyWeakPtr{T})
    @assert !isnull(ptr)
    T(ptr.pyobj[:lock]())
 end
# get{T}(ptr::PyWeakPtr{T}, other) = T(if !isnull, other ...)

unwrap(obj) = obj
unwrap(objs::Array) = map(unwrap, objs)
unwrap(objs::Tuple) = map(unwrap, objs)

# wrap(::Type, obj) = obj
# wrap{T<:PyWrapped}(::Type{T}, obj) = T(obj)

# convert that also works for Vectors
convert1{T}(::Type{T}, obj) = T(obj)
convert1{T}(::Type{Vector{T}}, obj) = map(x -> convert1(T,x), collect(obj))

macro wrap_type(decl::Symbol)
    typesym::Symbol = decl
    esc(quote
        export $typesym
        immutable $typesym <: PyWrapped
            pyobj::PyObject
        end
        PyCall2.unwrap(obj::$typesym) = obj.pyobj
    end)
end

macro wrap_ctor(decl::Expr)
    @assert decl.head === :call
    @assert decl.args[1].head === :(.)
    @assert length(decl.args[1].args) == 2
    pkg::Symbol = decl.args[1].args[1]
    @assert decl.args[1].args[2].head === :quote
    @assert length(decl.args[1].args[2].args) == 1
    typesym::Symbol = decl.args[1].args[2].args[1]
    args = decl.args[2:end]
    argnames = []
    argtypes = []
    for arg in args
        if isa(arg, Symbol)
            push!(argnames, arg)
            push!(argtypes, :Any)
        else
            @assert isa(arg, Expr)
            @assert arg.head === :(::)
            @assert length(arg.args) == 2
            push!(argnames, arg.args[1])
            push!(argtypes, arg.args[2])
        end
    end
    esc(quote
        $typesym($(args...)) =
            $typesym($pkg.$typesym($([:(PyCall2.unwrap($argname))
                                      for argname in argnames]...)))
    end)
end

macro wrap_field(decl::Expr)
    if decl.head === :(::)
        defn::Expr, rettype = decl.args
    else
        defn, rettype = decl, :Any
    end
    @assert defn.head === :.
    objtypesym::Symbol, fieldexpr::Expr = defn.args
    @assert fieldexpr.head === :quote
    fieldsym::Symbol = fieldexpr.args[1]
    esc(quote
        export $fieldsym
        $fieldsym(obj::$objtypesym) =
            PyCall2.convert1($rettype, obj.pyobj[$(QuoteNode(fieldsym))])
    end)
end

macro wrap_memfun(decl::Expr)
    if decl.head === :(::)
        fun::Expr, rettypesym = decl.args
    else
        fun, rettypesym = decl, :Any
    end
    @assert fun.head === :call

    defn::Expr = fun.args[1]
    @assert defn.head === :.
    objtypesym::Symbol, memfunexpr::Expr = defn.args
    @assert memfunexpr.head === :quote
    memfunsym::Symbol = memfunexpr.args[1]

    declargs = []
    callargs = []
    for arg in fun.args[2:end]
        if isa(arg, Expr) && arg.head === :(::)
            argnamesym::Symbol, argtypesym = arg.args
        else
            @assert isa(arg, Symbol)
            argnamesym, argtypesym = arg, :Any
        end
        push!(declargs, :($argnamesym::$argtypesym))
        push!(callargs, :(PyCall2.unwrap($argnamesym)))
    end

    esc(quote
        export $memfunsym
        $(Expr(:call, memfunsym, :(obj::$objtypesym), declargs...)) =
            PyCall2.convert1($rettypesym,
                $(Expr(:call, :(obj.pyobj[$(QuoteNode(memfunsym))]),
                    callargs...)))
    end)
end

end



module RegionCalculus

using PyCall
using PyCall2

@pyimport RegionCalculus as RC

import Base: -, *, &, |, $, <<, >>, ==, !=, <=, >=, <, >
import Base: contains, isempty, rank, show, size



@wrap_type ibox
typealias IBox ibox
export IBox

@wrap_ctor RC.ibox()
@wrap_ctor RC.ibox(d::Int)
@wrap_ctor RC.ibox(lo::Vector{Int}, hi::Vector{Int})

@wrap_memfun IBox.valid()::Bool
@wrap_memfun IBox.rank()::Int
@wrap_memfun IBox.empty()::Bool
isempty(b::IBox) = empty(b)
@wrap_memfun IBox.lower()::Vector{Int}
@wrap_memfun IBox.upper()::Vector{Int}
@wrap_memfun IBox.shape()::Vector{Int}
@wrap_memfun IBox.size()::Int

@wrap_memfun IBox.__rshift__(p::Vector{Int})::IBox
>>(b::IBox, p::Vector{Int}) = __rshift__(b, p)
@wrap_memfun IBox.__lshift__(p::Vector{Int})::IBox
<<(b::IBox, p::Vector{Int}) = __lshift__(b, p)
@wrap_memfun IBox.__mul__(p::Vector{Int})::IBox
*(b::IBox, p::Vector{Int}) = __mul__(b, p)

@wrap_memfun IBox.__eq__(b::IBox)::Bool
==(b1::IBox, b2::IBox) = __eq__(b1, b2)
@wrap_memfun IBox.__ne__(b::IBox)::Bool
!=(b1::IBox, b2::IBox) = __ne__(b1, b2)
@wrap_memfun IBox.__le__(b::IBox)::Bool
<=(b1::IBox, b2::IBox) = __le__(b1, b2)
@wrap_memfun IBox.__ge__(b::IBox)::Bool
>=(b1::IBox, b2::IBox) = __ge__(b1, b2)
@wrap_memfun IBox.__lt__(b::IBox)::Bool
<(b1::IBox, b2::IBox) = __lt__(b1, b2)
@wrap_memfun IBox.__gt__(b::IBox)::Bool
>(b1::IBox, b2::IBox) = __gt__(b1, b2)
@wrap_memfun IBox.contains(p::Vector{Int})::Bool
@wrap_memfun IBox.isdisjoint(b::IBox)::Bool
@wrap_memfun IBox.issubset(b::IBox)::Bool
@wrap_memfun IBox.issuperset(b::IBox)::Bool
@wrap_memfun IBox.is_strict_subset(b::IBox)::Bool
@wrap_memfun IBox.is_strict_superset(b::IBox)::Bool

@wrap_memfun IBox.bounding_box(b::IBox)::IBox
@wrap_memfun IBox.__and__(b::IBox)::IBox
(&)(b1::IBox, b2::IBox) = __and__(b1, b2)
@wrap_memfun IBox.intersection(p::IBox)::IBox

@wrap_memfun IBox.__str__()::String
show(io::IO, b::IBox) = print(io, __str__(b))



@wrap_type iregion
typealias IRegion iregion
export IRegion

@wrap_ctor RC.iregion()
@wrap_ctor RC.iregion(d::Int)
@wrap_ctor RC.iregion(b::IBox)
@wrap_ctor RC.iregion(bs::Vector{IBox})
@wrap_memfun IRegion.boxes()::Vector{IBox}

@wrap_memfun IRegion.valid()::Bool
@wrap_memfun IRegion.rank()::Int
@wrap_memfun IRegion.empty()::Bool
isempty(r::IRegion) = empty(r)
@wrap_memfun IRegion.size()::Int

@wrap_memfun IRegion.bounding_box()::IBox
@wrap_memfun IRegion.__and__(r::IRegion)::IRegion
(&)(r1::IRegion, r2::IRegion) = __and__(r1, r2)
@wrap_memfun IRegion.__sub__(r::IRegion)::IRegion
(-)(r1::IRegion, r2::IRegion) = __sub__(r1, r2)
@wrap_memfun IRegion.__or__(r::IRegion)::IRegion
(|)(r1::IRegion, r2::IRegion) = __or__(r1, r2)
@wrap_memfun IRegion.__xor__(r::IRegion)::IRegion
($)(r1::IRegion, r2::IRegion) = __xor__(r1, r2)
@wrap_memfun IRegion.intersection(r::IRegion)::IRegion
@wrap_memfun IRegion.difference(r::IRegion)::IRegion
@wrap_memfun IRegion.union(r::IRegion)::IRegion
@wrap_memfun IRegion.symmetric_difference(r::IRegion)::IRegion

@wrap_memfun IRegion.contains(p::Vector{Int})::Bool
@wrap_memfun IRegion.isdisjoint(r::IRegion)::Bool

@wrap_memfun IRegion.__le__(r::IRegion)::Bool
<=(r1::IRegion, r2::IRegion) = __le__(r1, r2)
@wrap_memfun IRegion.__ge__(r::IRegion)::Bool
>=(r1::IRegion, r2::IRegion) = __ge__(r1, r2)
@wrap_memfun IRegion.__lt__(r::IRegion)::Bool
<(r1::IRegion, r2::IRegion) = __lt__(r1, r2)
@wrap_memfun IRegion.__gt__(r::IRegion)::Bool
>(r1::IRegion, r2::IRegion) = __gt__(r1, r2)
@wrap_memfun IRegion.issubset(r::IRegion)::Bool
@wrap_memfun IRegion.issuperset(r::IRegion)::Bool
@wrap_memfun IRegion.is_strict_subset(r::IRegion)::Bool
@wrap_memfun IRegion.is_strict_superset(r::IRegion)::Bool
@wrap_memfun IRegion.__eq__(r::IRegion)::Bool
==(r1::IRegion, r2::IRegion) = __eq__(r1, r2)
@wrap_memfun IRegion.__ne__(r::IRegion)::Bool
!=(r1::IRegion, r2::IRegion) = __ne__(r1, r2)

@wrap_memfun IRegion.__str__()::String
show(io::IO, r::IRegion) = print(io, __str__(r))

end



module SimulationIO

using PyCall
using PyCall2

@pyimport SimulationIO as SIO



import Base: rank



@wrap_type CopyObj
@wrap_type DataBlock
@wrap_type DataRange
@wrap_type DataSet
@wrap_type ExtLink

@wrap_type Basis
@wrap_type BasisVector
@wrap_type Configuration
@wrap_type CoordinateField
@wrap_type CoordinateSystem
@wrap_type DiscreteField
@wrap_type DiscreteFieldBlock
@wrap_type DiscreteFieldBlockComponent
@wrap_type Discretization
@wrap_type DiscretizationBlock
@wrap_type Field
@wrap_type Manifold
@wrap_type Parameter
@wrap_type ParameterValue
@wrap_type Project
@wrap_type SubDiscretization
@wrap_type TangentSpace
@wrap_type TensorComponent
@wrap_type TensorType



@wrap_memfum DataBlock.rank()::Int
@wrap_memfum DataBlock.shape()::Vector{Int}
@wrap_memfum DataBlock.npoints()::Int
@wrap_memfum DataBlock.invariant()::Bool

@wrap_memfum DataRange.rank()::Int
@wrap_memfum DataRange.shape()::Vector{Int}
@wrap_memfum DataRange.npoints()::Int
@wrap_memfum DataRange.invariant()::Bool
@wrap_memfum DataRange.origin()::Float64
@wrap_memfum DataRange.delta()::Vector{Float64}

@wrap_memfum DataSet.rank()::Int
@wrap_memfum DataSet.shape()::Vector{Int}
@wrap_memfum DataSet.npoints()::Int
@wrap_memfum DataSet.invariant()::Bool
@wrap_memfun DataSet.writeData_int(data::Vector{Int})::Void
@wrap_memfun DataSet.writeData_double(data::Vector{Float64})::Void

@wrap_memfum CopyObj.rank()::Int
@wrap_memfum CopyObj.shape()::Vector{Int}
@wrap_memfum CopyObj.npoints()::Int
@wrap_memfum CopyObj.invariant()::Bool
@wrap_memfum CopyObj.name()::String
@wrap_memfum CopyObj.readData_int()::Vector{Int}
@wrap_memfum CopyObj.readData_double()::Vector{Float64}

@wrap_memfum ExtLink.rank()::Int
@wrap_memfum ExtLink.shape()::Vector{Int}
@wrap_memfum ExtLink.npoints()::Int
@wrap_memfum ExtLink.invariant()::Bool
@wrap_memfum ExtLink.filename()::String
@wrap_memfum ExtLink.objname()::String



@wrap_memfun Basis.name()::String
@wrap_memfun Basis.tangentspace()::TangentSpace
@wrap_memfun Basis.configuration()::Configuration
@wrap_memfun Basis.basisvectors()::PyMap{String, BasisVector}
@wrap_memfun Basis.directions()::PyMap{Int, BasisVector}

@wrap_memfun Basis.invariant()::Bool
@wrap_memfun Basis.createBasisVector(name::String, dim::Integer)::BasisVector



@wrap_memfun BasisVector.name()::String
@wrap_memfun BasisVector.basis()::Basis
@wrap_memfun BasisVector.direction()::Int

@wrap_memfun BasisVector.invariant()::Bool



@wrap_memfun Configuration.name()::String
@wrap_memfun Configuration.project()::Project
@wrap_memfun Configuration.parametervalues()::PyMap{String, ParameterValue}
@wrap_memfun Configuration.bases()::PyMap{String, PyWeakPtr{Basis}}
@wrap_memfun Configuration.coordinatesystems()::
    PyMap{String, PyWeakPtr{CoordinateSystem}}
@wrap_memfun Configuration.discretefields()::
    PyMap{String, PyWeakPtr{DiscreteField}}
@wrap_memfun Configuration.discretizations()::
    PyMap{String, PyWeakPtr{Discretization}}
@wrap_memfun Configuration.fields()::PyMap{String, PyWeakPtr{Field}}
@wrap_memfun Configuration.manifolds()::PyMap{String, PyWeakPtr{Manifold}}
@wrap_memfun Configuration.tangentspaces()::
    PyMap{String, PyWeakPtr{TangentSpace}}

@wrap_memfun Configuration.invariant()::Bool
@wrap_memfun Configuration.insertParameterValue(
    parametervalue::ParameterValue)::Void



@wrap_memfun CoordinateField.name()::String
@wrap_memfun CoordinateField.coordinatesystem()::CoordinateSystem
@wrap_memfun CoordinateField.direction()::Int
@wrap_memfun CoordinateField.field()::Field

@wrap_memfun CoordinateField.invariant()::Bool



@wrap_field CoordinateSystem.name::String
@wrap_field CoordinateSystem.project::PyWeakPtr{Project}
@wrap_field CoordinateSystem.configuration::Configuration
@wrap_field CoordinateSystem.manifold::Manifold
@wrap_field CoordinateSystem.coordinatefields::
    PyMap{String, CoordinateField}
@wrap_field CoordinateSystem.directions::PyMap{Int, CoordinateField}

@wrap_memfun CoordinateSystem.invariant()::Bool
@wrap_memfun CoordinateSystem.createCoordinateField(
    name::String,
    direction::Int,
    coordinatefield::Field)::CoordinateField



@wrap_field DiscreteField.name::String
@wrap_field DiscreteField.field::PyWeakPtr{Field}
@wrap_field DiscreteField.configuration::Configuration
@wrap_field DiscreteField.discretization::Discretization
@wrap_field DiscreteField.basis::Basis
@wrap_field DiscreteField.discretefieldblocks::
    PyMap{String, DiscreteFieldBlock}

@wrap_memfun DiscreteField.invariant()::Bool
@wrap_memfun DiscreteField.createDiscreteFieldBlock(
    name::String,
    discretizationblock::DiscretizationBlock)::DiscreteFieldBlock



@wrap_field DiscreteFieldBlock.name::String
@wrap_field DiscreteFieldBlock.discretefield::PyWeakPtr{DiscreteField}
@wrap_field DiscreteFieldBlock.discretizationblock::DiscretizationBlock
@wrap_field DiscreteFieldBlock.discretefieldblockcomponents::
    PyMap{String, DiscreteFieldBlockComponent}

@wrap_memfun DiscreteFieldBlock.invariant()::Bool
@wrap_memfun DiscreteFieldBlock.createDiscreteFieldBlockComponent(
    name::String,
    tensorcomponent::TensorComponent)::DiscreteFieldBlockComponent



@wrap_field DiscreteFieldBlockComponent.name::String
@wrap_field DiscreteFieldBlockComponent.discretefieldblock::
    PyWeakPtr{DiscreteFieldBlock}
@wrap_field DiscreteFieldBlockComponent.tensorcomponent::TensorComponent

@wrap_memfun DiscreteFieldBlockComponent.invariant()::Bool
@wrap_memfun DiscreteFieldBlockComponent.datablock()::DataBlock
@wrap_memfun DiscreteFieldBlockComponent.datarange()::DataRange
@wrap_memfun DiscreteFieldBlockComponent.dataset()::DataSet
@wrap_memfun DiscreteFieldBlockComponent.copyobj()::CopyObj
@wrap_memfun DiscreteFieldBlockComponent.extlink()::ExtLink



@wrap_field Discretization.name::String
@wrap_field Discretization.manifold::PyWeakPtr{Manifold}
@wrap_field Discretization.configuration::Configuration
@wrap_field Discretization.discretizationblocks::
    PyMap{String, DiscretizationBlock}
@wrap_field Discretization.child_discretizations::
    PyMap{String, PyWeakPtr{SubDiscretization}}
@wrap_field Discretization.parent_discretizations::
    PyMap{String, PyWeakPtr{SubDiscretization}}

@wrap_memfun Discretization.invariant()::Bool
@wrap_memfun Discretization.createDiscretizationBlock(
    name::String)::DiscretizationBlock



@wrap_field DiscretizationBlock.name::String
@wrap_field DiscretizationBlock.box::IBox
#TODO @wrap_field DiscretizationBlock.active::IRegion
@wrap_field DiscretizationBlock.discretization::PyWeakPtr{Discretization}

@wrap_memfun DiscretizationBlock.invariant()::Bool
@wrap_memfun DiscretizationBlock.setBox(
    ioffset::Vector{Int},
    ishape::Vector{Int})::Void



@wrap_field Field.name::String
@wrap_field Field.project::PyWeakPtr{Project}
@wrap_field Field.configuration::Configuration
@wrap_field Field.manifold::Manifold
@wrap_field Field.tangentspace::TangentSpace
@wrap_field Field.tensortype::TensorType
@wrap_field Field.discretefields::PyMap{String, DiscreteField}

@wrap_memfun Field.invariant()::Bool
@wrap_memfun Field.createDiscreteField(
    name::String,
    configuration::Configuration,
    discretization::Discretization,
    basis::Basis)::DiscreteField



@wrap_field Manifold.name::String
@wrap_field Manifold.project::PyWeakPtr{Project}
@wrap_field Manifold.configuration::Configuration
@wrap_field Manifold.dimension::Int
@wrap_field Manifold.discretizations::PyMap{String, Discretization}
@wrap_field Manifold.fields::PyMap{String, PyWeakPtr{Field}}
@wrap_field Manifold.coordinatesystems::
    PyMap{String, PyWeakPtr{CoordinateSystem}}

@wrap_memfun Manifold.invariant()::Bool
@wrap_memfun Manifold.createDiscretization(
    name::String,
    configuration::Configuration)::Discretization
@wrap_memfun Manifold.createSubDiscretization(
    name::String,
    parent_discretization::Discretization,
    child_discretization::Discretization,
    factor::Vector{Float64},
    offset::Vector{Float64})::SubDiscretization



@wrap_field Parameter.name::String
@wrap_field Parameter.project::PyWeakPtr{Project}
@wrap_field Parameter.parametervalues::PyMap{String, ParameterValue}

@wrap_memfun Parameter.invariant()::Bool
@wrap_memfun Parameter.createParameterValue(
    name::String)::ParameterValue



@wrap_field ParameterValue.name::String
@wrap_field ParameterValue.parameter::PyWeakPtr{Parameter}
@wrap_field ParameterValue.configurations::
    PyMap{String, PyWeakPtr{Configuration}}

@wrap_memfun ParameterValue.invariant()::Bool



@wrap_field Project.name::String
@wrap_field Project.parameters::PyMap{String, Parameter}
@wrap_field Project.configurations::PyMap{String, Configuration}
@wrap_field Project.tensortypes::PyMap{String, TensorType}
@wrap_field Project.manifolds::PyMap{String, Manifold}
@wrap_field Project.tangentspaces::PyMap{String, TangentSpace}
@wrap_field Project.fields::PyMap{String, Field}
@wrap_field Project.coordinatesystems::PyMap{String, CoordinateSystem}

@wrap_memfun Project.invariant()::Bool
@wrap_memfun Project.createStandardTensorTypes()::Void
#TODO @wrap_memfun Project.write(loc::H5.CommonFG)::Void
@wrap_memfun Project.createParameter(name::String)::Parameter
@wrap_memfun Project.createConfiguration(name::String)::Configuration
@wrap_memfun Project.createTensorType(
    name::String,
    dimension::Integer,
    rank::Integer)::TensorType
@wrap_memfun Project.createManifold(
    name::String,
    configuration::Configuration,
    dimension::Integer)::Manifold
@wrap_memfun Project.createTangentSpace(
    name::String,
    configuration::Configuration,
    dimension::Integer)::TangentSpace
@wrap_memfun Project.createField(
    name::String,
    configuration::Configuration,
    manifold::Manifold,
    tangentspace::TangentSpace,
    tensortype::TensorType)::Field
@wrap_memfun Project.createCoordinateSystem(
    name::String,
    configuration::Configuration,
    manifold::Manifold)::CoordinateSystem

export createProject, readProject
createProject(name::String) = Project(SIO.createProject(name))
#TODO readProject(file::H5.CommonFG) = Project(SIO.readProject(file))



@wrap_field SubDiscretization.name::String
@wrap_field SubDiscretization.manifold::PyWeakPtr{Manifold}
@wrap_field SubDiscretization.parent_discretization::Discretization
@wrap_field SubDiscretization.child_discretization::Discretization
@wrap_field SubDiscretization.factor::Vector{Float64}
@wrap_field SubDiscretization.offset::Vector{Float64}

@wrap_memfun SubDiscretization.invariant()::Bool
@wrap_memfun SubDiscretization.child2parent(
    child_idx::Vector{Float64})::Vector{Float64}
@wrap_memfun SubDiscretization.parent2child(
    child_idx::Vector{Float64})::Vector{Float64}



@wrap_field TangentSpace.name::String
@wrap_field TangentSpace.project::PyWeakPtr{Project}
@wrap_field TangentSpace.configuration::Configuration
@wrap_field TangentSpace.dimension::Int
@wrap_field TangentSpace.bases::PyMap{String, Basis}
@wrap_field TangentSpace.fields::PyMap{String, PyWeakPtr{Field}}

@wrap_memfun TangentSpace.invariant()::Bool
@wrap_memfun TangentSpace.createBasis(
    name::String,
    configuration::Configuration)::Basis



@wrap_field TensorComponent.name::String
@wrap_field TensorComponent.tensortype::PyWeakPtr{TensorType}
@wrap_field TensorComponent.storage_index::Int
@wrap_field TensorComponent.indexvalues::Vector{Int}

@wrap_memfun TensorComponent.invariant()::Bool



@wrap_field TensorType.name::String
@wrap_field TensorType.project::PyWeakPtr{Project}
@wrap_field TensorType.dimension::Int
@wrap_field TensorType.rank::Int
@wrap_field TensorType.tensorcomponents::
    PyMap{String, TensorComponent}
@wrap_field TensorType.storage_indices::PyMap{Int, TensorComponent}

@wrap_memfun TensorType.invariant()::Bool

end
