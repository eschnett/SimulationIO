unshift!(LOAD_PATH, ".")

using SimulationIO
using Base.Test

project1 = createProject("project1")
@test name(project1) == "project1"
@test isempty(parameters(project1))
@test isempty(configurations(project1))
@test isempty(tensortypes(project1))
@test isempty(manifolds(project1))
@test isempty(tangentspaces(project1))
@test isempty(fields(project1))
@test isempty(coordinatesystems(project1))
@test invariant(project1)

createStandardTensorTypes(project1)
@test length(tensortypes(project1)) == 15
tensortype1 = tensortypes(project1)["Vector3D"]
#TODO: check tensortypes and tensorcomponents

parameter1 = createParameter(project1, "parameter1")
@test length(parameters(project1)) == 1
@test name(parameter1) == "parameter1"
@test name(get(project(parameter1))) == name(project1)
@test isempty(parametervalues(parameter1))
@test invariant(parameter1)

#TODO parametervalue1 = createParameterValue(parameter1, "parametervalue1")
#TODO @test length(parametervalues(parameter1)) == 1

#TODO info(project1)
#TODO info(parameter1)
#TODO info(parametervalue1)
#TODO parameter(parametervalue1)
#TODO get(parameter(parametervalue1))
#TODO name(get(parameter(parametervalue1)))
#TODO name(parameter1)

#TODO @test name(get(parameter(parametervalue1))) == name(parameter1)
#TODO @test isempty(configurations(parametervalue1))
#TODO @test invariant(parametervalue1)

configuration1 = createConfiguration(project1, "configuration1")
@test length(configurations(project1)) == 1
@test name(configuration1) == "configuration1"
@test name(get(project(configuration1))) == name(project1)
@test isempty(parametervalues(configuration1))
@test isempty(bases(configuration1))
@test isempty(discretefields(configuration1))
@test isempty(discretizations(configuration1))
@test isempty(fields(configuration1))
@test isempty(manifolds(configuration1))
@test isempty(tangentspaces(configuration1))
@test invariant(configuration1)

manifold1 = createManifold(project1, "manifold1", configuration1, 2)
@test length(manifolds(project1)) == 1
@test name(manifold1) == "manifold1"
@test name(get(project(manifold1))) == name(project1)
@test name(configuration(manifold1)) == name(configuration1)
@test dimension(manifold1) == 2
@test isempty(discretizations(manifold1))
@test isempty(fields(manifold1))
@test isempty(coordinatesystems(manifold1))
@test invariant(manifold1)

tangentspace1 = createTangentSpace(project1, "tangentspace1", configuration1, 3)
@test length(tangentspaces(project1)) == 1
@test name(tangentspace1) == "tangentspace1"
@test name(get(project(tangentspace1))) == name(project1)
@test name(configuration(tangentspace1)) == name(configuration1)
@test dimension(tangentspace1) == 3
@test isempty(bases(tangentspace1))
@test isempty(fields(tangentspace1))
@test invariant(tangentspace1)

field1 = createField(project1, "field1", configuration1, manifold1,
    tangentspace1, tensortype1)
@test length(fields(project1)) == 1
@test name(field1) == "field1"
@test name(get(project(field1))) == name(project1)
@test name(configuration(field1)) == name(configuration1)
@test name(manifold(field1)) == name(manifold1)
@test name(tangentspace(field1)) == name(tangentspace1)
@test name(tensortype(field1)) == name(tensortype1)
