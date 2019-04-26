// Copyright 2019 Sitkin Dmitry
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <utility>
#include <random>
#include <ctime>
#include <queue>
#include <iostream>
#include <string>

#define INFINITI 10000000
#define WEIGHT 5
#define NUM_OF_VERTEX 10000
// using namespace std;

void print_d(int* d, int size) {
    for (int i = 0; i < size; i++) {
        std::cout << d[i] << " ";
    }
    std::cout << std::endl;
}

void print_graph(int** G, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            std::cout << G[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int* init_round(int countEdge, int countVertex) {
    int* one_graph = new int[countVertex * countVertex];
    int** graph = new int*[countVertex];

    for (int i = 0; i < countVertex; i++) {
        graph[i] = new int[countVertex];
    }
    srand((unsigned int)std::time(NULL));
    for (int i = 0; i < countVertex; i++) {
        for (int j = 0; j < countVertex; j++) {
            if (i == j) {
                graph[i][j] = 0;
            } else {
                if (j == i + 1) {
                    graph[i][j] = 1 + std::rand() % WEIGHT;
                } else {
                    graph[i][j] = INFINITI;
                }
            }
        }
    }
    // print_graph(graph, countVertex);

    graph[countVertex - 1][0] = 1 + std::rand() % WEIGHT;
    for (int i = 0, t = 0; i < countVertex; i++) {
        for (int j = 0; j < countVertex; j++) {
            one_graph[t] = graph[j][i];
            t++;
        }
    }
    return one_graph;
}

    int* init_graph(int countEdge, int countVertex) {
        int* one_graph = new int[countVertex * countVertex];
        int** graph = new int*[countVertex];

        for (int i = 0; i < countVertex; i++) {
            graph[i] = new int[countVertex];
        }
        srand((unsigned int)std::time(NULL));

        for (int i = 0; i < countVertex; i++) {
            for (int j = 0; j < countVertex; j++) {
                if (i == j) {
                    graph[i][j] = 0;
                } else {
                    graph[i][j] = std::rand() % WEIGHT;
                    if (graph[i][j] == 0) {
                        graph[i][j] = INFINITI;
                    }
                }
            }
        }
        for (int i = 0, t = 0; i < countVertex; i++) {
            for (int j = 0; j < countVertex; j++) {
                one_graph[t] = graph[j][i];
                t++;
            }
        }
    return one_graph;
}


int* init_d(int size) {
    int* d = new int[size];
    for (int i = 0; i < size; i++) {
        d[i] = INFINITI;
    }
    return d;
}

int* d_for_round(int* graph, int start, int count_vertex) {
    int* d = init_d(count_vertex);
    d[start] = 0;
    for (int ofset = 0, i = 1 + start; (i < count_vertex); ofset++, i++) {
        d[i] = d[i - 1] + graph[i*count_vertex + start + ofset];
    }
    for (int i = 0; i < start; i++) {
        if (i == 0) {
            d[i] = d[count_vertex - 1] + graph[count_vertex - 1];
        } else {
            d[i] = d[i - 1] + graph[i*count_vertex + (i - 1)];
        }
    }
    return d;
}
void compare(int* d, int* ideal, int size) {
    int mistakes = 0;
    for (int i = 0; i < size; i++)
    if ((d[i] != ideal[i])) {
        mistakes++;
    }
    // std::cout << "MISTAKES: " << mistakes << std::endl;

    if (mistakes == 0) {
        std::cout << "It's OK" << std::endl;
    } else {
        std::cout << "It's NOT OK" << std::endl;
    }
}
int* dijkstra(int* graph, int start, int count_vertex) {
    int* d = init_d(count_vertex);
    d[start] = 0;
    std::priority_queue<std::pair<int, int>>  queue;
    queue.push(std::make_pair(0, start));
    while (!queue.empty()) {
        int v = queue.top().second, cur_d = queue.top().first;
        queue.pop();

        if (cur_d > d[v])  continue;

        for (int i = 0; i < count_vertex; ++i) {
            int to = i, len = graph[i * count_vertex + v];
            if (d[v] + len < d[to]) {
                d[to] = d[v] + len;
                queue.push(std::make_pair(d[to], to));
            }
        }
    }
    return d;
}

int* parallel_dijkstra(int* graph, int start, int count_vertex) {
    int* d = init_d(count_vertex);
    d[start] = 0;
    std::priority_queue<std::pair<int, int>>  queue;
    queue.push(std::make_pair(0, start));
    while (!queue.empty()) {
        int v = queue.top().second, cur_d = queue.top().first;
        queue.pop();

        if (cur_d > d[v])  continue;

#pragma omp parallel for
            for (int i = 0; i < count_vertex; ++i) {
                int to = i, len = graph[i * count_vertex + v];
                if (d[v] + len < d[to]) {
                    d[to] = d[v] + len;
                    #pragma omp critical
                    queue.push(std::make_pair(d[to], to));
                }
            }
    }
    return d;
}
int main(int argc, char *argv[]) {
    srand((unsigned int)std::time(NULL));
    omp_set_num_threads(6);
    int vert = NUM_OF_VERTEX;
    int count_edge = (vert - 1) + std::rand() % ((vert * (vert - 1)) / 2);
    int start = std::rand() % (NUM_OF_VERTEX - 1);

    int *graph = init_graph(count_edge, NUM_OF_VERTEX);
    int *result = NULL;  // = dijkstra(graph, start, NUM_OF_VERTEX);
    int *parallel_result = NULL;

    double t1 = 0.0;
    double t2 = 0.0;
    double time, ptime;

    t1 = omp_get_wtime();
    result = dijkstra(graph, start, NUM_OF_VERTEX);
    t2 = omp_get_wtime();

    // std::cout <<"Dijkstra" <<std::endl;
    // for (int i = NUM_OF_VERTEX-1000; i < NUM_OF_VERTEX-950; i++)
    // {
    //    std::cout << parallel_result[i] << "  ";
    // }
    // std::cout << std::endl;
    // print_d(parallel_result, NUM_OF_VERTEX);
    // std::cout<<"Ideal" << std::endl;
    /*for (int i = NUM_OF_VERTEX - 1000; i < NUM_OF_VERTEX-950; i++)
    {
        std::cout << ideal_d[i] << "  ";
    }
    std::cout << std::endl;*/
    // print_d(ideal_d, NUM_OF_VERTEX);
    time = t2 - t1;
    std::cout << "time = " << time << std::endl;
    t1 = omp_get_wtime();
    parallel_result = parallel_dijkstra(graph, start, NUM_OF_VERTEX);
    t2 = omp_get_wtime();
    ptime = t2 - t1;
    std::cout << "ptime = " << ptime << std::endl;
    std::cout << "Perfomance: " << time / ptime << std::endl;
    compare(parallel_result, result, NUM_OF_VERTEX);
    return 0;
}
