#include "cannon_header.h"

using namespace std;

void print_matrix(int **mat, int n){
    int n_sq = n*n;
    int *temp = *mat;
    for (int i = 0; i< n_sq; i++){
        cout<<*temp<<"\t";
        if((i+1) % n == 0 && i > 0)
            cout<<endl;
        temp++;
    }
}

int check_proc_count_validity(data_info *info, int proc_no){

    info->proc = proc_no;
    int sqrp = (int)sqrt(proc_no);

    if (sqrp*sqrp != proc_no) {
	cout<<"Processor's count is not complete square\n";
	return -1;
    }

    info->sqr_proc = sqrp;
    info->b_size = info->n / sqrp;

    if (info->n % sqrp) {
        cout<<"problem size "<< info->n << " is not multiple of square root of processor count"<< sqrp <<endl;
        return -1;
    }

    info->b_cells = info->b_size*info->b_size;

    return 1;
}


void copy_data_to_block(int **source_mat, int src_r, int src_c, int **dest_mat, int dest_r, int dest_c, int blk_size, int mat_size)
{
	int *src = *source_mat + mat_size * src_r + src_c;
	int *dest = *dest_mat + mat_size * dest_r + dest_c;

	for (int i = 0; i < blk_size; i++) {
		memcpy(dest, src, sizeof(int)*blk_size);
		src += mat_size;
		dest += blk_size;
	}

}
void copy_data_to_matrix(int **source_mat, int src_r, int src_c, int **dest_mat, int dest_r, int dest_c, int blk_size, int mat_size)
{
	int *src = *source_mat + mat_size * src_r + src_c;
	int *dest = *dest_mat + mat_size * dest_r + dest_c;

	for (int i = 0; i < blk_size; i++) {
		memcpy(dest, src, sizeof(int)*blk_size);
		src += blk_size;
		dest += mat_size;
	}

}

void create_blocks(int **mat, int **block_mat, int blk_size, int mat_size)
{
    int *temp = *block_mat;
    int blk_count = mat_size / blk_size;
    int blk_size_sq = blk_size*blk_size;

    for (int i = 0; i < blk_count; i++) {
        for (int j = 0; j < blk_count; j++) {
            copy_data_to_block( &(*mat), i * blk_size, j * blk_size, &temp, 0, 0, blk_size, mat_size);
            temp += blk_size_sq;
        }
    }
}

void initialize_matrix(int **mat, int n, int fill_mat,  int is_block) {

    if (!is_block) {
	*mat = new int[n*n];
	if (fill_mat == 1) {
		for (int i=0; i<n*n; i++){
#if Debug
			int number = i%n;
#else
			int number = rand()%(2*Range + 1) - Range;
#endif
			(*mat)[i] = number;
		}
	}
    }else {
    	*mat = new int[n*n];
    }
}

void initialize_mpi(data_info *info){

	int dimensions[2] = {info->sqr_proc, info->sqr_proc};
	int periods[2] = {1, 1};

	MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 0, &info->group);
	MPI_Comm_rank(info->group, &(info->rank));
	MPI_Cart_coords(info->group, info->rank, 2, info->indexes);

    	int dims[2];

    	dims[V] = 1;
    	dims[H] = 0;
   	MPI_Cart_sub(info->group, dims, &info->subg_v);

    	dims[V] = 0;
    	dims[H] = 1;
    	MPI_Cart_sub(info->group, dims, &info->subg_h);

    	if (info->rank == 0) {
		double initialization_start = MPI_Wtime();
        	initialize_matrix(&(info->A), info->n, 1, 0);
	        check_buff(info->A,"A")
		initialize_matrix(&(info->B), info->n, 1, 0);
		check_buff(info->B,"B")

		initialize_matrix(&(info->A_block), info->n, 0, 1);
		check_buff(info->A_block,"A_block")
		initialize_matrix(&(info->B_block), info->n, 0, 1);
		check_buff(info->B_block,"B_block")
		initialize_matrix(&(info->C_block), info->n, 0, 1);
		check_buff(info->C_block,"C_block")
		double initialization_end = MPI_Wtime();
		info->initialization_time = initialization_end - initialization_start;

		create_blocks(&(info->A), &(info->A_block), info->b_size, info->n);
		create_blocks(&(info->B), &(info->B_block), info->b_size, info->n);
		double scattering_start = MPI_Wtime();
		info->scattering_time = scattering_start - initialization_end;
	} else {
		info->initialization_time = 0;
		info->scattering_time = 0;
	}
	//print_val(info->rank, "rank")
	//print_val(info->b_size, "b_size")
	double start = MPI_Wtime();
	initialize_matrix(&(info->b_A), info->b_size, 0, 1);
	check_buff(info->b_A,"b_A")
	initialize_matrix(&(info->b_B), info->b_size, 0, 1);
	check_buff(info->b_B,"b_B")
	initialize_matrix(&(info->b_C), info->n, 0, 1);
	check_buff(info->b_C,"b_C")
	initialize_matrix(&(info->temp_A), info->b_size, 0, 1);
	check_buff(info->temp_A,"temp_A")
	initialize_matrix(&(info->temp_B), info->b_size, 0, 1);
	check_buff(info->temp_B,"temp_B")
	double end = MPI_Wtime();
	info->initialization_time += (end - start);
	MPI_Scatter(info->A_block, info->b_cells, MPI_INT, info->b_A, info->b_cells, MPI_INT, 0, info->group);
	MPI_Scatter(info->B_block, info->b_cells, MPI_INT, info->b_B, info->b_cells, MPI_INT, 0, info->group);
	start = MPI_Wtime();
	info->scattering_time += (start - end);
}

void create_matrix_from_blocks(int **mat, int **block_mat, int blk_size, int mat_size)
{
    int *temp = *block_mat;
    int blk_count = mat_size / blk_size;
    int blk_size_sq = blk_size*blk_size;

    for (int i = 0; i < blk_count; i++) {
        for (int j = 0; j < blk_count; j++) {
            copy_data_to_matrix(&temp, 0, 0, &(*mat), i * blk_size, j * blk_size, blk_size, mat_size);
            temp += blk_size_sq;
        }
    }
}

void block_shift(int **data, int **temp, size_t count, MPI_Comm group, int shift)
{
	int src, dest;
	MPI_Status status;

	MPI_Cart_shift(group, 0, shift, &src, &dest);
	MPI_Sendrecv(*data, count, MPI_INT, dest, 0,
				*temp, count, MPI_INT, src, 0,
				group, &status);
	memcpy(*data, *temp, sizeof(int)*count);
}

void matrix_mult(int **A, int **B, int **C, int n){
    for (int i = 0; i < n; i++) {
	for (int j = 0; j < n; j++) {
		int temp = 0;
		for (int k = 0; k < n; k++)
        		 temp += (*A)[n*i + k]*(*B)[n*k + j];
		(*C)[n*i + j] += temp;
	}
    }
}

void matrix_mult_parallel_1(int **A, int **B, int **C, int n) {
    #pragma omp parallel for
    for (int i = 0;  i<n;  i++) {
        for (int k = 0;  k<n;  k++) {
              for (int j = 0;  j<n;  j++) {
                  (*C)[n*i + j] += ((*A)[n*i + k]*(*B)[n*k + j]);
              }
        }
    }
}

void matrix_mult_parallel_2(int **A, int **B, int **C, int n) {
    for (int k = 0;  k<n;  k++) {
        #pragma omp parallel for
        for (int i = 0;  i<n;  i++) {
              for (int j = 0;  j<n;  j++) {
                  (*C)[n*i + j] += ((*A)[n*i + k]*(*B)[n*k + j]);
              }
        }
    }
}

void run_cannon(data_info *info) {

    block_shift(&(info->b_A), &(info->temp_A), info->b_cells, info->subg_v, -info->indexes[H]);
    block_shift(&(info->b_B), &(info->temp_B), info->b_cells, info->subg_h, -info->indexes[V]);

    memset(info->b_C, 0, sizeof(int)*info->b_cells);

    for (int i = 0; i < info->sqr_proc; i++){

        if (Shared == 0)
            matrix_mult(&(info->b_A), &(info->b_B), &(info->b_C), info->b_size);
    	else if (Shared == 1)
            matrix_mult_parallel_1(&(info->b_A), &(info->b_B), &(info->b_C), info->b_size);
        else if (Shared == 2)
            matrix_mult_parallel_2(&(info->b_A), &(info->b_B), &(info->b_C), info->b_size);

    	if (info->rank == 3 && Debug) {
        	printf("A:\n");
	        print_matrix(&(info->b_A), info->b_size);
	        cout<<endl;
	        printf("B:\n");
	        print_matrix(&(info->b_B), info->b_size);
	        cout<<endl;
	        printf("C:\n");
	        print_matrix(&(info->b_C), info->b_size);
	        cout<<endl;
   	}
        block_shift(&(info->b_A), &(info->temp_A), info->b_cells, info->subg_v, -1);
        block_shift(&(info->b_B), &(info->temp_B), info->b_cells, info->subg_h, -1);
    }
}

int main(int argc, char **argv) {
    int proc_no;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&proc_no);

    double start = MPI_Wtime();
    data_info info;

    info.n = Matrix_size;

    int res = check_proc_count_validity(&info, proc_no);
    if (res != 1) {
        MPI_Finalize();
        exit(res);
    }

    srand (time(NULL));
    initialize_mpi(&info);

    double scattering_time = MPI_Wtime();

    run_cannon(&info);

    double multiplication_time = MPI_Wtime();

    MPI_Gather(info.b_C, info.b_cells, MPI_INT, info.C_block, info.b_cells, MPI_INT, 0, info.group);
    if (info.rank == 0) {
        initialize_matrix(&(info.C), info.n, 0, 0);
        create_matrix_from_blocks(&(info.C), &(info.C_block), info.b_size, info.n);
    }

    double gathering_time = MPI_Wtime();

    /* print results */
    if (info.rank == 0) {
	if (Debug) {
	        printf("A:\n");
	        print_matrix(&(info.A), info.n);
		cout<<endl;
	        printf("B:\n");
	        print_matrix(&(info.B), info.n);
		cout<<endl;
	        printf("C:\n");
	        print_matrix(&(info.C), info.n);
		cout<<endl;
	}

        double elapsed_time = MPI_Wtime() - start;
	cout<<"initialization time: "<<info.initialization_time<<endl;
	cout<<"Scattering time: "<<info.scattering_time<<endl;
	cout<<"Multiplication time: "<<(multiplication_time - scattering_time)<<endl;
        cout<<"Gathering time:"<<(gathering_time - multiplication_time)<<endl;
	cout<<"Total time for 1D: "<<(info.scattering_time - scattering_time + gathering_time)<<endl;
        cout<<"Total running time: "<<elapsed_time<<endl;
        printf("\n");

	delete[] info.A, info.B, info.C, info.A_block, info.B_block, info.C_block;
    }
    delete[] info.b_A, info.b_B, info.b_C, info.temp_A, info.temp_B;

    MPI_Finalize();
    return 0;
}
