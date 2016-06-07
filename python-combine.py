#! /usr/bin/env python

from math import *
import os
import re
import sys

import numpy as np

import H5
import RegionCalculus as RC
import SimulationIO as SIO



# Combine multiple discretization boxes into one

include_parameters = {}

print sys.argv
if len(sys.argv) not in {3, 4}:
    print "Synopsis:"
    print "    %s [--iteration=<iter>] <inputfile> <outputfile>" % sys.argv[0]
    sys.exit(1)

argc = 1
if sys.argv[argc].startswith("-"):
    m = re.match(r"--iteration=(\d+)", sys.argv[argc])
    if m:
        iteration = int(m.group(1))
        include_parameters["iteration"] = iteration
    argc += 1

input_filename = sys.argv[argc]
argc += 1
output_filename = sys.argv[argc]
argc += 1

if not os.access(input_filename, os.R_OK):
    print "Error: Input file \"%s\" does not exist" % input_filename
    sys.exit(2)
if os.path.exists(output_filename):
    print "Error: Output file \"%s\" exists already" % output_filename
    sys.exit(3)



indent_level = 0
def indent():
    global indent_level
    indent_level += 2
def outdent():
    global indent_level
    indent_level -= 2
def message(*msgs):
    sys.stdout.write(" "*indent_level)
    for msg in msgs:
        sys.stdout.write(str(msg))
    sys.stdout.write("\n")



# Read project
file = H5.H5File()
file.openFile(input_filename, H5.H5F_ACC_RDONLY, H5.FileAccPropList())
message("Reading project...")
project = SIO.readProject(file)
indent()
message("Project \"%s\"" % project.name())
outdent()

# Create project
message()
message("Copying metadata...")
project2 = SIO.createProject(project.name())

# Copy parameters
indent()
for parameter in project.parameters().values():
    message("Parameter \"%s\"" % parameter.name())
    parameter2 = project2.createParameter(parameter.name())

    indent()
    for parametervalue in parameter.parametervalues().values():
        message("ParameterValue \"%s\"" % parametervalue.name())
        parametervalue2 = \
            parameter2.createParameterValue(parametervalue.name())
        # TODO: Set actual parameter value
    outdent()
outdent()

# Copy configurations
indent()
for configuration in project.configurations().values():
    message("Configuration \"%s\"" % configuration.name())
    configuration2 = project2.createConfiguration(configuration.name())

    indent()
    for parametervalue in configuration.parametervalues().values():
        message("ParameterValue \"%s\"" % parametervalue.name())
        parameter = parametervalue.parameter()
        parameter2 = project2.parameters()[parameter.name()]
        parametervalue2 = \
            parameter2.parametervalues()[parametervalue.name()]
        configuration2.insertParameterValue(parametervalue2)
    outdent()

# Copy tensor types
for tensortype in project.tensortypes().values():
    message("TensorType \"%s\"" % tensortype.name())
    tensortype2 = \
        project2.createTensorType(tensortype.name(), tensortype.dimension(),
                                  tensortype.rank())

    indent()
    for tensorcomponent in tensortype.tensorcomponents().values():
        message("TensorComponent \"%s\"" % tensorcomponent.name())
        tensorcomponent2 = \
            tensortype2.createTensorComponent(tensorcomponent.name(),
                                              tensorcomponent.storage_index(),
                                              tensorcomponent.indexvalues())
    outdent()
outdent()

# Copy manifolds
indent()
for manifold in project.manifolds().values():
    message("Manifold \"%s\"" % manifold.name())
    configuration2 = \
        project2.configurations()[manifold.configuration().name()]
    manifold2 = \
        project2.createManifold(manifold.name(), configuration2,
            manifold.dimension())

    indent()
    for discretization in manifold.discretizations().values():
        message("Discretization \"%s\"" % discretization.name())
        configuration2 = \
            project2.configurations()[
                discretization.configuration().name()]
        discretization2 = \
            manifold2.createDiscretization(discretization.name(),
                configuration2)
        # TODO: Handle subdiscretizations

        # Read and combine discretizationblocks
        boxes = []
        actives = []
        indent()
        for discretizationblock in \
                discretization.discretizationblocks().values():
            message("DiscretizationBlock \"%s\"" % discretizationblock.name())
            boxes.append(discretizationblock.box())
            actives.append(discretizationblock.active())
        outdent()
        combined_box = RC.box_t(manifold.dimension())
        for box in boxes:
            combined_box = combined_box.bounding_box(box)
        message("combined_box: %s" % combined_box)
        combined_active = RC.region_t(manifold.dimension())
        for active in actives:
            if active.valid():
                combined_active = combined_active.union(active)
        message("combined_active: %s" % combined_active)

        discretizationblock2 = \
            discretization2.createDiscretizationBlock("discretizationblock")
        discretizationblock2.setBox(combined_box)
        discretizationblock2.setActive(combined_active)
    outdent()
outdent()

# Copy tangentspaces
indent()
for tangentspace in project.tangentspaces().values():
    message("TangentSpace \"%s\"" % tangentspace.name())
    configuration2 = \
        project2.configurations()[tangentspace.configuration().name()]
    tangentspace2 = \
        project2.createTangentSpace(tangentspace.name(), configuration2,
                                    tangentspace.dimension())

    indent()
    for basis in tangentspace.bases().values():
        message("Basis \"%s\"" % basis.name())
        configuration2 = \
            project2.configurations()[basis.configuration().name()]
        basis2 = tangentspace2.createBasis(basis.name(), configuration2)
    outdent()
outdent()

# Copy fields
indent()
for field in project.fields().values():
    message("Field \"%s\"" % field.name())
    configuration2 = \
        project2.configurations()[field.configuration().name()]
    manifold2 = project2.manifolds()[field.manifold().name()]
    tangentspace2 = project2.tangentspaces()[field.tangentspace().name()]
    tensortype2 = project2.tensortypes()[field.tensortype().name()]
    field2 = \
        project2.createField(field.name(), configuration2, manifold2,
                             tangentspace2, tensortype2)

    indent()
    for discretefield in field.discretefields().values():
        message("DiscreteField \"%s\"" % discretefield.name())
        configuration2 = \
            project2.configurations()[discretefield.configuration().name()]
        discretization2 = \
            manifold2.discretizations()[
                discretefield.discretization().name()]
        basis2 = tangentspace2.bases()[discretefield.basis().name()]
        discretefield2 = \
            field2.createDiscreteField(discretefield.name(),
                                       configuration2, discretization2, basis2)

        if not discretefield.discretefieldblocks().empty():
            discretizationblock2 = \
                discretization2.discretizationblocks()[
                    "discretizationblock"]
            discretefieldblock2 = \
                discretefield2.createDiscreteFieldBlock(
                    "discretefieldblock", discretizationblock2)

            for tensorcomponent2 in tensortype2.tensorcomponents().values():
                discretefieldblockcomponent2 = \
                    discretefieldblock2.createDiscreteFieldBlockComponent(
                        tensorcomponent2.name(), tensorcomponent2)
                dataset2 = discretefieldblockcomponent2.createDataSet_double()

    outdent()
outdent()

# Copy coordinatesystems
indent()
for coordinatesystem in project.coordinatesystems().values():
    message("CoordinateSystem \"%s\"" % coordinatesystem.name())
    configuration2 = \
        project2.configurations()[coordinatesystem.configuration().name()]
    manifold2 = project2.manifolds()[coordinatesystem.manifold().name()]
    coordinatesystem2 = \
        project2.createCoordinateSystem(coordinatesystem.name(),
                                        configuration2, manifold2)

    indent()
    for coordinatefield in coordinatesystem.coordinatefields().values():
        message("CoordinateField \"%s\"" % coordinatefield.name())
        field2 = project2.fields()[coordinatefield.field().name()]
        coordinatefield2 = \
            coordinatesystem2.createCoordinateField(coordinatefield.name(),
                                                    coordinatefield.direction(),
                                                    field2)
    outdent()
outdent()

# Write project
cpl = H5.FileCreatPropList()
apl = H5.FileAccPropList()
apl.setFcloseDegree(H5.H5F_CLOSE_STRONG)
apl.setLibverBounds(H5.H5F_LIBVER_LATEST, H5.H5F_LIBVER_LATEST)
file2 = H5.H5File(output_filename, H5.H5F_ACC_TRUNC, cpl, apl)
project2.write(file2)

message()
message("Copying data..")



# Copy and combine datasets
indent()
for field2 in project2.fields().values():
    message("Field \"%s\"" % field2.name())
    field = project.fields()[field2.name()]

    indent()
    for discretefield2 in field2.discretefields().values():
        message("DiscreteField \"%s\"" % discretefield2.name())
        discretefield = field.discretefields()[discretefield2.name()]

        found_parameter = False
        skip_parameter = True
        configuration = discretefield.configuration()
        for parametervalue in configuration.parametervalues().values():
            parameter = parametervalue.parameter()
            if parameter.name() in include_parameters:
                found_parameter = True
                value = None
                if parametervalue.value_type == parametervalue.type_int:
                    value = parametervalue.getValue_int()
                elif parametervalue.value_type == parametervalue.type_double:
                    value = parametervalue.getValue_double()
                elif parametervalue.value_type == parametervalue.type_string:
                    value = parametervalue.getValue_string()
                if value == include_parameters[parameter.name()]:
                    skip_parameter = False
        if found_parameter and skip_parameter:
            indent()
            message("Skipping...")
            outdent()
            continue

        # Loop over datasets to be written
        indent()
        for discretefieldblock2 in \
                discretefield2.discretefieldblocks().values():
            discretizationblock2 = discretefieldblock2.discretizationblock()

            for discretefieldblockcomponent2 in \
                    discretefieldblock2.discretefieldblockcomponents().values():
                tensorcomponent2 = \
                    discretefieldblockcomponent2.tensorcomponent()
                message("TensorComponent \"%s\"" % tensorcomponent2.name())

                box2 = discretizationblock2.box()
                lower2 = box2.lower()
                upper2 = box2.upper()
                shape2 = box2.shape()
                message("combined_box=%s" % box2)

                dataset2 = discretefieldblockcomponent2.dataset()

                # Loop over datasets to be read
                points_total = box2.size()
                points_read = 0
                indent()
                for discretefieldblock in \
                        discretefield.discretefieldblocks().values():
                    message("DiscreteFieldBlock \"%s\"" %
                            discretefieldblock.name())
                    discretizationblock = \
                        discretefieldblock.discretizationblock()
                    indent()

                    discretefieldblockcomponent = \
                        discretefieldblock.storage_indices()[
                            tensorcomponent2.storage_index()]
                    message("DiscreteFieldBlockComponent \"%s\"" %
                            discretefieldblockcomponent.name())

                    tensorcomponent = \
                        discretefieldblockcomponent.tensorcomponent()
                    assert (tensorcomponent.storage_index() ==
                            tensorcomponent2.storage_index())

                    datarange = discretefieldblockcomponent.datarange()
                    copyobj = discretefieldblockcomponent.copyobj()
                    if datarange:
                        # TODO: implement this
                        pass

                    elif copyobj:
                        # Read dataset
                        box = copyobj.box()
                        lower = box.lower()
                        upper = box.upper()
                        shape = box.shape()
                        # TODO: disregard overlap (ghost zones)
                        points_read += copyobj.box().size()
                        message("box=%s   %d/%d (%d%%)" %
                                (box, points_read, points_total,
                                 100.0 * points_read / points_total))
                        data = copyobj.readData_double()
    
                        # Write into dataset
                        dataset2.writeData_double(data, box)
    
                    else:
                        # unhandled datablock type
                        assert False

                    outdent()
                outdent()

        outdent()

    outdent()
outdent()

file2.close()

message()
message("Done.")
