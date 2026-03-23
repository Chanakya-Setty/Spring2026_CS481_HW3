#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load openmpi/4.1.7-gcc12
mpiexec ./hw3 5120 5000 1 /scratch/$USER/output.5120.5000.mpi.1
mpiexec ./hw3 5120 5000 1 /scratch/$USER/output.5120.5000.mpi.1
mpiexec ./hw3 5120 5000 1 /scratch/$USER/output.5120.5000.mpi.1
mpiexec ./hw3 5120 5000 2 /scratch/$USER/output.5120.5000.mpi.2
mpiexec ./hw3 5120 5000 2 /scratch/$USER/output.5120.5000.mpi.2
mpiexec ./hw3 5120 5000 2 /scratch/$USER/output.5120.5000.mpi.2
mpiexec ./hw3 5120 5000 4 /scratch/$USER/output.5120.5000.mpi.4
mpiexec ./hw3 5120 5000 4 /scratch/$USER/output.5120.5000.mpi.4
mpiexec ./hw3 5120 5000 4 /scratch/$USER/output.5120.5000.mpi.4
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
mpiexec ./hw3 5120 5000 8 /scratch/$USER/output.5120.5000.mpi.8
mpiexec ./hw3 5120 5000 16 /scratch/$USER/output.5120.5000.mpi.16
mpiexec ./hw3 5120 5000 16 /scratch/$USER/output.5120.5000.mpi.16
mpiexec ./hw3 5120 5000 16 /scratch/$USER/output.5120.5000.mpi.16
mpiexec ./hw3 5120 5000 32 /scratch/$USER/output.5120.5000.mpi.32
mpiexec ./hw3 5120 5000 32 /scratch/$USER/output.5120.5000.mpi.32
mpiexec ./hw3 5120 5000 32 /scratch/$USER/output.5120.5000.mpi.32
