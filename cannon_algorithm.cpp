#include "cannon_header.h"

#define Matrix_size 8
#define V 1
#define H 0
#define Range 100

using namespace std;

void copy_block_data(struct matrix *source_mat, int src_r, int src_c, struct matrix *dest_mat, int dest_r, int dest_c, int blk_size, int mat_size)
{
	int *src = source_mat + mat_size * src_r + src_c;
	int *dest = dest_mat + mat_size * dest_r + dest_c;

	for (int i = 0; i < blk_size; i++) {
		memcpy(dest, src, sizeof(int)*blk_size);
		src += mat_size;
		dest += mat_size;
	}
}

void create_blocks(int *mat, int *block_mat, int blk_size, int mat_size)
{
	int *temp = block_mat;
    int blk_count = mat_size / blk_size;
    int blk_size_sq = blk_size*blk_size;

    for (int i = 0; i < blk_count; i++) {
        for (j = 0; j < blk_count; j++) {
            copy_block_data( mat, i * blk_size, j * blk_size, temp, 0, 0);
            temp += blk_size_sq;
        }
    }
}

int check_proc_count_validity(data_info *info, int proc_no){

    info->proc = proc_no;
	int sqrp = (int)sqrt(proc_no);

	if (sqp*sqp != proc_no) {
		cout<<"Processor's count is not complete square"
		return -1;
	}

	info->sqr_proc = sqrp;
	info->b_size = info->n / sqrp;

	if (info->n % sqrp) {
        cout<<"ERROR: problem size "<< info->n << " is not multiple of square root of processor count"<< sqrp <<endl;
        return -1;
	}

    info->b_cells = info->b_size*info->b_size;

    return 1;
}

void initialize_matrix(int *mat, int n, int fill_mat) {
    int **matrix = new int*[n];
    for (int i =0; i<n; i++)
        matrix[i] = new int[n];
    if(fill_mat == 1) {
        for(int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                int sign = 1 + (int) (Range * (rand() / (RAND_MAX + 1.0)));
                int number = 1 + (int) (Range * (rand() / (RAND_MAX + 1.0)));
                if(sign <= 50)
                    number *= -1;
                matrix[i][j] = number;
            }
        }
    }/*else if(fill_mat == 2) {
        for(int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                matrix[i][j] = 0;
            }
        }
    }*/
    mat = &matrix[0][0];
}

int intialize_mpi(data_info *info){

    int dimensions[2] = {info->sqr_proc, info->sqr_proc};
	int periods[2] = {1, 1};

	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &info->group);
    MPI_Comm_rank(info->group, &(info->rank));
    MPI_Cart_coords(info->group, info->rank, 2, info->coords);

    int dims[2];

    dims[V] = 1;
    dims[H] = 0;
    MPI_Cart_sub(info->mesh, dims, &info->subg_v);

    dims[V] = 0;
    dims[H] = 1;
    MPI_Cart_sub(info->mesh, dims, &info->subg_h);

    if (info->rank == 0) {
        initialize_matrix(info->A, info->n, 1);
        initialize_matrix(info->B, info->n, 1);

		initialize_matrix(info->A_block, info->n, 0);
		initialize_matrix(info->B_block, info->n, 0);

		create_blocks(info->A, info->A_block, info->b_size, info->n);
		create_blocks(info->B, info->B_block, info->b_size, info->n);
	}

	intialize_matrix(info->b_A, info->b_size, 0);
	intialize_matrix(info->b_B, info->b_size, 0);
	initialize_matrix(info->b_C, info->n, 0);
	intialize_matrix(info->temp_A, info->b_size, 0);
	intialize_matrix(info->temp_B, info->b_size, 0);

	MPI_Scatter(info->A_block, info->b_cells, MPI_INT, info->b_A, info->b_cells, MPI_INT, 0, info->group);
	MPI_Scatter(info->B_block, info->b_cells, MPI_INT, info->b_B, info->b_cells, MPI_INT, 0, info->group);
}

void create_matrix_from_blocks(int *mat, int *block_mat, int blk_size, int mat_size)
{
	int *temp = block_mat;
    int blk_count = mat_size / blk_size;
    int blk_size_sq = blk_size*blk_size;

    for (int i = 0; i < blk_count; i++) {
        for (int j = 0; j < blk_count; j++) {
            copy_block_data(temp, 0, 0, mat, i * blk_size, j * blk_size);
            tmp += blk_size_sq;
        }
    }
}

void ring_shift(int *data, int *temp, size_t count, MPI_Comm ring, int delta)
{
	int src, dst;
	MPI_Status status;

	MPI_Cart_shift(ring, 0, delta, &src, &dst);
	MPI_Sendrecv(data, count, MPI_INT, dst, 0,
				temp, count, MPI_INT, src, 0,
				ring, &status);
	memcpy(data, temp, sizeof(int)*count);
}

void matrix_mult(int *A, int *B, int *C, int n){
    for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			int c = 0;
			for (k = 0; k < n; k++)
                c += A[n*i + k]*B[n*k + j];
			C[n*i + j] += c;
		}
    }
}

void run_cannon(data_info *info) {

    ring_shift(info->b_A, info->temp_A, info->b_cells, info->subg_v, -info->coords[H]);
    ring_shift(info->b_B, info->temp_B, info->b_cells, info->subg_h, -info->coords[V]);

    memset(info->b_C, 0, sizeof(int)*info->b_cells);

    for (int i = 0; i < info->sqr_proc){
        matrix_mult(info->b_A, info->b_B, info->b_C, info->b_size);
        ring_shift(info->b_A, info->temp_A, info->b_cells, info->subg_v, -1);
        ring_shift(info->b_B, info->temp_B, info->b_cells, info->subg_h, -1);
    }
}

void print_matrix(int *mat, int n){
    int n_sq = n*n;
    int *temp = mat;
    for (int i = 0; i< n_sq; i++){
        cout<<*temp<<"\t";
        if(i % n == 0)
            cout<<endl;
        temp++;
    }

}

int main(int argc, char **argv) {
    int proc_no;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&proc_no);

    cout<<"proc_no: "<<proc_no<<endl;

    double start = MPI_Wtime();
    data_info info;

    info.n = Matrix_size;

    int res = check_proc_count_validity(&info, proc_no);
    if (res != 1) {
        MPI_Finalize();
        exit(res);
    }

    srand (time(NULL));
    res = initialize_mpi(&info);
    if (res != 1) {
        MPI_Finalize();
        exit(res);
    }

    double load_time = MPI_Wtime();

    run_cannon(&info);

    double cannon_time = MPI_Wtime();

    MPI_Gather(info.b_C, info.b_cells, MPI_INT, info.C_block, info.b_cells, MPI_INT, 0, info.group);
    if (info.rank == 0) {
        initialize_matrix(info.C, info.n, 0);
        create_matrix_from_blocks(info.C, info.C_block, info.b_size, 0);
    }

    double gather_time = MPI_Wtime();

    /* print results */
	if (info->rank == 0) {
        printf("A:\n");
        print_matrix(info.A, info.n);
        printf("B:\n");
        print_matrix(info.B, info.n);
        printf("C:\n");
        print_matrix(info.C, info.n);

        double elapsed_time = MPI_Wtime() - start_time;
		printf("data loading time: %f\n", load_time - start_time);
		printf("matrix multiplication time: %f\n", cannon_time - load_time);
        printf("gather time: %f\n", gather_time - cannon_time);
        printf("total time: %f\n", elapsed_time);
        printf("\n");
    }
    MPI_Finalize();
    return 0;
}
