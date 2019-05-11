// Copyright 2019 Mezina Margarita

#include <tbb/tbb.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#define PATH_INFINITY ~(unsigned int)0

typedef unsigned char u_char;
typedef unsigned int u_int;

struct res_format {
    u_int* distance;
    std::vector<u_int>* path;
};

/* GENERATE GRAPH MATRIX */
u_char** GenerateMatrix(u_int vertex_count, u_int edges_count, bool writing_work) {
    std::cout << "<-------GENERATE MATRIX------->\n";

    // Memory allocation matrix
    u_char** graph_matrix = new u_char*[vertex_count];
    for (u_int i = 0; i < vertex_count; ++i) {
        graph_matrix[i] = new u_char[vertex_count];
    }

    // Generate matrix
    if (edges_count <= vertex_count * (vertex_count - 1) / 2) {
        for (u_int i = 0; i < vertex_count; ++i) {
            for (u_int j = 0; j < vertex_count; ++j)
                graph_matrix[i][j] = 0;
        }
        for (u_int i = 0; i < edges_count; ++i) {
            u_int x, y;
            do {
                x = std::rand() % vertex_count;
                y = std::rand() % vertex_count;
            } while (x == y || graph_matrix[x][y] != 0);
            u_char value = std::rand() % 100 + 1;
            graph_matrix[x][y] = value;
        }
    } else {
        for (u_int x = 0; x < vertex_count; ++x) {
            for (u_int y = 0; y < vertex_count; ++y)
                graph_matrix[x][y] = (x != y) ? std::rand() % 100 + 1 : 0;
        }
        for (u_int i = 0; i < vertex_count * (vertex_count - 1) - edges_count; ++i) {
            u_int x, y;
            do {
                x = std::rand() % vertex_count;
                y = std::rand() % vertex_count;
            } while (graph_matrix[x][y] == 0);
            graph_matrix[x][y] = 0;
        }
    }

    // Write
    if (writing_work) {
        for (u_int x = 0; x < vertex_count; ++x) {
            for (u_int y = 0; y < vertex_count; ++y)
                std::cout << "\t" << (u_int)(graph_matrix[x][y]);
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    return graph_matrix;
}

/* SEQUENTIAL REALISATION */
res_format SequentialResult(u_char** graph_matrix, u_int vertex_count, u_int start_vertex, bool writing_work) {
    std::cout << "<-----SEQUATION REALISATION----->\n";

    // Memory allocation
    u_int** distance_matrix = new u_int*[vertex_count + 1];
    for (u_int i = 0; i <= vertex_count; ++i)
        distance_matrix[i] = new u_int[vertex_count];
    u_int* prev_vertex = new u_int[vertex_count];
    for (u_int i = 0; i < vertex_count; ++i)
       distance_matrix[0][i] = prev_vertex[i] = PATH_INFINITY;
    distance_matrix[0][start_vertex] = 0;
    prev_vertex[start_vertex] = start_vertex;
    bool check_compare = false;
    if (writing_work) {
        std::cout << "\tIteration 0:\t";
        for (u_int i = 0; i < vertex_count; ++i) {
            if (distance_matrix[0][i] == PATH_INFINITY)
                std::cout << "inf\t";
            else
                std::cout << distance_matrix[0][i] << "\t";
        }
        std::cout << "\n";
    }

    // Find distance matrix
    int iter;
    for (iter = 0; !check_compare; iter++) {
        check_compare = 1;
        // Copy elements
        for (u_int vertex = 0; vertex < vertex_count; ++vertex)
            distance_matrix[iter + 1][vertex] = distance_matrix[iter][vertex];
        // Update
        for (u_int from = 0; from < vertex_count; ++from) {
            if (distance_matrix[iter][from] == PATH_INFINITY) continue;
            if (iter != 0 && distance_matrix[iter][from] == distance_matrix[iter - 1][from]) continue;
            for (u_int to = 0; to < vertex_count; ++to) {
                if (graph_matrix[from][to] == 0) continue;
                if (distance_matrix[iter + 1][to] > distance_matrix[iter][from] + graph_matrix[from][to]) {
                    check_compare = 0;
                    distance_matrix[iter + 1][to] = distance_matrix[iter][from] + graph_matrix[from][to];
                    prev_vertex[to] = from;
                }
            }
        }
        // Write
        if (writing_work) {
            std::cout << "\tIteration " << (iter + 1) << ":\t";
            for (unsigned int i = 0; i < vertex_count; ++i) {
                if (distance_matrix[iter + 1][i] == PATH_INFINITY)
                    std::cout << "inf\t";
                else
                    std::cout << distance_matrix[iter + 1][i] << "\t";
            }
            std::cout << "\n";
        }
    }

    // Find vectors_paths
    std::vector<u_int>* path = new std::vector<u_int>[vertex_count];
    for (u_int vertex = 0; vertex < vertex_count; ++vertex) {
        if (distance_matrix[iter - 1][vertex] == PATH_INFINITY) continue;
        u_int cur_vertex = vertex;
        do {
            path[vertex].push_back(cur_vertex);
            cur_vertex = prev_vertex[cur_vertex];
        } while (path[vertex].back() != start_vertex);
        reverse(path[vertex].begin(), path[vertex].end());
    }

    // Return
    if (writing_work) std::cout << "\n";
    res_format result = { distance_matrix[iter - 1], path };
    for (int i = 0; i < iter - 1; ++i)
        delete[] distance_matrix[i];
    delete[] distance_matrix;
    delete[] prev_vertex;
    return result;
}

/* PARALLEL REALISATION */
res_format ParallelResult(u_char** graph_matrix, u_int vertex_count, u_int start_vertex, bool writing_work) {
    std::cout << "<-----PARALLEL REALISATION----->\n";

    // Memory allocation
    u_int** distance_matrix = new u_int*[vertex_count + 1];
    for (u_int i = 0; i <= vertex_count; ++i)
        distance_matrix[i] = new u_int[vertex_count];
    u_int* prev_vertex = new u_int[vertex_count];
    tbb::parallel_for(tbb::blocked_range<int>(0, vertex_count, 1), [=](const tbb::blocked_range<int>& r) {
        int begin = r.begin();
        int end = r.end();
        for (int i = begin; i != end; ++i)
            distance_matrix[0][i] = prev_vertex[i] = PATH_INFINITY;
    });
    distance_matrix[0][start_vertex] = 0;
    prev_vertex[start_vertex] = start_vertex;
    bool* check_compare = new bool;
    *check_compare = false;
    if (writing_work) {
        std::cout << "\tIteration 0:\t";
        for (u_int i = 0; i < vertex_count; ++i) {
            if (distance_matrix[0][i] == PATH_INFINITY)
                std::cout << "inf\t";
            else
                std::cout << distance_matrix[0][i] << "\t";
        }
        std::cout << "\n";
    }

    // Find distance matrix
    int iter;
    for (iter = 0; !*check_compare; iter++) {
        *check_compare = true;
        // Copy elements
        tbb::parallel_for(tbb::blocked_range<int>(0, vertex_count, 1), [=](tbb::blocked_range<int>& r) {
            int begin = r.begin();
            int end = r.end();
            for (int vertex = begin; vertex != end; ++vertex)
                distance_matrix[iter + 1][vertex] = distance_matrix[iter][vertex];
        });
        tbb::parallel_for(tbb::blocked_range<int>(0, vertex_count, 1), [=](tbb::blocked_range<int>& r) {
            int begin = r.begin();
            int end = r.end();
            for (int from = begin; from < end; ++from) {
                if (distance_matrix[iter][from] == PATH_INFINITY) continue;
                if (iter != 0 && distance_matrix[iter][from] == distance_matrix[iter - 1][from]) continue;
                for (u_int to = 0; to < vertex_count; ++to) {
                    if (graph_matrix[from][to] == 0) continue;
                    if (distance_matrix[iter + 1][to] > distance_matrix[iter][from] + graph_matrix[from][to]) {
                        *check_compare = false;
                        distance_matrix[iter + 1][to] = distance_matrix[iter][from] + graph_matrix[from][to];
                        prev_vertex[to] = from;
                    }
                }
            }
        });
        // Write
        if (writing_work) {
            std::cout << "\tIteration " << (iter + 1) << ":\t";
            for (unsigned int i = 0; i < vertex_count; ++i) {
                if (distance_matrix[iter + 1][i] == PATH_INFINITY)
                    std::cout << "inf\t";
                else
                    std::cout << distance_matrix[iter + 1][i] << "\t";
            }
            std::cout << "\n";
        }
    }

    // Find vectors_paths
    std::vector<u_int>* path = new std::vector<u_int>[vertex_count];
    tbb::parallel_for(tbb::blocked_range<int>(0, vertex_count, 1), [=](tbb::blocked_range<int>& r) {
        int begin = r.begin();
        int end = r.end();
        for (int vertex = begin; vertex != end; ++vertex) {
            if (distance_matrix[iter - 1][vertex] == PATH_INFINITY) continue;
            u_int cur_vertex = vertex;
            do {
                path[vertex].push_back(cur_vertex);
                cur_vertex = prev_vertex[cur_vertex];
            } while (path[vertex].back() != start_vertex);
            reverse(path[vertex].begin(), path[vertex].end());
        }
    });

    // Return
    if (writing_work) std::cout << "\n";
    res_format result = { distance_matrix[iter - 1], path };
    for (int i = 0; i < iter - 1; ++i)
        delete[] distance_matrix[i];
    delete[] distance_matrix;
    delete[] prev_vertex;
    return result;
}

/* PRINT RESULT */
void PrintResults(res_format result, u_int vertex_count, bool writing_work, bool is_sequential) {
    if (is_sequential)
        std::cout << "<-----SEQUENTIAL RESULTS----->\n";
    else
        std::cout << "<-----PARALLEL RESULTS----->\n";

    // Write
    u_int* distance = result.distance;
    std::vector<u_int>* path = result.path;
    if (writing_work) {
        for (u_int vertex = 0; vertex < vertex_count; ++vertex) {
            std::cout << "\t" << vertex + 1 << ")\t";
            if (distance[vertex] == PATH_INFINITY) {
                std::cout << "not path" << std::endl;
                continue;
            }
            std::cout << "Distance: " << distance[vertex] << std::endl;
            std::cout << "\t\tPath: ";
            std::cout << path[vertex][0] + 1;
            for (unsigned int i = 1; i < path[vertex].size(); ++i)
                std::cout << "->" << path[vertex][i] + 1;
            std::cout << "\n";
        }
    }
}

int main(int argc, char** argv) {
    // Graph params
    u_int vertex_count;
    u_int edges_count;
    u_char** graph_matrix;
    bool writing_work = 0;
    u_int start_vertex;
    u_int threads_count;

    // Enter graph params
    /*std::cout << "Enter count of threads: ";
    std::cin >> threads_count;
    std::cout << "Enter count of vertexes: ";
    std::cin >> vertex_count;
    std::cout << "Enter count of edges: ";
    std::cin >> edges_count;
    std::cout << "Enter start vertex: ";
    std::cin >> start_vertex;
    start_vertex--;
    std::cout << "Enter 1, if should print intermidiate information, else enter any other value: ";
    std::string s;
    std::cin >> s;
    if (s == "1") writing_work = true;*/
    threads_count = 4;
    vertex_count = 5;
    edges_count = 10;
    start_vertex = 0;
    writing_work = true;

    // Generate matrix
    tbb::task_scheduler_init init(threads_count);
    srand(static_cast<int>(time(0)));
    graph_matrix = GenerateMatrix(vertex_count, edges_count, writing_work);

    // Parallel realisation
    tbb::tick_count par_start_time, par_finish_time;
    par_start_time = tbb::tick_count::now();
    res_format par_result = ParallelResult(graph_matrix, vertex_count, start_vertex, writing_work);
    par_finish_time = tbb::tick_count::now();

    // Write parallel results
    PrintResults(par_result, vertex_count, writing_work, false);
    std::cout << "\tTIME: " << (par_finish_time - par_start_time).seconds() << "\n\n";

    // Free memory
    delete[] par_result.distance;
    for (u_int i = 0; i < par_result.path->size(); ++i)
        par_result.path[i].clear();
    delete[] par_result.path;

    // Sequential realisation
    tbb::tick_count seq_start_time, seq_finish_time;
    seq_start_time = tbb::tick_count::now();
    res_format seq_result = SequentialResult(graph_matrix, vertex_count, start_vertex, writing_work);
    seq_finish_time = tbb::tick_count::now();

    // Write sequential results
    PrintResults(seq_result, vertex_count, writing_work, true);
    std::cout << "\tTIME: " << (seq_finish_time - seq_start_time).seconds() << "\n\n";

    // Free memory
    delete[] seq_result.distance;
    for (u_int i = 0; i < seq_result.path->size(); ++i)
         seq_result.path[i].clear();
    delete[] seq_result.path;

    std::cout << "Acceleration: " << (seq_finish_time - seq_start_time).seconds()
        / (par_finish_time - par_start_time).seconds() << "\n";

    // system("pause");

    return 0;
}
