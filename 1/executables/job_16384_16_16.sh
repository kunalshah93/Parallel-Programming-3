#!/bin/bash
#SBATCH -J 1_16384_16_16    # Job Name
#SBATCH -o ./1/task_obj_files/1_16384_16_16.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 16           # Total number of  tasks requested
#SBATCH -N 16
#SBATCH -p normal  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
ibrun ./1/executables/1_size_16384  > ./1/output/1_16384_16_16
