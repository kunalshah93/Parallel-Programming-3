#!/bin/bash
#SBATCH -J 2_8192_16    # Job Name
#SBATCH -o ./2/2a/task_obj_files/8192_16.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 1          # Total number of  tasks requested
#SBATCH -N 16
#SBATCH -p normal  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
export OMP_NUM_THREADS=16
ibrun tacc_affinity ./2/2a/executables/8192  > ./2/2a/output/8192_16
