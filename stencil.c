#include "stencil.h"


int main(int argc, char **argv) {

	if (4 != argc) {
		printf("Usage: stencil input_file output_file number_of_applications\n");
		return 1;
	}

	char *input_name = argv[1];
	char *output_name = argv[2];
	int num_steps = atoi(argv[3]); //number of times to
	const int root = 0;
	int rank, size;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double *input = NULL; //So that each process 
	int num_values;

	// Read input file
	if(rank == root){
		if (0 > (num_values = read_input(input_name, &input))) {
			return 2;
		}
	}

	MPI_Bcast(&num_values, 1, MPI_INT, root, MPI_COMM_WORLD);


	// Stencil values
	double h = 2.0*PI/num_values;
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH/2;
	const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};

	// Start timer
	double start = MPI_Wtime();

	double *output = NULL;

	if(rank == root){
		// Allocate data for result
		if (NULL == (output = malloc(num_values * sizeof(double)))) {
			perror("Couldn't allocate memory for output");
			return 2;
		}
	}

	int elements_per_process = num_values/size;
	double *process_memory;
	double *left_buffer;
	double *data;
	double *right_buffer;

	if (NULL == (process_memory = malloc((EXTENT + elements_per_process + EXTENT)* sizeof(double)))) {
		perror("Couldn't allocate memory for output");
		return 2;
	}

	left_buffer = process_memory;
	data = process_memory+EXTENT;
	right_buffer = process_memory+elements_per_process+EXTENT;


	
	MPI_Scatter(input, elements_per_process, MPI_DOUBLE, data, elements_per_process, MPI_DOUBLE, root, MPI_COMM_WORLD);

	

	//Setting up persistant communication
	MPI_Request recv_obj[2];
	MPI_Request send_obj[2];
	int src; //source and destination are the same

	if(rank-1 < 0){
		src = size-1;
	} else {
		src = rank -1;
	}

	MPI_Recv_init(left_buffer,EXTENT,MPI_DOUBLE,src, 222, MPI_COMM_WORLD, &recv_obj[0]);
	MPI_Send_init(left_buffer+EXTENT,EXTENT,MPI_DOUBLE,src, 222, MPI_COMM_WORLD, &send_obj[0]);

	if(rank+1>size-1){
		src = 0;
	} else{
		src = rank+1;
	}

	MPI_Recv_init(right_buffer,EXTENT,MPI_DOUBLE,src, 222, MPI_COMM_WORLD, &recv_obj[1]);
	MPI_Send_init(right_buffer-EXTENT,EXTENT,MPI_DOUBLE,src, 222, MPI_COMM_WORLD, &send_obj[1]);


	// MPI_Startall(2,recv_obj);
	// MPI_Startall(2,send_obj);

	// MPI_Waitall(2, recv_obj, &status);
	// MPI_Waitall(2, send_obj, &status);

	if (rank==0){
		for(int i=0;i<elements_per_process+EXTENT*2;i++) {
			printf("Rank %d index %d = %lf\n",rank,i,process_memory[i]);
		}
	}

	
	double *process_output = malloc(elements_per_process * sizeof(double)); //output for each process


	// Repeatedly apply stencil
	for (int s=0; s<num_steps; s++) {

		MPI_Startall(2,recv_obj);
		MPI_Startall(2,send_obj);

		MPI_Waitall(2, recv_obj, &status);
		MPI_Waitall(2, send_obj, &status);


		// Apply stencil

		for (int i=EXTENT; i<elements_per_process+EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = i + j;
				result += STENCIL[j] * process_memory[index];
			}
			process_output[i] = result;
		}
		
		// Swap input and output
		if (s < num_steps-1) {
			double *tmp = data;
			data = process_output;
			process_output = tmp;
		}
	}

	MPI_Gather(process_output, elements_per_process, MPI_DOUBLE, output, elements_per_process, MPI_DOUBLE, root, MPI_COMM_WORLD);

	free(input);
	MPI_Request_free(&recv_obj[0]);
	MPI_Request_free(&recv_obj[1]);
	MPI_Request_free(&send_obj[0]);
	MPI_Request_free(&send_obj[1]);
	// Stop timer
	double my_execution_time = MPI_Wtime() - start;

	// Write result
	//printf("%f\n", my_execution_time);
	if(rank == root){
		#ifdef PRODUCE_OUTPUT_FILE
		if (0 != write_output(output_name, output, num_values)) {
			return 2;
		}
		#endif
	}


	// Clean up
	free(output);
	free(process_memory);

	MPI_Finalize(); 

	return 0;
}


int read_input(const char *file_name, double **values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "r"))) {
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values)) {
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double)))) {
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i=0; i<num_values; i++) {
		if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file)){
		perror("Warning: couldn't close input file");
	}
	return num_values;
}


int write_output(char *file_name, const double *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4f ", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n")) {
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}
