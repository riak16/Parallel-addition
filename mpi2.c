#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>


int master_init(int num_size, int comm_size, FILE *file_out, double *time_duration);
void make_sum_master(char *total_sum, char *num1, char *num2, int num_size, int comm_size);
void make_sum_worker(int rank, int num_size, int comm_size);
int add_nums(char *results, int with_one, char *num1, char *num2, int start_pos, int tasks);
int allocate_memory(int size, char **p1, char **p2, char **psum);
void free_memory(char *num1, char *num2, char *total_sum);
int fill_random_numbers(int size, char *num1, char *num2);


int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Enter number of digits in numbers to generate.\n");
		return 0;
	}
	int num_size = atoi(argv[1]);

	FILE *file_out = NULL;
	if (argc == 3)
		file_out = fopen(argv[2], "w");

	MPI_Init(&argc, &argv);
	int rank, comm_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	
	if (!rank) {
		double time_duration = 0;
		master_init(num_size, comm_size, file_out, &time_duration);
		printf("%f", time_duration);
	}
	else 
		make_sum_worker(rank, num_size, comm_size);
	
	if (argc == 3)
		fclose(file_out);
	
	MPI_Finalize();
	return 0;
}

int master_init(int num_size, int comm_size, FILE *file_out, double *time_duration) 
{
	char *num1 = NULL;
	char *num2 = NULL;
	char *total_sum = NULL;
	if (!allocate_memory(num_size, &num1, &num2, &total_sum))
		return 0;
	fill_random_numbers(num_size, num1, num2);
	
	if (file_out) {
		fprintf(file_out, "%s\n", num1);
		fprintf(file_out, "%s\n", num2);
	}

	double time_start = MPI_Wtime();

	make_sum_master(total_sum, num1, num2, num_size, comm_size);

	if (time_duration) 
		*time_duration = MPI_Wtime() - time_start;
	
	if (file_out)
		fprintf(file_out, "%s\n", total_sum);
	
	free_memory(num1, num2, total_sum);
	return 1;
}

void make_sum_master(char *total_sum, char *num1, char *num2, int num_size, int comm_size) 
{
	int tasks = num_size / comm_size;
	int tasks_other = tasks; // Number of tasks that have all processes except zero process.
	int add_tasks = num_size - tasks * comm_size; // Tasks that will be computed by zero process.
	tasks += add_tasks;
	int start_pos = 0;
	int rank = 0;

	int i = 0;
	for (i = comm_size - 1; i >= 1; i--) {
		int offset = i * tasks_other + add_tasks;
		MPI_Send(num1 + offset, tasks_other, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		MPI_Send(num2 + offset, tasks_other, MPI_CHAR, i, 0, MPI_COMM_WORLD);
	}

	char *sum_with_one = (char *)calloc(tasks, sizeof(char));
	char *sum_without_one = (char *)calloc(tasks, sizeof(char)); 

	assert(sum_with_one && sum_without_one);

	int sum_with_one_stat = -1;
	if (rank != comm_size - 1)
		sum_with_one_stat = add_nums(sum_with_one, 1, num1, num2, start_pos, tasks);
	int sum_without_one_stat = add_nums(sum_without_one, 0, num1, num2, start_pos, tasks);


	int with_one_stat = 0;
	if (rank != comm_size - 1)
		MPI_Recv(&with_one_stat, 1, MPI_INT, rank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		

	if (with_one_stat) {
		if (sum_with_one_stat)
			total_sum[0] = '1';
		else
			total_sum[0] = '0';
		memcpy(total_sum + 1, sum_with_one, tasks);
	} else {
		if (sum_without_one_stat)
			total_sum[0] = '1';
		else 
			total_sum[0] = '0';
		memcpy(total_sum + 1, sum_without_one, tasks);
	}
	free(sum_with_one);
	free(sum_without_one);

	for (i = comm_size - 1; i >= 1; i--) {
		int offset = i * tasks_other + add_tasks + 1;
		MPI_Recv(total_sum + offset, tasks_other, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if(i==1)
			printf("%s sum\n",total_sum );
	}
}


void make_sum_worker(int rank, int num_size, int comm_size)
{
	int tasks = num_size / comm_size;
	int start_pos = 0; // I have zero offset in my pieces of nums.
	char *num1 = (char *)calloc(tasks, sizeof(char));
	char *num2 = (char *)calloc(tasks, sizeof(char));
	char *sum_with_one = (char *)calloc(tasks, sizeof(char));
	char *sum_without_one = (char *)calloc(tasks, sizeof(char)); 
	
	assert(num1 && num2 && sum_with_one && sum_without_one);

	MPI_Recv(num1, tasks, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(num2, tasks, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	int sum_with_one_stat = -1;
	if (rank != comm_size - 1)
		sum_with_one_stat = add_nums(sum_with_one, 1, num1, num2, start_pos, tasks);
	int sum_without_one_stat = add_nums(sum_without_one, 0, num1, num2, start_pos, tasks);

	if (rank == comm_size - 1) 
		MPI_Send(&sum_without_one_stat, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

	int with_one_stat = 0;
	if (rank != comm_size - 1) {
		MPI_Recv(&with_one_stat, 1, MPI_INT, rank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (with_one_stat)
			MPI_Send(&sum_with_one_stat, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
		else 
			MPI_Send(&sum_without_one_stat, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);	
	}

	if (with_one_stat) 
	 	MPI_Send(sum_with_one, tasks, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	else 
		MPI_Send(sum_without_one, tasks, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

	free(sum_with_one);
	free(sum_without_one);
}



int add_nums(char *results, int with_one, char *num1, char *num2, int start_pos, int tasks) 
{
	if (with_one)
		with_one = 1;
	int i = 0; 
	for (i = tasks - 1; i >= 0; i--) {
		int dig = num1[start_pos + i] + num2[start_pos + i] + with_one - 2*'0';
		results[i] = (dig % 10) + '0';
		with_one = dig / 10;
	}
	return with_one;
}

int allocate_memory(int size, char **p1, char **p2, char **psum)
{
	char *num1 = (char *)calloc(size+1, sizeof(char)); // One additional cell for null - EOF.
	if (!num1)
		return 0;
	char *num2 = (char *)calloc(size+1, sizeof(char));
	if (!num2) {
		free(num1);
		return 0;
	}	
	char *sum = (char *)calloc(size+2, sizeof(char));
	if (!sum) {
		free(num1);
		free(num2);
		return 0;
	}
	*p1 = num1;
	*p2 = num2;
	*psum = sum;
	return 1;
}

void free_memory(char *num1, char *num2, char *total_sum) 
{
	free(num1);
	free(num2);
	free(total_sum);
}

int fill_random_numbers(int size, char *num1, char *num2)
{	
	srand(time(NULL));
	int i = 0;
	for (i = 0; i < size; i++) {
		num1[i] = '0' + (rand() % 10);
		num2[i] = '0' + (rand() % 10);
	}
	printf("%s num1 \n",num1);
	printf("%s num2 \n",num2);
	return 1;
}