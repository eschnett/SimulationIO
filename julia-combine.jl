#! /usr/bin/env julia

using PyCall

# Add current directory to Python module search path
unshift!(PyVector(pyimport("sys")["path"]), "")

@pyimport RegionCalculus as RC
@pyimport SimulationIO as SIO

using HDF5



const indent_level = Ref{Int}(0)
indent() = indent_level[] += 2
outdent() = indent_level[] -= 2
message(msgs...) = println(" "^indent_level[], msgs...)



# Combine multiple discretization boxes into one
filename = "cactus/iof5-uniform-p2.s5"
filename2 = "cactus/iof5-uniform-p2-combined.s5"

# Read project
message("Reading project...")
project = SIO.readProjectHDF5(filename)
indent()
message("Project \"$(project[:name]())\"")
outdent()
file[:close]()

# Create project
message()
message("Copying metadata...")
project2 = SIO.createProject(project[:name]())

# Copy parameters
indent()
for parameter in project[:parameters]()[:values]()
    message("Parameter \"$(parameter[:name]())\"")
    parameter2 = project2[:createParameter](parameter[:name]())

    indent()
    for parametervalue in parameter[:parametervalues]()[:values]()
        message("ParameterValue \"$(parametervalue[:name]())\"")
        parametervalue2 =
            parameter2[:createParameterValue](parametervalue[:name]())
        # TODO: Set actual parameter value
    end
    outdent()
end
outdent()

# Copy configurations
indent()
for configuration in project[:configurations]()[:values]()
    message("Configuration \"$(configuration[:name]())\"")
    configuration2 = project2[:createConfiguration](configuration[:name]())

    indent()
    for parametervalue in configuration[:parametervalues]()[:values]()
        message("ParameterValue \"$(parametervalue[:name]())\"")
        parameter = parametervalue[:parameter]()
        parameter2 = get(project2[:parameters](), parameter[:name]())
        parametervalue2 =
            get(parameter2[:parametervalues](), parametervalue[:name]())
        configuration2[:insertParameterValue](parametervalue2)
    end
    outdent()
end

# Copy tensor types
for tensortype in project[:tensortypes]()[:values]()
    message("TensorType \"$(tensortype[:name]())\"")
    tensortype2 =
        project2[:createTensorType](tensortype[:name](),
            tensortype[:dimension](), tensortype[:rank]())

    indent()
    for tensorcomponent in tensortype[:tensorcomponents]()[:values]()
        message("TensorComponent \"$(tensorcomponent[:name]())\"")
        tensorcomponent2 =
            tensortype2[:createTensorComponent](tensorcomponent[:name](),
                tensorcomponent[:storage_index](),
                tensorcomponent[:indexvalues]())
    end
    outdent()
end
outdent()

# Copy manifolds
indent()
for manifold in project[:manifolds]()[:values]()
    message("Manifold \"$(manifold[:name]())\"")
    configuration2 =
        get(project2[:configurations](), manifold[:configuration]()[:name]())
    manifold2 =
        project2[:createManifold](manifold[:name](), configuration2,
            manifold[:dimension]())

    indent()
    for discretization in manifold[:discretizations]()[:values]()
        message("Discretization \"$(discretization[:name]())\"")
        configuration2 =
            get(project2[:configurations](),
                discretization[:configuration]()[:name]())
        discretization2 =
            manifold2[:createDiscretization](discretization[:name](),
                configuration2)
        # TODO: Handle subdiscretizations

        # Read and combine discretizationblocks
        boxes = []
        actives = []
        indent()
        for discretizationblock in
                discretization[:discretizationblocks]()[:values]()
            message("DiscretizationBlock \"$(discretizationblock[:name]())\"")
            push!(boxes, discretizationblock[:box]())
            push!(actives, discretizationblock[:active]())
        end
        outdent()
        combined_box = RC.ibox(manifold[:dimension]())
        for box in boxes
            combined_box = combined_box[:bounding_box](box)
        end
        message("combined_box: $(combined_box[:__str__]())")
        combined_active = RC.iregion(manifold[:dimension]())
        for active in actives
            if active[:valid]()
                combined_active = combined_active[:union](active)
            end
        end
        message("combined_active: $(combined_active[:__str__]())")

        discretizationblock2 =
            discretization2[:createDiscretizationBlock]("discretizationblock")
        discretizationblock2[:setBox](combined_box)
        discretizationblock2[:setActive](combined_active)
    end
    outdent()
end
outdent()

# Copy tangentspaces
indent()
for tangentspace in project[:tangentspaces]()[:values]()
    message("TangentSpace \"$(tangentspace[:name]())\"")
    configuration2 =
        get(project2[:configurations](),
            tangentspace[:configuration]()[:name]())
    tangentspace2 =
        project2[:createTangentSpace](tangentspace[:name](), configuration2,
            tangentspace[:dimension]())

    indent()
    for basis in tangentspace[:bases]()[:values]()
        message("Basis \"$(basis[:name]())\"")
        configuration2 =
            get(project2[:configurations](), basis[:configuration]()[:name]())
        basis2 = tangentspace2[:createBasis](basis[:name](), configuration2)
    end
    outdent()
end
outdent()

# Copy fields
indent()
for field in project[:fields]()[:values]()
    message("Field \"$(field[:name]())\"")
    configuration2 =
        get(project2[:configurations](), field[:configuration]()[:name]())
    manifold2 = get(project2[:manifolds](), field[:manifold]()[:name]())
    tangentspace2 = get(project2[:tangentspaces](),
        field[:tangentspace]()[:name]())
    tensortype2 = get(project2[:tensortypes](), field[:tensortype]()[:name]())
    field2 =
        project2[:createField](field[:name](), configuration2, manifold2,
            tangentspace2, tensortype2)

    indent()
    for discretefield in field[:discretefields]()[:values]()
        message("DiscreteField \"$(discretefield[:name]())\"")
        configuration2 =
            get(project2[:configurations](),
                discretefield[:configuration]()[:name]())
        discretization2 =
            get(manifold2[:discretizations](),
                discretefield[:discretization]()[:name]())
        basis2 = get(tangentspace2[:bases](), discretefield[:basis]()[:name]())
        discretefield2 =
            field2[:createDiscreteField](discretefield[:name](),
                configuration2, discretization2, basis2)

        if !isempty(discretefield[:discretefieldblocks]())
            discretizationblock2 =
                get(discretization2[:discretizationblocks](),
                    "discretizationblock")
            discretefieldblock2 =
                discretefield2[:createDiscreteFieldBlock](
                    "discretefieldblock", discretizationblock2)

            for tensorcomponent2 in tensortype2[:tensorcomponents]()[:values]()
                discretefieldblockcomponent2 =
                    discretefieldblock2[:createDiscreteFieldBlockComponent](
                        tensorcomponent2[:name](), tensorcomponent2)
            end
        end

    end
    outdent()
end
outdent()

# Copy coordinatesystems
indent()
for coordinatesystem in project[:coordinatesystems]()[:values]()
    message("CoordinateSystem \"$(coordinatesystem[:name]())\"")
    configuration2 =
        get(project2[:configurations](),
            coordinatesystem[:configuration]()[:name]())
    manifold2 = get(project2[:manifolds](),
                    coordinatesystem[:manifold]()[:name]())
    coordinatesystem2 =
        project2[:createCoordinateSystem](coordinatesystem[:name](),
            configuration2, manifold2)

    indent()
    for coordinatefield in coordinatesystem[:coordinatefields]()[:values]()
        message("CoordinateField \"$(coordinatefield[:name]())\"")
        field2 = get(project2[:fields](), coordinatefield[:field]()[:name]())
        coordinatefield2 =
            coordinatesystem2[:createCoordinateField](coordinatefield[:name](),
                coordinatefield[:direction](), field2)
    end
    outdent()
end
outdent()

# Write project
project2[:writeHDF5](filename2)

message()
message("Copying data..")

hfile = h5open(filename, "r")
hfile2 = h5open(filename2, "r+",
                "fclose_degree", HDF5.H5F_CLOSE_STRONG,
                "libver_bounds",
                (HDF5.H5F_LIBVER_LATEST, HDF5.H5F_LIBVER_LATEST))



# Copy and combine datasets
indent()
for field2 in project2[:fields]()[:values]()
    message("Field \"$(field2[:name]())\"")
    field = get(project[:fields](), field2[:name]())

    indent()
    for discretefield2 in field2[:discretefields]()[:values]()
        message("DiscreteField \"$(discretefield2[:name]())\"")
        discretefield = get(field[:discretefields](), discretefield2[:name]())

        # Loop over datasets to be written
        indent()
        for discretefieldblock2 in
                discretefield2[:discretefieldblocks]()[:values]()
            discretizationblock2 = discretefieldblock2[:discretizationblock]()

            for discretefieldblockcomponent2 in
                    discretefieldblock2[:discretefieldblockcomponents]()[
                        :values]()
                tensorcomponent2 =
                    discretefieldblockcomponent2[:tensorcomponent]()
                message("TensorComponent \"$(tensorcomponent2[:name]())\"")

                # Create dataset
                discretefieldblockcomponent2[:setData_double]()
                path2 = discretefieldblockcomponent2[:getPath]()
                name2 = discretefieldblockcomponent2[:getName]()
                # message("path2=$path2 name2=$name2")
                lower2 = discretizationblock2[:box]()[:lower]()
                upper2 = discretizationblock2[:box]()[:upper]()
                shape2 = discretizationblock2[:box]()[:shape]()
                message("combined_box=$lower2:$upper2")
                # Guessing a good chunk size:
                # - chunk should fit in L3 cache
                # - chunk should be larger than stripe size
                # - chunk should be larger than bandwidth-latency product
                # A typical L3 cache size is several MByte
                # A typical stripe size is 128 kByte
                # A typical disk latency-bandwidth product is
                #    L = 1 / (10,000/min) / 2 = 0.006 sec
                #    BW = 100 MByte/sec
                #    BW * L = 600 kByte
                # We choose a chunk size of 64^3:
                #    64^3 * 8 B = 2 MByte
                # Maybe 32^3 would be a better size?
                chunksize = [min(64, sz) for sz in shape2]
                # chunksize = HDF5.heuristic_chunk(Float64, shape2)
                # shuffling improves compression
                # level 1 is fast, but still offers good compression
                clevel = 1
                dataset =
                    d_create(hfile2, "$path2/$name2",
                        datatype(Float64), dataspace(shape2...),
                        "chunk", chunksize, "shuffle", (), "compress", clevel)

                # Loop over datasets to be read
                indent()
                for discretefieldblock in
                        discretefield[:discretefieldblocks]()[:values]()
                    message("DiscreteFieldBlock " *
                        "\"$(discretefieldblock[:name]())\"")
                    discretizationblock =
                        discretefieldblock[:discretizationblock]()

                    discretefieldblockcomponent =
                        get(discretefieldblock[:storage_indices](),
                            tensorcomponent2[:storage_index]())
                    message("DiscreteFieldBlockComponent " *
                        "\"$(discretefieldblockcomponent[:name]())\"")

                    tensorcomponent =
                        discretefieldblockcomponent[:tensorcomponent]()
                    @assert tensorcomponent[:storage_index]() ==
                        tensorcomponent2[:storage_index]()

                    # Read dataset
                    path = discretefieldblockcomponent[:getPath]()
                    name = discretefieldblockcomponent[:getName]()
                    # message("path=$path name=$name")
                    lower = discretizationblock[:box]()[:lower]()
                    upper = discretizationblock[:box]()[:upper]()
                    shape = discretizationblock[:box]()[:shape]()
                    message("box=$lower:$upper")
                    data = read(hfile, "$path/$name")
                    @assert size(data) == shape

                    # Write into dataset
                    dataset[lower[1]+1:upper[1],
                            lower[2]+1:upper[2],
                            lower[3]+1:upper[3]] = data
                end
                outdent()

                close(dataset)

            end
        end
        outdent()

    end
    outdent()
end
outdent()

close(hfile2)
close(hfile)

message()
message("Done.")
