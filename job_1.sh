#!/bin/bash
#SBATCH -J 1_SIZE_NODE_CORES    # Job Name
#SBATCH -o ./1/task_obj_files/1_SIZE_NODE_CORES.o%j      # Output and error file name (%j expands to jobID)
#SBATCH --ntasks-per-node CORES           # Total number of  tasks requested
#SBATCH -N NODE
#SBATCH -p development  # Queue (partition) name -- normal, development, etc.
#SBATCH -t 02:00:00     # Run time (hh:mm:ss) - 1.5 hours
ibrun ./1/executables/1_size_SIZE  > ./1/fast_op/1_SIZE_NODE_CORES
