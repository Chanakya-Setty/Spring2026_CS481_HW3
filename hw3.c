/*
 Name: Chanakya Thirumala Setty
 Email: cthirumalasetty@crimson.ua.edu
 Course Section: CS 481
 Homework #: 3
 To Compile: mpicc -Wall -O3 -o hw3 hw3.c
 To Run: mpiexec -n <num_processes> ./hw3 <board_size> <max_generations> <num_processes> <output_file>
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

/* function to allocate a 2-D array with contiguous memory */
int **allocarray(int P, int Q) {
    int i;
    int *p, **a;

    p = (int *)malloc(P * Q * sizeof(int));
    a = (int **)malloc(P * sizeof(int *));

    if (p == NULL || a == NULL)
        printf("Error allocating memory\n");

    /* row major storage */
    for (i = 0; i < P; i++)
        a[i] = &p[i * Q];

    return a;
}

/* function to free a 2-D array */
void freearray(int **a) {
    free(&a[0][0]);
    free(a);
}

/* function to initialize a 2-D array with a given value */
void initarray(int **a, int mrows, int ncols, int value) {
    int i, j;
    for (i = 0; i < mrows; i++)
        for (j = 0; j < ncols; j++)
            a[i][j] = value;
}

int main(int argc, char **argv)
{
    int N, K;
    int rank, size;
    double starttime, endtime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* check command-line arguments */
    if (argc < 5) {
        if (rank == 0)
            printf("Usage: %s <board_size> <max_generations> <num_processes> <output_file>\n", argv[0]);
        MPI_Finalize();
        exit(-1);
    }

    N = atoi(argv[1]);
    K = atoi(argv[2]);
    /* argv[3] is num_processes (informational, actual count from MPI_Comm_size) */
    char *output_file = argv[4];

    /* calculate how many rows each process gets (1D row-wise distribution) */
    int base_rows = N / size;
    int remainder = N % size;

    /* processes with rank < remainder get one extra row */
    int my_rows, my_start;
    if (rank < remainder) {
        my_rows = base_rows + 1;
        my_start = rank * (base_rows + 1) + 1;
    } else {
        my_rows = base_rows;
        my_start = remainder * (base_rows + 1) + (rank - remainder) * base_rows + 1;
    }

    /* allocate local arrays with ghost rows and ghost columns */
    /* local size: (my_rows + 2) rows x (N + 2) columns */
    int **local_current = allocarray(my_rows + 2, N + 2);
    int **local_next = allocarray(my_rows + 2, N + 2);

    /* initialize local arrays to 0 */
    initarray(local_current, my_rows + 2, N + 2, 0);
    initarray(local_next, my_rows + 2, N + 2, 0);

    /* process 0 initializes the full board and distributes to all processes */
    int **full_board = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        full_board = allocarray(N + 2, N + 2);
        initarray(full_board, N + 2, N + 2, 0);

        /* initialize board with random values (default seed for reproducibility) */
        for (int i = 1; i <= N; i++)
            for (int j = 1; j <= N; j++)
                full_board[i][j] = rand() % 2;

        /* prepare scatter parameters */
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));

        int curr_start = 1;
        for (int i = 0; i < size; i++) {
            int rows_i = (i < remainder) ? base_rows + 1 : base_rows;
            sendcounts[i] = rows_i * (N + 2);
            displs[i] = curr_start * (N + 2);
            curr_start += rows_i;
        }
    }

    /* scatter the initial board to all processes */
    int recvcount = my_rows * (N + 2);
    MPI_Scatterv(rank == 0 ? &full_board[0][0] : NULL,
                 sendcounts, displs, MPI_INT,
                 &local_current[1][0], recvcount, MPI_INT,
                 0, MPI_COMM_WORLD);

    /* determine neighbor ranks (MPI_PROC_NULL for boundary processes) */
    int top_neighbor = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
    int bottom_neighbor = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;

    /* synchronize before timing */
    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();

    int max_generations = 0;

    for (int k = 0; k < K; k++) {
        max_generations = k + 1;

        /* exchange ghost rows with neighbors using non-blocking MPI */
        MPI_Request requests[4];
        MPI_Status statuses[4];

        /* send my top data row to top neighbor, receive top ghost row from top neighbor */
        MPI_Isend(&local_current[1][0], N + 2, MPI_INT,
                  top_neighbor, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(&local_current[0][0], N + 2, MPI_INT,
                  top_neighbor, 1, MPI_COMM_WORLD, &requests[1]);

        /* send my bottom data row to bottom neighbor, receive bottom ghost row from bottom neighbor */
        MPI_Isend(&local_current[my_rows][0], N + 2, MPI_INT,
                  bottom_neighbor, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(&local_current[my_rows + 1][0], N + 2, MPI_INT,
                  bottom_neighbor, 0, MPI_COMM_WORLD, &requests[3]);

        /* wait for all ghost row exchanges to complete */
        MPI_Waitall(4, requests, statuses);

        /* compute next generation for local rows */
        bool local_update = false;
        for (int i = 1; i <= my_rows; i++) {
            for (int j = 1; j <= N; j++) {
                int n_count = local_current[i-1][j-1] + local_current[i-1][j] + local_current[i-1][j+1]
                            + local_current[i][j-1]                           + local_current[i][j+1]
                            + local_current[i+1][j-1] + local_current[i+1][j] + local_current[i+1][j+1];
                int alive = local_current[i][j];
                int next_state = (n_count == 3) || (alive && n_count == 2);
                if (next_state != alive)
                    local_update = true;
                local_next[i][j] = next_state;
            }
        }

        /* check globally if any process had a change */
        int local_flag = local_update ? 1 : 0;
        int global_flag = 0;
        MPI_Allreduce(&local_flag, &global_flag, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);

        /* exit if no change across all processes */
        if (!global_flag) {
            if (rank == 0)
                printf("No change after iteration %d, exiting.\n", max_generations);
            break;
        }

        /* swap current and next arrays */
        int **temp = local_current;
        local_current = local_next;
        local_next = temp;
    }

    endtime = MPI_Wtime();

    if (rank == 0)
        printf("Time taken for size %d with %d processes = %lf seconds\n",
               N, size, endtime - starttime);

    /* gather the final board to process 0 using MPI_Gatherv */
    int *recvcounts = NULL;
    int *recvdispls = NULL;

    if (rank == 0) {
        recvcounts = (int *)malloc(size * sizeof(int));
        recvdispls = (int *)malloc(size * sizeof(int));

        int curr_start = 1;
        for (int i = 0; i < size; i++) {
            int rows_i = (i < remainder) ? base_rows + 1 : base_rows;
            recvcounts[i] = rows_i * (N + 2);
            recvdispls[i] = curr_start * (N + 2);
            curr_start += rows_i;
        }
    }

    int sendcount = my_rows * (N + 2);
    MPI_Gatherv(&local_current[1][0], sendcount, MPI_INT,
                rank == 0 ? &full_board[0][0] : NULL,
                recvcounts, recvdispls, MPI_INT,
                0, MPI_COMM_WORLD);

    /* process 0 writes the final board to the output file */
    if (rank == 0) {
        FILE *fp = fopen(output_file, "w");
        if (fp != NULL) {
            for (int i = 1; i <= N; i++) {
                for (int j = 1; j <= N; j++) {
                    fprintf(fp, "%d ", full_board[i][j]);
                }
                fprintf(fp, "\n");
            }
            fclose(fp);
        } else {
            printf("Error opening output file: %s\n", output_file);
        }

        printf("Total Generations: %d\n", max_generations);

        /* free rank 0 specific allocations */
        freearray(full_board);
        free(sendcounts);
        free(displs);
        free(recvcounts);
        free(recvdispls);
    }

    /* free local arrays */
    freearray(local_current);
    freearray(local_next);

    MPI_Finalize();
    return 0;
}
