#!/bin/bash

CC=mpicxx
#rm -rf ./1
#mkdir -p ./1
#mkdir -p ./1/task_obj_files
#mkdir -p ./1/executables
#mkdir -p ./1/output
mkdir -p ./1/fast_op
size=16384
for i in `seq  1 1`;
do
	node=4
	for j in `seq 1 1`;
	do
		$CC -O3 -o ./1/executables/1_size_"$size" cannon_algorithm.cpp -D Matrix_size=$size -D Shared=0
		sed -e "s/SIZE/$size/g" -e "s/NODE/$node/g" -e "s/CORES/1/g" job_1.sh > ./1/executables/job_"$size"_"$node"_1.sh
		sed -e "s/SIZE/$size/g" -e "s/NODE/$node/g" -e "s/CORES/16/g" job_1.sh > ./1/executables/job_"$size"_"$node"_16.sh
		#cp job_1.sh ./1/executables/job_"$size"_"$node"_1.sh
		#cp job_1.sh ./1/executables/job_"$size"_"$node"_16.sh
		sbatch ./1/executables/job_"$size"_"$node"_16.sh
		#slp=$((i*2/(j*j)))
		sleep 17m
		sbatch ./1/executables/job_"$size"_"$node"_1.sh
		#sleep 7m
		node=$((node*4))
	done
	size=$((size*2))
done

