#include <iostream>
#include <mpi.h>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <iomanip>

using namespace std;
#define ROOT 0


void master(vector<vector<int>>& matrix, int block_size, int blocks_count, int lines_per_slave){
    vector<int> initialLine(matrix.size());
    for (int i = 0; i < initialLine.size(); i++) {
        initialLine[i] = i;
    }

    vector<MPI_Request> send_requests(blocks_count);
    for (int i = 0; i < blocks_count; ++i) {
        MPI_Isend(initialLine.data() + i * block_size, block_size, MPI_INT, 1, i, MPI_COMM_WORLD, &send_requests[i]);
    }

    vector<MPI_Request> recv_requests;
    for (int i = 0; i < matrix.size(); ++i) {
        for (int j = 0; j < blocks_count; ++j) {
            MPI_Request request;
            MPI_Irecv(matrix[i].data() + j * block_size, block_size, MPI_INT, i / lines_per_slave + 1, j, MPI_COMM_WORLD, &request);
            recv_requests.push_back(request);
        }
    }


    for (int i = 0; i < recv_requests.size(); ++i) {
        MPI_Wait(&recv_requests[i], MPI_STATUS_IGNORE);
    }
}

void slave(vector<int> buffer, int lines_count, int blocks_count, int rank, int world_size) {
    vector<MPI_Request> send_requests;
    vector<MPI_Request> send_requests_to_root;
    for (int k = 0; k < blocks_count; ++k) {
        MPI_Status status;
        MPI_Recv(buffer.data(), buffer.size(), MPI_INT, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        for (int i = 0; i < lines_count; ++i) {
            for (int j = 0; j < buffer.size(); ++j) {
                buffer[j] += 1;
            }

            MPI_Request request;
            MPI_Isend(buffer.data(), buffer.size(), MPI_INT, ROOT, status.MPI_TAG, MPI_COMM_WORLD, &request);
            send_requests_to_root.push_back(request);

            if (i + 1 == lines_count && rank + 1 != world_size) {
                MPI_Request request;
                MPI_Isend(buffer.data(), buffer.size(), MPI_INT, rank + 1, status.MPI_TAG, MPI_COMM_WORLD, &request);
                send_requests.push_back(request);
            }
        }
    }

}

void print(vector<vector<int>>& a){
    for (int i = 0; i < a.size(); i++) {
        for (int j = 0; j < a[i].size(); j++) {
            cout << setw(2) << a[i][j] << " ";
        }

        cout << endl;
    }
    cout << endl;
}

int main() {

    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int block_size = 8;
    int slaves_count = world_size - 1;
    int blocks_count = slaves_count;
    int size = block_size * blocks_count;
    int lines_per_slave = size / slaves_count;


    if (rank == ROOT) {

        vector<vector<int>> a(size, vector<int>(size, 0));
        master(a, block_size, blocks_count, lines_per_slave);
        print(a);

    } else {
        vector<int> buffer(block_size);
        slave(buffer, lines_per_slave, blocks_count, rank, world_size);
    }

    MPI_Finalize();

    return 0;
}