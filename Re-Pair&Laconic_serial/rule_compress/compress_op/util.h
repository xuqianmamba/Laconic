#pragma once
#include <assert.h>
#include <omp.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <vector>
#define VertexT int64_t
#define INIT 100000
#define INF 1e-6
#define OMP_THRESHOLD 1024

namespace py = pybind11;

/*
 *  convert csr to double vector
 */

double timestamp() {
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + 1e-6 * t.tv_usec;
}

void coo_convert(std::vector<std::vector<VertexT>> &graph,
                 std::vector<VertexT> &src, std::vector<VertexT> &dst,
                 VertexT vertex_cnt) {
    assert(src.size() == dst.size());
    VertexT edge_cnt = src.size();
    std::mutex mtx;
    uint num_thread = omp_get_max_threads();
    uint block_size = (edge_cnt + num_thread - 1) / num_thread;

#pragma omp parallel for schedule(dynamic)
    for (uint t = 0; t < num_thread; ++t) {
        std::vector<std::vector<VertexT>> local_graph(vertex_cnt);
        for (VertexT e = t * block_size;
             e < (t + 1) * block_size && e < edge_cnt; ++e) {
            local_graph[src[e]].push_back(dst[e]);
        }
        mtx.lock();
#pragma omp parallel for schedule(dynamic)
        for (VertexT i = 0; i < vertex_cnt; ++i) {
            graph[i].insert(graph[i].end(), local_graph[i].begin(),
                            local_graph[i].end());
        }
        mtx.unlock();
    }
}

/*
 *  convert csr to double vector out-edge
 */
void csr_convert(std::vector<std::vector<VertexT>> &graph,
                 std::vector<VertexT> &vlist, std::vector<VertexT> &elist,
                 VertexT vertex_cnt) {
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT v = 0; v < vertex_cnt; v++) {
        VertexT start = vlist[v];
        VertexT end = vlist[v + 1];
        for (VertexT e = start; e < end; e++) {
            graph[v].push_back(elist[e]);
        }
    }
}

/*
 * convert csr to double vector in-edge
 */

void csr_convert_idx(std::vector<std::vector<VertexT>> &graph,
                     std::vector<VertexT> &vlist, std::vector<VertexT> &elist,
                     VertexT vertex_cnt) {
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT v = 0; v < vertex_cnt; v++) {
        VertexT start = vlist[v];
        VertexT end = vlist[v + 1];
        for (VertexT e = start; e < end; e++) {
            if(elist[e] > vertex_cnt) std::cout << elist[e] << std::endl;
            graph[elist[e]].push_back(v);
        }
    }
}

void insert_spliter(std::vector<int> &vlist, std::vector<int> &elist,
                    std::vector<int> &re, int max_symbol) {
    int v, e;
    int start, end;
    if (vlist.size() == 0)
        return;
    int cur = max_symbol;
    for (v = 0; v < vlist.size() - 1; v++) {
        start = vlist[v];
        end = vlist[v + 1];
        for (e = start; e < end; e++) {
            re.push_back(elist[e]);
        }
        re.push_back(cur++);
    }
}

void genNewIdForRule(std::vector<VertexT> &newRuleId, VertexT &newRule_cnt,
                     std::vector<bool> &merge_flag, VertexT vertex_cnt,
                     VertexT rule_cnt) {
    VertexT cur = vertex_cnt;
    for (VertexT i = 0; i < rule_cnt; i++) {
        newRuleId[i] = cur;
        if (merge_flag[i] == false) {
            cur++;
        }
    }
    newRule_cnt = cur - vertex_cnt; // now rule count
}

void genNewGraphCOO(std::vector<VertexT> &src, std::vector<VertexT> &dst,
                    std::vector<std::vector<VertexT>> &graph,
                    std::vector<VertexT> &newRuleId,
                    std::vector<bool> &merge_flag, VertexT vertex_cnt) {
    VertexT g_size = graph.size();
    for (VertexT v = 0; v < g_size; ++v) {
        VertexT srcID = v;
        VertexT e_size = graph[srcID].size();
        VertexT n_srcID =
            srcID >= vertex_cnt ? newRuleId[srcID - vertex_cnt] : srcID;
        if (srcID >= vertex_cnt && merge_flag[srcID - vertex_cnt] == true)
            continue;
        for (VertexT e = 0; e < e_size; e++) {
            VertexT dstID = graph[srcID][e];
            VertexT n_dstID =
                dstID >= vertex_cnt ? newRuleId[dstID - vertex_cnt] : dstID;
            src.push_back(n_srcID);
            dst.push_back(n_dstID);
        }
    }
}

void genNewGraphCSR(std::vector<VertexT> &vlist, std::vector<VertexT> &elist,
                    std::vector<std::vector<VertexT>> &graph,
                    std::vector<VertexT> &newRuleId,
                    std::vector<bool> &merge_flag, VertexT vertex_cnt) {
    VertexT g_size = graph.size();
    vlist.push_back(0);
    VertexT e_size = 0;
    for (VertexT v = 0; v < g_size; ++v) {
        VertexT srcID = v;
        VertexT e_size = graph[srcID].size();
        if (srcID >= vertex_cnt && merge_flag[srcID - vertex_cnt] == true)
            continue;
        for (VertexT e = 0; e < e_size; e++) {
            VertexT dstID = graph[srcID][e];
            VertexT n_dstID =
                dstID >= vertex_cnt ? newRuleId[dstID - vertex_cnt] : dstID;
            elist.push_back(n_dstID);
        }
        e_size = elist.size();
        vlist.push_back(e_size);
    }
}

template <typename T> py::array_t<T> vector2numpy1D(std::vector<T> &vec) {
    size_t size = vec.size();
    auto result = py::array_t<T>(size);
    py::buffer_info buf = result.request();
    T *ptr = (T *)buf.ptr;
    memcpy(ptr, &vec[0], sizeof(T) * size);
    return result;
}

template <typename T>
void numpy2vector1D(py::array_t<T> &input, std::vector<T> &output) {
    py::buffer_info buf = input.request();
    size_t size = buf.size;
    output.resize(size);
    T *ptr = (T *)buf.ptr;
    std::copy(ptr, ptr + size, output.begin());
}

