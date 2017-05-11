#!/bin/bash
#SBATCH -J 2_2048_1_CORES    # Job Name
#SBATCH -o ./2/2a/task_obj_files/2048_1.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 1          # Total number of  tasks requested
#SBATCH -N 1
#SBATCH -p development  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
export OMP_NUM_THREADS=16
ibrun tacc_affinity ./2/2a/executables/2048  > ./2/2a/output/2048_1
