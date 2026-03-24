#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load openmpi/4.1.7-gcc12
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
