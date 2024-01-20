#!/bin/bash
# ignore! SBATCH --job-name=lab1-pthreads
# ignore! SBATCH --partition tornado
# ignore! SBATCH --nodes=1
# ignore! SBATCH --output=./res/out
# ignore! SBATCH --error=./res/err
#SBATCH --time=01:00:00

module purge
module add compiler/gcc/11.2.0
module add mpi/openmpi/4.1.3/gcc/11
module add python/3.9

./launcher/runner.sh $@
