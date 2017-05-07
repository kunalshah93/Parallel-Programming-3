#include "mpi.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

//#define Matrix_size 8
#define V 1
#define H 0
#define Range 100
#define check_buff(buf, name) \
		if (buf == NULL) \
			cout<<name<<" is null "<<endl;
#define print_val(val,name) \
		cout<<name<<" value is: "<<val<<endl;	
#define Debug 0
//#define Shared 0

struct data_info {

        MPI_Comm group, subg_v, subg_h;
        int rank;
        int n;
        int b_size;
        int b_cells;
        int proc;
        int sqr_proc;
        int *A, *B, *C;// Main A, B, C blocks
        int *A_block, *B_block, *C_block; // A and B divided into blocks to be used for scattering, C used for gathering
        int *b_A, *b_B, *b_C; // Block A, B, C with the processors
        int indexes[2];
        int *temp_A, *temp_B;
	double scattering_time, initialization_time;
};
