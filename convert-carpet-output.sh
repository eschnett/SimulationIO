#! /bin/bash

set -e
set -x

# uniform grid
./convert-carpet-output cactus/iof5-uniform.s5 ~/simulations/iof5-uniform/output-0000/iof5-uniform/*.h5

# AMR grid (vertex centred)
./convert-carpet-output cactus/iof5-refined.s5 /Users/eschnett/simulations/iof5-refined/output-0000/iof5-refined/*.h5

# AMR grid (cell-centred)
./convert-carpet-output cactus/iof5-refined-cell.s5 /Users/eschnett/simulations/iof5-refined-cell/output-0000/iof5-refined-cell/*.h5

# multi-block grid
./convert-carpet-output cactus/iof5-multipatch.s5 /Users/eschnett/simulations/iof5-multipatch/output-0000/iof5-multipatch/*.h5

# combined multi-block AMR grid
./convert-carpet-output cactus/iof5-multipatch-kerrschild.s5 /Users/eschnett/simulations/iof5-multipatch-kerrschild/output-0000/iof5-multipatch-kerrschild/*.h5
