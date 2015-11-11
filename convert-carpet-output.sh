#! /bin/bash

set -e
set -x

for sim in uniform refined refined-cell multipatch multipatch-kerrschild; do
  for procs in '' -p2; do
    ./convert-carpet-output cactus/iof5-$sim$procs.s5 ~/simulations/iof5-$sim$procs/output-0000/iof5-$sim/*.h5
  done
done
