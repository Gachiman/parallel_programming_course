// Copyright Bogatova Margarita
#include <time.h>
#include <stdlib.h>
#include <omp.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
using DstType = int;
using GraphType = std::vector< std::vector <DstType> >;

// Secondary functions
void PrintGraph(const GraphType& g);
GraphType EnterGraph(std::size_t n, std::size_t m);
GraphType RandomGenerateGraph(std::size_t n, std::size_t m);
void PrintPathToVertex
        (const std::vector<int>& prev, int source_vertex, int curr_v);

// Functions for sequential implementation
double SeqImplementation(int start, std::size_t n, GraphType g,
    std::vector<DstType>* dst, std::vector<int>* prev);
void FindDistance(int start, std::size_t n, const GraphType& d,
    std::vector<DstType>* dst, std::vector<int>* prev_v);

// Functions for parallel implementation
double ParallelImplementation(int start, std::size_t n, GraphType g,
    std::vector<DstType>*dst, std::vector<int>* prev);
void FindDistanceOMP(int start, std::size_t n, const GraphType& d,
    std::vector<DstType>*dst, std::vector<int>* prev_v);

// Functions to verify the correctness of the algorithm
void GraphExample();
bool VerifyCorrectness(
    const std::vector<DstType>& dst1, const std::vector<int>& prev1,
    const std::vector<DstType>& dst2, const std::vector<int>& prev2);

int main(int argc, char** argv) {
    GraphType g;
    std::vector<int> prev_seq, prev_par;
    std::vector<DstType> dist_seq, dist_par;
    std::size_t n, m;
    double seq_time, par_time;
    int start, num_threads;
    // int response;
    // if (argc == 4) {
    //     n = std::atoi(argv[1]);
    //     m = std::atoi(argv[2]);
    //     num_threads = std::atoi(argv[3]);
    // } else {
    //     std::cout << "Enter the number of vertices: ";
    //     std::cin >> n;
    //     std::cout << "Enter the number of edges: ";
    //     std::cin >> m;
    //     std::cout << "Enter the number of threads: ";
    //     std::cin >> num_threads;
    // }
    // std::cout << "To generate an adjacency matrix randomly\t"
    //     << "press 1" << std::endl;
    // std::cout << "To enter the adjacency matrix from the keyboard\t"
    //     << "press 2" << std::endl;
    // std::cout << "Your input: ";
    // std::cin >> response;
    //
    // if (response == 2) {
    //     g = EnterGraph(n, m);
    // } else {
    //     g = RandomGenerateGraph(n, m);
    //     if (n <= 50) PrintGraph(g);
    // }
    //
    // std::cout << "Enter start  vertex: ";
    // std::cin >> start;

    n = 10;
    m = 45;
    num_threads = 4;
    start = 1;
    g = RandomGenerateGraph(n, m);
    try {
        double res = SeqImplementation(start, n, g, &dist_seq, &prev_seq);
        seq_time = res;
        omp_set_num_threads(num_threads);
        res = ParallelImplementation(start, n, g, &dist_par, &prev_par);
        par_time = res;
    } catch(std::out_of_range e) {
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Sequmental implementation takes time:\t" <<
        seq_time << " ms" << std::endl;
    std::cout << "Parallel implementation takes time:\t" <<
        par_time << " ms" << std::endl;
    std::cout << "The correctness of the algorithm:\t" << std::boolalpha
    << VerifyCorrectness(dist_seq, prev_seq, dist_par, prev_par) << std::endl;
    std::cout << "Acceleration:\t\t\t\t" << seq_time / par_time << std::endl;

    return 0;
}

void PrintGraph(const GraphType& g) {
    std::cout << "Adjacency matrix: " << std::endl;
    for (const auto& s : g) {
        for (const auto& c : s) {
            std::cout << c << "\t";
        }
        std::cout << std::endl;
    }
}
GraphType RandomGenerateGraph(std::size_t n, std::size_t m) {
    GraphType g;
    int max_e = n*(n - 1) / 2;
    srand(static_cast<unsigned int>(time(0)));
    g.resize(n);
    for (int i = 0; i < static_cast<int>(n); ++i) {
        g[i].assign(n, -1);
        g[i][i] = 0;
    }
    if (static_cast<int>(m) < max_e >> 2) {
        for (int j = 0; j < static_cast<int>(m); ++j) {
            int a = 0, b = 0;
            while (true) {
                a = std::rand() % n;
                b = std::rand() % n;
                if ((g[b][a] == -1 || g[a][b] == -1) ||
                    j + 1 > static_cast<int>(n*(n - 1) / 2))
                    break;
            }
            DstType dst = std::rand() % 100 + 1;
            g[a][b] = dst;
            g[b][a] = dst;
        }
    } else {
        for (int i = 0; i < static_cast<int>(n); ++i) {
            for (int j = i + 1; j < static_cast<int>(n); ++j) {
                DstType dst = std::rand() % 100 + 1;
                g[i][j] = dst;
                g[j][i] = dst;
            }
        }
        for (int j = 0; j < max_e -static_cast<int>(m); ++j) {
            int a = 0, b = 0;
            while (true) {
                a = std::rand() % n;
                b = std::rand() % n;
                if ((g[b][a] != -1 && g[a][b] != -1))
                    break;
            }
            g[a][b] = -1;
            g[b][a] = -1;
        }
    }
    return g;
}
GraphType EnterGraph(std::size_t n, std::size_t m) {
    GraphType g;
    g.resize(n);
    for (int i = 0; i < static_cast<int>(n); ++i) {
        for (int j = 0; j < static_cast<int>(n); ++j) {
            DstType d;
            std::cin >> d;
            g[i].push_back(d);
        }
    }
    return g;
}
void PrintPathToVertex(
    const std::vector<int>& prev, int source_vertex, int curr_v) {
    if (curr_v != source_vertex) {
        PrintPathToVertex(prev, source_vertex, prev[curr_v]);
    }
    std::cout << curr_v + 1 << " ";
}

double SeqImplementation(int start, std::size_t n, GraphType g,
            std::vector<DstType>*dst, std::vector<int>* prev) {
    double s_time, f_time;
    s_time = omp_get_wtime();
    FindDistance(start, n, g, dst, prev);
    f_time = omp_get_wtime();
    return f_time - s_time;
}
void FindDistance(int start, std::size_t n, const GraphType& d,
            std::vector<DstType>*dst, std::vector<int>* prev_v) {
    if (start < 1 || start > static_cast<int>(n))
        throw std::out_of_range("Incorrect start or finish vertex");
    (*dst).assign(n, -1);
    (*prev_v).assign(n, -1);

    /* Lambda function that returns true if the distance to the
    vertex a is less than the distance to the vertex b    */
    auto set_function = [dst](DstType a, DstType b) {
        return ((*dst)[a] <(*dst)[b]) ||
            (((*dst)[a] == (*dst)[b]) && (a < b));
    };

    // Set of unvisited vertices, ordered by non-decreasing distances to them
    std::set<int, decltype(set_function) > dst_queue(set_function);

    (*dst)[start - 1] = 0;
    dst_queue.insert(start - 1);

    while (!dst_queue.empty()) {
        int next_vertex = *dst_queue.begin();
        dst_queue.erase(dst_queue.begin());
        for (int i = 0; i < static_cast<int>(n); ++i) {
            if (!d[next_vertex][i] || d[next_vertex][i] == -1)
                continue;
            if ((*dst)[i] >(*dst)[next_vertex] + d[next_vertex][i]
                                                || (*dst)[i] == -1) {
                (*prev_v)[i] = next_vertex;
                dst_queue.erase(i);
                (*dst)[i] = (*dst)[next_vertex] + d[next_vertex][i];
                dst_queue.insert(i);
            }
        }
    }
}

double ParallelImplementation(int start, std::size_t n, GraphType g,
            std::vector<DstType>*dst, std::vector<int>* prev) {
    double s_time, f_time;
    s_time = omp_get_wtime();
    FindDistanceOMP(start, n, g, dst, prev);
    f_time = omp_get_wtime();
    return f_time - s_time;
}
void FindDistanceOMP(int start, std::size_t n, const GraphType& d,
            std::vector<DstType>* dst, std::vector<int>* prev_v) {
    if (start < 1 || start > static_cast<int>(n))
        throw std::out_of_range("Incorrect start or finish vertex");
    dst->assign(n, -1);
    prev_v->assign(n, -1);
    /* Lambda function that returns true if the distance to the
    vertex a is less than the distance to the vertex b    */
    auto set_function = [dst](DstType a, DstType b) {
        return ((*dst)[a] < (*dst)[b]) ||
            (((*dst)[a] == (*dst)[b]) && (a < b));
    };

    // Set of unvisited vertices, ordered by non-decreasing distances to them
    std::set<int, decltype(set_function) > dst_queue(set_function);

    (*dst)[start - 1] = 0;
    dst_queue.insert(start - 1);

    int i;
    while (!dst_queue.empty()) {
        int next_vertex = *dst_queue.begin();
         dst_queue.erase(dst_queue.begin());
#pragma omp parallel for private(i)
        for (i = 0; i < static_cast<int>(n); ++i) {
            if (!d[next_vertex][i] || d[next_vertex][i] == -1)
                continue;
            if ((*dst)[i] > (*dst)[next_vertex] + d[next_vertex][i]
                                                || (*dst)[i] == -1) {
                (*prev_v)[i] = next_vertex;
#pragma omp critical
                {
                dst_queue.erase(i);
                (*dst)[i] = (*dst)[next_vertex] + d[next_vertex][i];
                dst_queue.insert(i);
                }
            }
        }
    }
}

void GraphExample() {
    DstType mx[6][6] = {
        {0, 7, 9, -1, -1, 14},
        {7, 0, 10, 15, -1, -1},
        {9, 10, 0, 11, -1, 2},
        {-1, 15, 11, 0, 6, -1},
        {-1, -1, -1, 6, 0, 9},
        {14, -1, 2, -1, 9, 0}
    };
    GraphType g;
    std::vector<int> prev, dst;
    int n = 6, start = 1;

    g.resize(n);
    for (int i = 0; i < static_cast<int>(n); ++i) {
        for (int j = 0; j < static_cast<int>(n); ++j) {
            g[i].push_back(mx[i][j]);
        }
    }

    FindDistance(start, n, g, &dst, &prev);
    for (int i = 0; i < n; ++i) {
        std::cout << "Shortest distance between " << start << " and " << i + 1
            << " is\t" << dst[i] << std::endl;
        if (dst[i] != -1) {
            std::cout << "Path: ";
            PrintPathToVertex(prev, start - 1, i);
            std::cout << std::endl;
        } else {
            std::cout << "There is no path to the vertex" << std::endl;
        }
    }
}

bool VerifyCorrectness(
    const std::vector<DstType>& dst1, const std::vector<int>& prev1,
    const std::vector<DstType>& dst2, const std::vector<int>& prev2) {
    return dst1 == dst2 && prev1 == prev2;
}
