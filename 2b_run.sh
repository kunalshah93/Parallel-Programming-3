#!/bin/bash

CC=mpicxx
flag=-openmp
#rm -rf ./1
#mkdir -p ./2
mkdir -p ./2/2b
mkdir -p ./2/2b/task_obj_files
mkdir -p ./2/2b/executables
mkdir -p ./2/2b/output

size=1024
for i in `seq  1 5`;
do
	node=16
	for j in `seq 1 1`;
	do
		$CC  $flag -o ./2/2b/executables/"$size" cannon_algorithm.cpp -D Matrix_size=$size -D Shared=2
		sed -e "s/SIZE/$size/g" -e "s/NODE/$node/g" 2b_job.sh > ./2/2b/executables/job_"$size"_"$node".sh
		#cp job_1.sh ./1/executables/job_"$size"_"$node"_1.sh
		#cp job_1.sh ./1/executables/job_"$size"_"$node"_16.sh
		sbatch ./2/2b/executables/job_"$size"_"$node".sh
		#slp=$((i*2/(j*j)))
		#sleep 7m
		node=$((node*4))
	done
	size=$((size*2))
done

