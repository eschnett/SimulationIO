module H5

using PyCall
@pyimport H5

export H5F_ACC_RDONLY
const H5F_ACC_RDONLY = H5.H5F_ACC_RDONLY

export H5File
H5File(filename::AbstractString, acc) = H5.H5File(filename, acc)

end



module SimulationIO

using PyCall
@pyimport SimulationIO as SIO

immutable PseudoDict
    obj::PyObject
end
import Base: getindex
getindex(dict::PseudoDict, key) = get(dict.obj, key)



export readProject
readProject(file) = SIO.readProject(file)

for attr in (
        :data_dataset,
        :discretefieldblockcomponents,
        :discretefieldblocks,
        :discretefields,
        :field,
        :fields,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject) = PseudoDict(obj[symbol($fun)])
    end
end

for fun0 in (
        :getName,
        :getPath,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject) = PseudoDict(obj[symbol($fun)]())
    end
end

for fun1 in (
        :createConfiguration,
        :createDiscretizationBlock,
        :make,
        :storage_indices,
        :write,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject, arg1) = PseudoDict(obj[symbol($fun)](arg1))
    end
end

for fun2 in (
        :createBasis,
        :createBasisVector,
        :createDiscreteFieldBlock,
        :createDiscreteFieldBlockComponent,
        :setRegion,
        :writeData_double,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject, arg1, arg2) =
            PseudoDict(obj[symbol($fun)](arg1, arg2))
    end
end

for fun3 in (
        :createCoordinateField,
        :createCoordinateSystem,
        :createManifold,
        :createTangentSpace,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject, arg1, arg2, arg3) =
            PseudoDict(obj[symbol($fun)](arg1, arg2, arg3))
    end
end

for fun4 in (
        :createDiscreteField,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject, arg1, arg2, arg3, arg4) =
            PseudoDict(obj[symbol($fun)](arg1, arg2, arg3, arg4))
    end
end

for fun5 in (
        :createField,
        )
    @eval begin
        export $fun
        $fun(obj::PyObject, arg1, arg2, arg3, arg4, arg5) =
            PseudoDict(obj[symbol($fun)](arg1, arg2, arg3, arg4, arg5))
    end
end
