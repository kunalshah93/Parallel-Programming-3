#!/bin/bash
#SBATCH -J 1_1024_4_1    # Job Name
#SBATCH -o ./1/task_obj_files/1_1024_4_1.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node 1           # Total number of  tasks requested
#SBATCH -N 4
#SBATCH -p development  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
ibrun ./1/executables/1_size_1024  > ./1/output/1_1024_4_1
