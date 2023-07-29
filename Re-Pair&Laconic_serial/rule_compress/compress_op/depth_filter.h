#pragma once
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>
// #include "util.h"
#include <omp.h>

#include "gen_mask.h"
#include <queue>

void mergeRule(VertexT rule_id, std::vector<std::vector<VertexT>> &graph,
               std::vector<std::vector<VertexT>> &graphT, VertexT vertex_cnt) {
    VertexT ID = rule_id + vertex_cnt;

    // insert childnode to parent node
    for (VertexT i = 0; i < graphT[ID].size(); ++i) {
        VertexT node = graphT[ID][i];
        std::vector<VertexT>::iterator it =
            find(graph[node].begin(), graph[node].end(), ID);
        std::vector<VertexT>::iterator pos = graph[node].erase(it);
        graph[node].insert(pos, graph[ID].begin(), graph[ID].end());
    }
    // update child node idx
    for (VertexT i = 0; i < graph[ID].size(); ++i) {
        VertexT child_id = graph[ID][i];
        std::vector<VertexT>::iterator it =
            find(graphT[child_id].begin(), graphT[child_id].end(), ID);
        // delete origin idx in ruleID
        std::vector<VertexT>::iterator pos = graphT[child_id].erase(it);
        // insert new idx to child id
        graphT[child_id].insert(pos, graphT[ID].begin(), graphT[ID].end());
    }
}

void depth_filter(std::vector<std::vector<VertexT>> &graph,
                  std::vector<std::vector<VertexT>> &graphT,
                  std::vector<bool> &flag, py::array_t<int> &depth, int m_depth,
                  int max_depth, int min_edge, VertexT vertex_cnt,
                  VertexT rule_cnt) {
    py::buffer_info depth_buf = depth.request();
    int *depth_ptr = (int *)depth_buf.ptr;
    std::vector<int> depth_count(m_depth + 1, 0);
    std::vector<int> degree(rule_cnt, 0);
    for (VertexT i = vertex_cnt; i < vertex_cnt + rule_cnt; i++) {
        degree[i - vertex_cnt] = graph[i].size();
    }
    for (VertexT i = 0; i < rule_cnt; i++) {
        depth_count[depth_ptr[i + vertex_cnt]] += degree[i];
    }
    // std::cout << "depth:" << m_depth << std::endl;
    // for(int i = 0; i < m_depth; i++){
    //     std::cout << depth_count[i] << std::endl;
    // }
    while (m_depth >= max_depth) {
        if (depth_count[m_depth] >= min_edge) {
            m_depth--;
            continue;
        }
        // std::cout << depth_count[m_depth] << std::endl;
        int r = 0;
        for (r = 0; r < rule_cnt; r++) {
            if (depth_ptr[r + vertex_cnt] == m_depth) {
                mergeRule(r, graph, graphT, vertex_cnt);
                flag[r] = true;
            }
        }
        m_depth--;
        for (VertexT i = vertex_cnt; i < vertex_cnt + rule_cnt; i++) {
            degree[i - vertex_cnt] = graph[i].size();
        }
        for (VertexT i = 0; i <= m_depth; i++) {
            depth_count[i] = 0;
        }
        for (VertexT i = 0; i < rule_cnt; i++) {
            depth_count[depth_ptr[i + vertex_cnt]] +=
                flag[i] == true ? 0 : degree[i];
        }
    }
}

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>, VertexT, VertexT>
depth_filter_csr(py::array_t<VertexT> &np_input_vlist,
                 py::array_t<VertexT> &np_input_elist, VertexT vertex_cnt,
                 VertexT rule_cnt, int max_depth, int min_edge) {
    std::vector<VertexT> vlist;
    std::vector<VertexT> elist;
    numpy2vector1D(np_input_vlist, vlist);
    numpy2vector1D(np_input_elist, elist);
    std::vector<std::vector<VertexT>> graph(vertex_cnt + rule_cnt);
    csr_convert(graph, vlist, elist, vertex_cnt + rule_cnt);
    std::vector<std::vector<VertexT>> graphT(vertex_cnt + rule_cnt);
    csr_convert_idx(graphT, vlist, elist, vertex_cnt + rule_cnt);
    auto depth_re =
        gen_vertex_order_csr(np_input_vlist, np_input_elist, vertex_cnt, rule_cnt);
    py::array_t<int> depth = std::get<0>(depth_re);
    int m_depth = std::get<1>(depth_re);
    // std::cout << m_depth << std::endl;
    // std::cout << max_depth << std::endl;
    std::vector<bool> flag(rule_cnt, false);
    depth_filter(graph, graphT, flag, depth, m_depth, max_depth, min_edge,
                 vertex_cnt, rule_cnt);
    VertexT newRule_cnt;
    std::vector<VertexT> newRuleId(rule_cnt);
    genNewIdForRule(newRuleId, newRule_cnt, flag, vertex_cnt, rule_cnt);
    std::vector<VertexT> new_vlist;
    std::vector<VertexT> new_elist;
    genNewGraphCSR(new_vlist, new_elist, graph, newRuleId, flag, vertex_cnt);
    py::array_t<int> np_vlist = vector2numpy1D(new_vlist);
    py::array_t<int> np_elist = vector2numpy1D(new_elist);
    return {np_vlist, np_elist, vertex_cnt, newRule_cnt};
}
