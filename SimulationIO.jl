module PyCall2

export PyMap, PyWeakPtr
export @wrap_type, @wrap_field, @wrap_memfun

using PyCall

immutable PyMap{K,V}
    pyobj::PyObject
end
import Base: isempty, length
isempty(map::PyMap) = isempty(map.pyobj)
length(map::PyMap) = length(map.pyobj)
import Base: getindex, haskey
getindex{K,V}(map::PyMap{K,V}, key) = V(map_get(map.pyobj, K(key)))
haskey(map::PyMap, key) = map.pyobj[:count](K(key)) != 0
#TODO import Base: start, next
#TODO start{K,V}(map::PyMap{K,V}) = ... state
#TODO next{K,V}(map::PyMap{K,V}, state) = ... item, state
#TODO
#TODO collect{K,V}(map::PyMap{K,V}) =
#TODO keys
#TODO values
#TODO collect ... Pair

immutable PyWeakPtr{T}
    pyobj::PyObject
end
import Base: isnull, get
isnull{T}(ptr::PyWeakPtr{T}) = ptr.pyobj[:expired]()::Bool
function get{T}(ptr::PyWeakPtr{T})
    @assert !isnull(ptr)
    T(ptr.pyobj[:lock]())
 end
# get{T}(ptr::PyWeakPtr{T}, other) = T(if !isnull, other ...)

unwrap(obj) = obj

macro wrap_type(decl::Symbol)
    typesym::Symbol = decl
    esc(quote
        export $typesym
        immutable $typesym
            pyobj::PyObject
        end
        PyCall2.unwrap(obj::$typesym) = obj.pyobj
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
            $rettype(obj.pyobj[$(QuoteNode(fieldsym))])
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
        $(Expr(:call, memfunsym, :(obj::$objtypesym),
                declargs...)) =
            $rettypesym(
                $(Expr(:call, :(obj.pyobj[$(QuoteNode(memfunsym))]),
                    callargs...)))
    end)
end

end



module H5

using PyCall
@pyimport H5

export H5F_ACC_RDONLY
#TODO const H5F_ACC_RDONLY = H5.H5F_ACC_RDONLY

export H5File
H5File(filename::AbstractString, acc) = H5.H5File(filename, acc)

end



module SimulationIO

using PyCall
using PyCall2

@pyimport SimulationIO as SIO



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



@wrap_field Basis.name::AbstractString
@wrap_field Basis.tangentspace::PyWeakPtr{TangentSpace}
@wrap_field Basis.configuration::Configuration
@wrap_field Basis.basisvectors::PyMap{AbstractString, BasisVector}
@wrap_field Basis.directions::PyMap{Int, BasisVector}

@wrap_memfun Basis.invariant()::Bool
@wrap_memfun Basis.createBasisVector(
    name::AbstractString,
    dim::Integer)::BasisVector



@wrap_field BasisVector.name::AbstractString
@wrap_field BasisVector.basis::PyWeakPtr{Basis}
@wrap_field BasisVector.direction::Int

@wrap_memfun BasisVector.invariant()::Bool



@wrap_field Configuration.name::AbstractString
@wrap_field Configuration.project::PyWeakPtr{Project}
@wrap_field Configuration.parametervalues::
    PyMap{AbstractString, ParameterValue}
@wrap_field Configuration.bases::PyMap{AbstractString, PyWeakPtr{Basis}}
@wrap_field Configuration.coordinatesystems::
    PyMap{AbstractString, PyWeakPtr{CoordinateSystem}}
@wrap_field Configuration.discretefields::
    PyMap{AbstractString, PyWeakPtr{DiscreteField}}
@wrap_field Configuration.discretizations::
    PyMap{AbstractString, PyWeakPtr{Discretization}}
@wrap_field Configuration.fields::PyMap{AbstractString, PyWeakPtr{Field}}
@wrap_field Configuration.manifolds::PyMap{AbstractString, PyWeakPtr{Manifold}}
@wrap_field Configuration.tangentspaces::
    PyMap{AbstractString, PyWeakPtr{TangentSpace}}

@wrap_memfun Configuration.invariant()::Bool
@wrap_memfun Configuration.insertParameterValue(
    parametervalue::ParameterValue)::Void



@wrap_field CoordinateField.name::AbstractString
@wrap_field CoordinateField.coordinatesystem::PyWeakPtr{CoordinateSystem}
@wrap_field CoordinateField.direction::Int
@wrap_field CoordinateField.field::Field

@wrap_memfun CoordinateField.invariant()::Bool



@wrap_field CoordinateSystem.name::AbstractString
@wrap_field CoordinateSystem.project::PyWeakPtr{Project}
@wrap_field CoordinateSystem.configuration::Configuration
@wrap_field CoordinateSystem.manifold::Manifold
@wrap_field CoordinateSystem.coordinatefields::
    PyMap{AbstractString, CoordinateField}
@wrap_field CoordinateSystem.directions::PyMap{Int, CoordinateField}

@wrap_memfun CoordinateSystem.invariant()::Bool
@wrap_memfun CoordinateSystem.createCoordinateField(
    name::AbstractString,
    direction::Int,
    coordinatefield::Field)::CoordinateField



@wrap_field DiscreteField.name::AbstractString
@wrap_field DiscreteField.field::PyWeakPtr{Field}
@wrap_field DiscreteField.configuration::Configuration
@wrap_field DiscreteField.discretization::Discretization
@wrap_field DiscreteField.basis::Basis
@wrap_field DiscreteField.discretefieldblocks::
    PyMap{AbstractString, DiscreteFieldBlock}

@wrap_memfun DiscreteField.invariant()::Bool
@wrap_memfun DiscreteField.createDiscreteFieldBlock(
    name::AbstractString,
    discretizationblock::DiscretizationBlock)::DiscreteFieldBlock



@wrap_field DiscreteFieldBlock.name::AbstractString
@wrap_field DiscreteFieldBlock.discretefield::PyWeakPtr{DiscreteField}
@wrap_field DiscreteFieldBlock.discretizationblock::DiscretizationBlock
@wrap_field DiscreteFieldBlock.discretefieldblockcomponents::
    PyMap{AbstractString, DiscreteFieldBlockComponent}

@wrap_memfun DiscreteFieldBlock.invariant()::Bool
@wrap_memfun DiscreteFieldBlock.createDiscreteFieldBlockComponent(
    name::AbstractString,
    tensorcomponent::TensorComponent)::DiscreteFieldBlockComponent



@wrap_field DiscreteFieldBlockComponent.name::AbstractString
@wrap_field DiscreteFieldBlockComponent.discretefieldblock::
    PyWeakPtr{DiscreteFieldBlock}
@wrap_field DiscreteFieldBlockComponent.tensorcomponent::TensorComponent
@wrap_field DiscreteFieldBlockComponent.data_dataset::H5.DataSet

@wrap_memfun DiscreteFieldBlockComponent.invariant()::Bool
@wrap_memfun DiscreteFieldBlockComponent.setData()::Void
#TODO @wrap_memfun DiscreteFieldBlockComponent.setData(
#TODO     datatype::H5.DataType,
#TODO     dataspace::H5.DataSpace)::Void
#TODO @wrap_memfun DiscreteFieldBlockComponent.getData_DataSet()::H5.DataSet
@wrap_memfun DiscreteFieldBlockComponent.getPath()::AbstractString
@wrap_memfun DiscreteFieldBlockComponent.getName()::AbstractString
@wrap_memfun DiscreteFieldBlockComponent.writeData_int(data::Vector{Int})::Void
@wrap_memfun DiscreteFieldBlockComponent.writeData_double(
    data::Vector{Float64})::Void



@wrap_field Discretization.name::AbstractString
@wrap_field Discretization.manifold::PyWeakPtr{Manifold}
@wrap_field Discretization.configuration::Configuration
@wrap_field Discretization.discretizationblocks::
    PyMap{AbstractString, DiscretizationBlock}
@wrap_field Discretization.child_discretizations::
    PyMap{AbstractString, PyWeakPtr{SubDiscretization}}
@wrap_field Discretization.parent_discretizations::
    PyMap{AbstractString, PyWeakPtr{SubDiscretization}}

@wrap_memfun Discretization.invariant()::Bool
@wrap_memfun Discretization.createDiscretizationBlock(
    name::AbstractString)::DiscretizationBlock



@wrap_field DiscretizationBlock.name::AbstractString
#TODO @wrap_field DiscretizationBlock.region::IBox
#TODO @wrap_field DiscretizationBlock.active::IRegion
@wrap_field DiscretizationBlock.discretization::PyWeakPtr{Discretization}

@wrap_memfun DiscretizationBlock.invariant()::Bool
@wrap_memfun DiscretizationBlock.setRegion(
    ioffset::Vector{Int},
    ishape::Vector{Int})::Void



@wrap_field Field.name::AbstractString
@wrap_field Field.project::PyWeakPtr{Project}
@wrap_field Field.configuration::Configuration
@wrap_field Field.manifold::Manifold
@wrap_field Field.tangentspace::TangentSpace
@wrap_field Field.tensortype::TensorType
@wrap_field Field.discretefields::PyMap{AbstractString, DiscreteField}

@wrap_memfun Field.invariant()::Bool
@wrap_memfun Field.createDiscreteField(
    name::AbstractString,
    configuration::Configuration,
    discretization::Discretization,
    basis::Basis)::DiscreteField



@wrap_field Manifold.name::AbstractString
@wrap_field Manifold.project::PyWeakPtr{Project}
@wrap_field Manifold.configuration::Configuration
@wrap_field Manifold.dimension::Int
@wrap_field Manifold.discretizations::PyMap{AbstractString, Discretization}
@wrap_field Manifold.fields::PyMap{AbstractString, PyWeakPtr{Field}}
@wrap_field Manifold.coordinatesystems::
    PyMap{AbstractString, PyWeakPtr{CoordinateSystem}}

@wrap_memfun Manifold.invariant()::Bool
@wrap_memfun Manifold.createDiscretization(
    name::AbstractString,
    configuration::Configuration)::Discretization
@wrap_memfun Manifold.createSubDiscretization(
    name::AbstractString,
    parent_discretization::Discretization,
    child_discretization::Discretization,
    factor::Vector{Float64},
    offset::Vector{Float64})::SubDiscretization



@wrap_field Parameter.name::AbstractString
@wrap_field Parameter.project::PyWeakPtr{Project}
@wrap_field Parameter.parametervalues::PyMap{AbstractString, ParameterValue}

@wrap_memfun Parameter.invariant()::Bool
@wrap_memfun Parameter.createParameterValue(
    name::AbstractString)::ParameterValue



@wrap_field ParameterValue.name::AbstractString
@wrap_field ParameterValue.parameter::PyWeakPtr{Parameter}
@wrap_field ParameterValue.configurations::
    PyMap{AbstractString, PyWeakPtr{Configuration}}

@wrap_memfun ParameterValue.invariant()::Bool



@wrap_field Project.name::AbstractString
@wrap_field Project.parameters::PyMap{AbstractString, Parameter}
@wrap_field Project.configurations::PyMap{AbstractString, Configuration}
@wrap_field Project.tensortypes::PyMap{AbstractString, TensorType}
@wrap_field Project.manifolds::PyMap{AbstractString, Manifold}
@wrap_field Project.tangentspaces::PyMap{AbstractString, TangentSpace}
@wrap_field Project.fields::PyMap{AbstractString, Field}
@wrap_field Project.coordinatesystems::PyMap{AbstractString, CoordinateSystem}

@wrap_memfun Project.invariant()::Bool
@wrap_memfun Project.createStandardTensorTypes()::Void
#TODO @wrap_memfun Project.write(loc::H5.CommonFG)::Void
@wrap_memfun Project.createParameter(name::AbstractString)::Parameter
@wrap_memfun Project.createConfiguration(name::AbstractString)::Configuration
@wrap_memfun Project.createTensorType(
    name::AbstractString,
    dimension::Integer,
    rank::Integer)::TensorType
@wrap_memfun Project.createManifold(
    name::AbstractString,
    configuration::Configuration,
    dimension::Integer)::Manifold
@wrap_memfun Project.createTangentSpace(
    name::AbstractString,
    configuration::Configuration,
    dimension::Integer)::TangentSpace
@wrap_memfun Project.createField(
    name::AbstractString,
    configuration::Configuration,
    manifold::Manifold,
    tangentspace::TangentSpace,
    tensortype::TensorType)::Field
@wrap_memfun Project.createCoordinateSystem(
    name::AbstractString,
    configuration::Configuration,
    manifold::Manifold)::CoordinateSystem

export createProject, readProject
createProject(name::AbstractString) = Project(SIO.createProject(name))
#TODO readProject(file::H5.CommonFG) = Project(SIO.readProject(file))



@wrap_field SubDiscretization.name::AbstractString
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



@wrap_field TangentSpace.name::AbstractString
@wrap_field TangentSpace.project::PyWeakPtr{Project}
@wrap_field TangentSpace.configuration::Configuration
@wrap_field TangentSpace.dimension::Int
@wrap_field TangentSpace.bases::PyMap{AbstractString, Basis}
@wrap_field TangentSpace.fields::PyMap{AbstractString, PyWeakPtr{Field}}

@wrap_memfun TangentSpace.invariant()::Bool
@wrap_memfun TangentSpace.createBasis(
    name::AbstractString,
    configuration::Configuration)::Basis



@wrap_field TensorComponent.name::AbstractString
@wrap_field TensorComponent.tensortype::PyWeakPtr{TensorType}
@wrap_field TensorComponent.storage_index::Int
@wrap_field TensorComponent.indexvalues::Vector{Int}

@wrap_memfun TensorComponent.invariant()::Bool



@wrap_field TensorType.name::AbstractString
@wrap_field TensorType.project::PyWeakPtr{Project}
@wrap_field TensorType.dimension::Int
@wrap_field TensorType.rank::Int
@wrap_field TensorType.tensorcomponents::
    PyMap{AbstractString, TensorComponent}
@wrap_field TensorType.storage_indices::PyMap{Int, TensorComponent}

@wrap_memfun TensorType.invariant()::Bool

end
