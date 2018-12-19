#include <iostream>
#include <mpi.h>
#include <map>
#include <string>
#include <vector>
#include <set>

using namespace std;

void populate_matrix(vector<vector<int>>& matrix, int dimension)
{
    int c = 1;
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            matrix[i][j] = c;
            c++;
        }
    }
}

int main()
{
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int dimension = 4;
    int block_size = 2;

    vector<vector<int>> A(dimension, vector<int>(dimension));
    populate_matrix(A, dimension);

    int rows = world_size - 1;
    int columns = dimension / block_size;
    int lines_in_row = dimension / rows;
    int lines_in_column = dimension / columns;

    vector<MPI_Request>  final_responses(columns * rows);

    int counter = 0;

    int t;
    if (rank == 0)
    {
        cin >> t;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    vector<int> from_top(block_size);
    vector<int> current_line(block_size);
    vector<vector<int>> B(rows*columns, vector<int>(block_size* rows));
    bool recieved = false;
    bool need_rec = false;
    for (int j = 0; j < columns; j++)
    {
        for (int i = 0; i < rows; i++) {
            if (rank == 0)
            {
                MPI_Request zero_req;
                int val;
                MPI_Irecv(&B[counter][0], block_size* rows, MPI_INT, MPI_ANY_SOURCE, 10 + (i * 100) * dimension + j * block_size, MPI_COMM_WORLD, &zero_req);
                final_responses[counter] = zero_req;
                counter++;
            }


            else if (i + 1 == rank) {
                MPI_Request recieve_req;
                vector<int>::const_iterator first = A[i].begin() + j * block_size;
                vector<int>::const_iterator last = A[i].begin() + (j + 1) * block_size;
                vector<int> current_beginning(first, last);



                if (rank > 1 && !recieved)
                {
                    MPI_Recv(&from_top[0], block_size, MPI_INT, rank - 1, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    recieved = true;
                }
                if (need_rec)
                {
                    MPI_Wait(&recieve_req, MPI_STATUS_IGNORE);
                    need_rec = false;
                }

                vector<int> zero_to_send(block_size * rows);
                int result_counter = 0;

                for (int k = 0; k < dimension / rows; k++)
                {
                    vector<int>::const_iterator first = A[i + k].begin() + j * block_size;
                    vector<int>::const_iterator last = A[i + k].begin() + (j + 1) * block_size;
                    vector<int> l(first, last);
                    current_line = l;


                    bool skip_update = false;
                    if (k == 0)
                    {
                        if (rank == 1)
                        {

                            from_top = current_line;
                            skip_update = true;
                        }
                    }
                    else
                    {
                        vector<int>::const_iterator first = A[i*rows + k - 1].begin() + j * block_size;
                        vector<int>::const_iterator last = A[i*rows + k - 1].begin() + (j + 1) * block_size;
                        vector<int> current_beginning(first, last);
                        from_top = current_beginning;
                    }
                    for (int k = 0; k < block_size; k++)
                    {
                        zero_to_send[result_counter] = from_top[k];
                        if (!skip_update)
                        {
                            zero_to_send[result_counter]++;
                        }
                        result_counter++;
                    }
                }

                MPI_Request send_req;
                MPI_Isend(zero_to_send.data(), block_size * rows, MPI_INT, 0, 10 + (i * 100) * dimension + j * block_size, MPI_COMM_WORLD, &send_req);


                if (rank != world_size - 1)
                {
                    MPI_Request req;
                    //fprintf(stderr, "Sending from rank %f \n", rank);
                    MPI_Isend(current_line.data(), block_size, MPI_INT, rank + 1, j, MPI_COMM_WORLD, &req);
                    MPI_Wait(&req, MPI_STATUS_IGNORE);
                }

                if (rank > 1 && (j < columns - 1 || i < rows - 1))
                {
                    MPI_Irecv(&from_top[0], block_size, MPI_INT, rank - 1, j + 1, MPI_COMM_WORLD, &recieve_req);
                    need_rec = true;
                }
                MPI_Wait(&send_req, MPI_STATUS_IGNORE);
            }
        }
    }

    if (rank == 0)
    {
        for (int i = 0; i < final_responses.size(); i++)
        {
            MPI_Wait(&final_responses[i], MPI_STATUS_IGNORE);
            // Send break?
        }
        int current_column_seek = 0;
        for (int a = 0; a < rows*columns; a++)
        {
            for (int b = 0; b < block_size* rows; b++)
            {
                int ost = b % block_size;
                int to_a = b / block_size;
                int current_column_seek = (a * block_size) % columns;
                A[to_a + (a % lines_in_row)*lines_in_row][ost + ((int)(a / lines_in_row))*lines_in_column] = B[a][b];
            }
        }
        for (int i = 0; i < dimension; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                cout << A[i][j] << " ";
            }

            cout << endl;
        }

        //fprintf(stderr, "%f \n", r);
    }

    MPI_Finalize();
}