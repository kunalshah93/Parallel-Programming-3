#!/bin/bash
#SBATCH -J 2b_1024_16   # Job Name
#SBATCH -o ./2/2b/task_obj_files/1024_16.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 1          # Total number of  tasks requested
#SBATCH -N 16
#SBATCH -p normal  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
export OMP_NUM_THREADS=16
ibrun tacc_affinity ./2/2b/executables/1024  > ./2/2b/output/1024_16
