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
tensortype1 = tensortypes(project1)["Scalar1D"]
@test dimension(tensortype1) == 1
@test rank(tensortype1) == 0
tensortype2 = tensortypes(project1)["Vector2D"]
@test dimension(tensortype2) == 2
@test rank(tensortype2) == 1
tensortype3 = tensortypes(project1)["SymmetricTensor3D"]
@test dimension(tensortype3) == 3
@test rank(tensortype3) == 2

tensorcomponent1 = storage_indices(tensortype1)[0]
@test name(get(tensortype(tensorcomponent1))) == name(tensortype1)
@test storage_index(tensorcomponent1) == 0
@test indexvalues(tensorcomponent1) == []
tensorcomponent2 = storage_indices(tensortype2)[1]
@test name(get(tensortype(tensorcomponent2))) == name(tensortype2)
@test storage_index(tensorcomponent2) == 1
@test indexvalues(tensorcomponent2) == [1]
tensorcomponent3 = storage_indices(tensortype3)[2]
@test name(get(tensortype(tensorcomponent3))) == name(tensortype3)
@test storage_index(tensorcomponent3) == 2
@test indexvalues(tensorcomponent3) == [0,2]

parameter1 = createParameter(project1, "parameter1")
@test length(parameters(project1)) == 1
@test name(parameter1) == "parameter1"
@test name(get(project(parameter1))) == name(project1)
@test isempty(parametervalues(parameter1))
@test invariant(parameter1)

parametervalue1 = createParameterValue(parameter1, "parametervalue1")
@test length(parametervalues(parameter1)) == 1
@test name(get(parameter(parametervalue1))) == name(parameter1)
@test isempty(configurations(parametervalue1))
@test invariant(parametervalue1)

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
    tangentspace1, tensortype3)
@test length(fields(project1)) == 1
@test name(field1) == "field1"
@test name(get(project(field1))) == name(project1)
@test name(configuration(field1)) == name(configuration1)
@test name(manifold(field1)) == name(manifold1)
@test name(tangentspace(field1)) == name(tangentspace1)
@test name(tensortype(field1)) == name(tensortype3)
