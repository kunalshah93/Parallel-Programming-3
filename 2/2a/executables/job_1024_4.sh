#!/bin/bash
#SBATCH -J 2_1024_4_CORES    # Job Name
#SBATCH -o ./2/2a/task_obj_files/1024_4.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 1          # Total number of  tasks requested
#SBATCH -N 4
#SBATCH -p development  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
export OMP_NUM_THREADS=16
ibrun tacc_affinity ./2/2a/executables/1024  > ./2/2a/output/1024_4
