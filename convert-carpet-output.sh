#! /bin/bash

set -e
set -x

# uniform grid
./convert-carpet-output cactus/iof5-uniform.h5 ~/simulations/iof5-uniform/output-0000/iof5-uniform/grid-coordinates.h5

# AMR grid
./convert-carpet-output cactus/iof5-refined.h5 /Users/eschnett/simulations/iof5-refined/output-0000/iof5-refined/grid-coordinates.h5

# multi-block grid
./convert-carpet-output cactus/iof5-multipatch.h5 /Users/eschnett/simulations/iof5-multipatch/output-0000/iof5-multipatch/grid-coordinates.h5

# combined multi-block AMR grid
./convert-carpet-output cactus/iof5-multipatch-kerrschild.h5 /Users/eschnett/simulations/iof5-multipatch-kerrschild/output-0000/iof5-multipatch-kerrschild/*.h5
