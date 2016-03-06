unshift!(LOAD_PATH, ".")

using SimulationIO
using H5
using RegionCalculus
using Base.Test



b1 = IBox()
b2 = IBox(3)
b3 = IBox([0,0,0], [1,1,1])
@test string(b1) == "dbox()"
@test string(b2) == "([0,0,0]:[0,0,0])"
@test string(b3) == "([0,0,0]:[1,1,1])"
@test !valid(b1)
@test valid(b2)
@test valid(b3)
@test rank(b2) == 3
@test rank(b3) == 3
@test isempty(b2)
@test !isempty(b3)
@test lower(b3) == [0,0,0]
@test upper(b3) == [1,1,1]
@test shape(b3) == [1,1,1]
@test size(b2) == 0
@test size(b3) == 1
@test isempty(b2 >> [1,2,3])
@test string(b3 >> [1,2,3]) == "([1,2,3]:[2,3,4])"
@test string(b3 << [1,2,3]) == "([-1,-2,-3]:[0,-1,-2])"
@test string(b3 * [1,2,3]) == "([0,0,0]:[1,2,3])"
@test b2 == b2
@test b3 == b3
@test b2 != b3
@test b2 <= b3
@test b3 >= b2
@test b2 < b3
@test b3 > b2
@test !contains(b2, [0,0,0])
@test contains(b3, [0,0,0])
@test !contains(b3, [1,1,1])
@test isdisjoint(b3, b3 >> [1,1,1])
@test bounding_box(b3, b3 >> [1,1,1]) == IBox([0,0,0], [2,2,2])
@test b3 & (b3 * [2,2,2]) == b3

r1 = IRegion()
r2 = IRegion(3)
r3 = IRegion(b3)
r4 = IRegion([b3, b3 >> [1,1,1]])
@test string(r1) == "dregion()"
@test string(r2) == "{}"
@test string(r3) == "{([0,0,0]:[1,1,1])}"
@test string(r4) == "{([0,0,0]:[1,1,1]),([1,1,1]:[2,2,2])}"
@test !valid(r1)
@test valid(r2)
@test valid(r3)
@test valid(r4)
@test rank(r2) == 3
@test rank(r3) == 3
@test rank(r4) == 3
@test isempty(r2)
@test !isempty(r3)
@test !isempty(r4)
@test size(r2) == 0
@test size(r3) == 1
@test size(r4) == 2
@test isempty(bounding_box(r2))
@test bounding_box(r3) == b3
@test bounding_box(r4) == b3 * [2,2,2]
@test r3 & r4 == r3
@test r4 - r3 == IRegion(b3 >> [1,1,1])
@test r3 - r4 == r2
@test r3 | r4 == r4
@test r4 $ r3 == IRegion(b3 >> [1,1,1])
@test contains(r3, [0,0,0])
@test contains(r4, [1,1,1])
@test !contains(r2, [1,1,1])
@test !contains(r3, [1,1,1])
@test isdisjoint(r3, r2)
@test r2 <= r2
@test r3 <= r4
@test r2 >= r2
@test r4 >= r3
@test r2 < r3
@test r3 < r4
@test r3 > r2
@test r4 > r3
@test r2 == r2
@test r2 == r2
@test r2 != r3
@test r2 != r4



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
@test name(tensortype(tensorcomponent1)[]) == name(tensortype1)
@test storage_index(tensorcomponent1) == 0
@test indexvalues(tensorcomponent1) == []
tensorcomponent2 = storage_indices(tensortype2)[1]
@test name(tensortype(tensorcomponent2)[]) == name(tensortype2)
@test storage_index(tensorcomponent2) == 1
@test indexvalues(tensorcomponent2) == [1]
tensorcomponent3 = storage_indices(tensortype3)[2]
@test name(tensortype(tensorcomponent3)[]) == name(tensortype3)
@test storage_index(tensorcomponent3) == 2
@test indexvalues(tensorcomponent3) == [0,2]

parameter1 = createParameter(project1, "parameter1")
@test length(parameters(project1)) == 1
@test name(parameter1) == "parameter1"
@test name(project(parameter1)[]) == name(project1)
@test isempty(parametervalues(parameter1))
@test invariant(parameter1)

parametervalue1 = createParameterValue(parameter1, "parametervalue1")
@test length(parametervalues(parameter1)) == 1
@test name(parameter(parametervalue1)[]) == name(parameter1)
@test isempty(configurations(parametervalue1))
@test invariant(parametervalue1)

configuration1 = createConfiguration(project1, "configuration1")
@test length(configurations(project1)) == 1
@test name(configuration1) == "configuration1"
@test name(project(configuration1)[]) == name(project1)
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
@test name(project(manifold1)[]) == name(project1)
@test name(configuration(manifold1)) == name(configuration1)
@test dimension(manifold1) == 2
@test isempty(discretizations(manifold1))
@test isempty(fields(manifold1))
@test isempty(coordinatesystems(manifold1))
@test invariant(manifold1)

tangentspace1 = createTangentSpace(project1, "tangentspace1", configuration1, 3)
@test length(tangentspaces(project1)) == 1
@test name(tangentspace1) == "tangentspace1"
@test name(project(tangentspace1)[]) == name(project1)
@test name(configuration(tangentspace1)) == name(configuration1)
@test dimension(tangentspace1) == 3
@test isempty(bases(tangentspace1))
@test isempty(fields(tangentspace1))
@test invariant(tangentspace1)

field1 = createField(project1, "field1", configuration1, manifold1,
    tangentspace1, tensortype3)
@test length(fields(project1)) == 1
@test name(field1) == "field1"
@test name(project(field1)[]) == name(project1)
@test name(configuration(field1)) == name(configuration1)
@test name(manifold(field1)) == name(manifold1)
@test name(tangentspace(field1)) == name(tangentspace1)
@test name(tensortype(field1)) == name(tensortype3)
