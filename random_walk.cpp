#include <iostream>
#include <cstdlib> // For atoi, rand, srand, abs
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0]
                      << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = std::atoi(argv[1]);
    max_steps   = std::atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
    // Seed the RNG so each walker is different
    std::srand(static_cast<unsigned>(std::time(nullptr)) * world_rank);

    int position = 0;
    int step;
    for (step = 1; step <= max_steps; ++step)
    {
        // Randomly move -1 or +1
        int delta = (std::rand() % 2) ? +1 : -1;
        position += delta;

        // Check if we've left the domain
        if (std::abs(position) > domain_size)
        {
            break;
        }
    }

    // Print the “finished” message for the autograder
    std::cout << "Rank " << world_rank
              << ": Walker finished in " << step << " steps."
              << std::endl;

    // Signal the controller that we're done (send the step count)
    MPI_Send(
        &step,         // data buffer
        1,             // count
        MPI_INT,       // data type
        0,             // destination rank (controller)
        0,             // tag
        MPI_COMM_WORLD // communicator
    );
}

void controller_process()
{
    int num_walkers = world_size - 1;
    int finished_count = 0;
    int recv_step;
    MPI_Status status;

    // Wait until each walker has sent its “I’m done” message
    while (finished_count < num_walkers)
    {
        MPI_Recv(
            &recv_step,      // buffer for incoming data
            1,               // max elements to receive
            MPI_INT,         // data type
            MPI_ANY_SOURCE,  // accept from any walker
            0,               // tag
            MPI_COMM_WORLD,  // communicator
            &status          // status object
        );
        ++finished_count;
    }

    // All walkers have finished
    std::cout << "Controller: All " << num_walkers
              << " walkers have finished." << std::endl;
}

