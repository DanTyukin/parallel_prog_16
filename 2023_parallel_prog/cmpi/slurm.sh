#!/bin/bash
#SBATCH --partition cascade
#SBATCH --nodes=3
#SBATCH --output=./res/out
#SBATCH --error=./res/err
#SBATCH --time=01:00:00

module purge
module add compiler/gcc/11.2.0
module add mpi/openmpi/4.1.3/gcc/11
module add python/3.9

make rebuild
make test