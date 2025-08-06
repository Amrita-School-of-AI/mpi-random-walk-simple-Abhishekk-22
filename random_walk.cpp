#include <mpi.h>
#include <iostream>
#include <cstdlib>   // For atoi, rand, srand
#include <ctime>     // For time()
#include <unistd.h>  // For getpid()

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Total number of processes

    if (argc < 3) {
        if (rank == 0) {
            std::cerr << "Usage: mpirun -np <num_processes> ./random_walk <domain_size> <max_steps>\n";
        }
        MPI_Finalize();
        return 1;
    }

    int domain_size = atoi(argv[1]);  // D → domain range [-D, D]
    int max_steps = atoi(argv[2]);

    if (rank == 0) {
        // Controller Process
        int completed_walkers = 0;
        int dummy_buffer;
        MPI_Status status;

        while (completed_walkers < size - 1) {
            MPI_Recv(&dummy_buffer, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            completed_walkers++;
        }

        std::cout << "Controller: All " << size - 1 << " walkers have finished.\n";
    } else {
        // Walker Processes
        srand(time(NULL) * rank + getpid());  // Unique seed per rank

        int position = 0;
        int steps = 0;

        while (steps < max_steps && position >= -domain_size && position <= domain_size) {
            int direction = rand() % 2 == 0 ? -1 : 1;  // Left (-1) or Right (+1)
            position += direction;
            steps++;
        }

        std::cout << "Rank " << rank << ": Walker finished in " << steps << " steps.\n";

        // Notify controller
        int dummy_value = 1;
        MPI_Send(&dummy_value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

