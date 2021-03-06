#! /usr/bin/env julia

using PyCall

@pyimport SimulationIO as SIO



# Read metadata via SimulationIO, then access data via path

filename = "julia-example.s5"
fieldname = "rho"
discretizationname = "uniform"
tensorcomponentname = "scalar"

# filename = "/Users/eschnett/tmp/e25b13-re2k5xy-extlink.s5"
# fieldname = "HYDROBASE::entrop"
# discretizationnames =
#     ["iteration.0000940544-timelevel.0-level.00",
#      "iteration.0000940544-timelevel.0-level.01",
#      "iteration.0000940544-timelevel.0-level.02",
#      "iteration.0000940544-timelevel.0-level.03",
#      "iteration.0000940544-timelevel.0-level.04",
#      "iteration.0000940544-timelevel.0-level.05",
#      "iteration.0000940544-timelevel.0-level.06",
#      "iteration.0000940544-timelevel.0-level.07",
#      "iteration.0000940544-timelevel.0-level.08"]
# discretizationname = discretizationnames[1]
# tensorcomponentname = "1"

# Read project
project = SIO.readProjectHDF5(filename)

rsum = 0.0
rsum2 = 0.0
rmin = typemax(Float64)
rmax = typemin(Float64)
rcount = 0.0
ngrids = 0

function findkey(pred, map)
    for (k,v) in map[:iteritems]()
        if pred(v) return k end
    end
    nothing
end
function findvalue(pred, map)
    for v in map[:itervalues]()
        if pred(v) return v end
    end
    nothing
end

field = get(project[:fields](), fieldname)
discretefield =
    findvalue(df -> df[:discretization]()[:name]() == discretizationname,
              field[:discretefields]())
@assert discretefield !== nothing

for discretefieldblock in discretefield[:discretefieldblocks]()[:itervalues]()
    discretefieldblockcomponent =
        findvalue(dfbc -> (dfbc[:tensorcomponent]()[:name]() ==
                           tensorcomponentname),
                  discretefieldblock[:discretefieldblockcomponents]())
    @assert discretefieldblockcomponent !== nothing

    dataset = discretefieldblockcomponent[:copyobj]()
    data = dataset[:readData_double]()

    ngrids += 1
    for val in data
        rsum += val
        rsum2 += val^2
        rmin = min(rmin, val)
        rmax = max(rmax, val)
        rcount += 1.0
    end
end

if rcount == 0.0
    ravg = 0.0
    rnorm2 = 0.0
else
    ravg = rsum / rcount
    rnorm2 = sqrt(rsum2 / rcount)
end
println("""\
$fieldname:
    ngrids=$ngrids
    npoints=$rcount
    min=$rmin
    max=$rmax
    avg=$ravg
    norm2=$rnorm2
""")
