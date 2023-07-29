#pragma once
#include <iostream>
#include <vector>
// #include <omp.h>
#include "assert.h"
#include "util.h"



std::tuple<py::array_t<int>, int> gen_edge_order(py::array_t<VertexT> &np_src,
                                                 py::array_t<VertexT> &np_dst,
                                                 VertexT vertex_cnt,
                                                 VertexT rule_cnt) {
    py::buffer_info src_buf = np_src.request();
    size_t src_size = src_buf.size;
    VertexT *src = (VertexT *)src_buf.ptr;
    py::buffer_info dst_buf = np_dst.request();
    size_t dst_size = dst_buf.size;
    VertexT *dst = (VertexT *)dst_buf.ptr;
    assert(src_size == dst_size);
    VertexT edge_cnt = src_size;
    auto order = py::array_t<int>(edge_cnt);
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    // memset(order_ptr, -1, edge_cnt);
    for (VertexT i = 0; i < edge_cnt; i++) {
        order_ptr[i] = INIT;
    }
    std::vector<int> in_degree(rule_cnt, 0);
    VertexT process_cnt = 0;
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT i = 0; i < edge_cnt; ++i) {
        VertexT dstID = dst[i];
        if (dstID >= vertex_cnt) {
            VertexT ruleID = dstID - vertex_cnt;
            // #pragma omp atomic
            in_degree[ruleID]++;
        }
    }
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT i = 0; i < edge_cnt; ++i) {
        VertexT srcID = src[i];
        VertexT dstID = dst[i];
        if (srcID < vertex_cnt) {
            order_ptr[i] = 0;
            // #pragma omp atomic
            process_cnt++;
            if (dstID >= vertex_cnt) {
                VertexT ruleID = dstID - vertex_cnt;
                // #pragma omp atomic
                in_degree[ruleID]--;
            }
        }
    }
    int step = 1;
    while (process_cnt < edge_cnt) {
        std::vector<int> tmp_degree(rule_cnt, 0);
        // #pragma omp parallel for schedule(dynamic)
        for (VertexT i = 0; i < edge_cnt; ++i) {
            VertexT srcID = src[i];
            VertexT dstID = dst[i];
            if (srcID >= vertex_cnt) { // rule
                if (in_degree[srcID - vertex_cnt] == 0 &&
                    order_ptr[i] == INIT) {
                    order_ptr[i] = step;
                    // #pragma omp atomic
                    process_cnt++;
                    if (dstID >= vertex_cnt &&
                        in_degree[dstID - vertex_cnt] > 0) {
                        VertexT ruleID = dstID - vertex_cnt;
                        // #pragma omp atomic
                        tmp_degree[ruleID]++;
                    }
                }
            }
        }
        // #pragma omp parallel for schedule(dynamic)
        for (VertexT i = 0; i < rule_cnt; ++i) {
            in_degree[i] -= tmp_degree[i];
        }
        step++;
    }
    return {order, step};
}

std::tuple<py::array_t<bool>, int>
gen_mask_edge(py::array_t<VertexT> &np_src, py::array_t<VertexT> &np_dst,
                     VertexT vertex_cnt, VertexT rule_cnt) {
    auto result = gen_edge_order(np_src, np_dst, vertex_cnt, rule_cnt);
    py::array_t<int> depth = std::get<0>(result);
    int m_depth = std::get<1>(result);
    py::buffer_info depth_buf = depth.request();
    int *depth_ptr = (int *)depth_buf.ptr;
    py::buffer_info src_buf = np_src.request();
    int size_ = src_buf.size;
    auto mask = py::array_t<bool>(size_ * m_depth);
    mask.resize({m_depth, size_});
    py::buffer_info mask_buf = mask.request();
    bool *mask_ptr = (bool *)mask_buf.ptr;
    size_t s;
    int d;
    for (d = 0; d < m_depth; ++d) {
        for (s = 0; s < size_; s++) {
            mask_ptr[d * size_ + s] = depth_ptr[s] == d ? true : false;
        }
    }
    return {mask, m_depth};
}

/*
 * Generate vertex order for compressed graph of CSR format
 */
std::tuple<py::array_t<int>, int>
gen_vertex_order_csc(const py::array_t<VertexT> &np_input_vlist,
                 const py::array_t<VertexT> &np_input_elist, VertexT vertex_cnt,
                 VertexT rule_cnt) {
    auto order = py::array_t<int>(vertex_cnt + rule_cnt);
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    for (VertexT i = 0; i < vertex_cnt + rule_cnt; ++i) {
        order_ptr[i] = INIT;
    }
    py::buffer_info vlist_buf = np_input_vlist.request();
    VertexT *vlist = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_input_elist.request();
    VertexT *elist = (VertexT *)elist_buf.ptr;
    VertexT edge_cnt = elist_buf.size;
    std::vector<int> in_degree(rule_cnt, 0);
    VertexT start, end, src, dst, eid;
    for (src = 0; src < rule_cnt; ++src) {
        start = vlist[src + vertex_cnt];
        end = vlist[src + vertex_cnt + 1];
        for (eid = start; eid < end; ++eid) {
            dst = elist[eid];
            if(dst >= vertex_cnt) in_degree[dst-vertex_cnt]++;
        }
    }
    std::vector<VertexT> in_worklist;
    std::vector<VertexT> out_worklist;
    for(src = 0; src < rule_cnt; ++src){
        if(in_degree[src] == 0){
            order_ptr[src+vertex_cnt] = 0;
            in_worklist.emplace_back(src+vertex_cnt);
        }
    }
    int step = 1;
    while (!in_worklist.size() == 0) {
        VertexT w_size = in_worklist.size();
        // #pragma omp parallel for schedule(dynamic)
        for (VertexT i = 0; i < w_size; i++) {
            src = in_worklist[i];
            start = vlist[src];
            end = vlist[src + 1];
            for (eid = start; eid < end; eid++) {
                dst = elist[eid];
                if (order_ptr[dst] == INIT && dst >= vertex_cnt) {
                    VertexT ruleID = dst-vertex_cnt;
                    in_degree[ruleID]--;
                    if (in_degree[ruleID] == 0) {
                        order_ptr[dst] = step;
                        out_worklist.emplace_back(dst);
                    }
                }
            }
        }
        step++;
        std::swap(in_worklist, out_worklist);
        out_worklist.clear();
    }
    for(src = 0; src < vertex_cnt; ++src){
        order_ptr[src] = step-1;
    }
    return {order, step};
}


/*
 * Generate vertex order for compressed graph of CSR format
 */
std::tuple<py::array_t<int>, int>
gen_vertex_order_csr(const py::array_t<VertexT> &np_input_vlist,
                 const py::array_t<VertexT> &np_input_elist, VertexT vertex_cnt,
                 VertexT rule_cnt) {
    auto order = py::array_t<int>(vertex_cnt + rule_cnt);
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    for (VertexT i = 0; i < vertex_cnt + rule_cnt; ++i) {
        order_ptr[i] = INIT;
    }
    py::buffer_info vlist_buf = np_input_vlist.request();
    VertexT *vlist = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_input_elist.request();
    VertexT *elist = (VertexT *)elist_buf.ptr;
    VertexT edge_cnt = elist_buf.size;
    std::vector<int> in_degree(rule_cnt, 0);
    VertexT start, end, src, dst, eid;
    for(src = 0; src < edge_cnt; ++src){
        if(elist[src] >= vertex_cnt){
            in_degree[elist[src]-vertex_cnt]++;
        }
    }
    std::vector<VertexT> in_worklist;
    std::vector<VertexT> out_worklist;
    for (src = 0; src < vertex_cnt; ++src) {
        order_ptr[src] = 0;
        in_worklist.emplace_back(src);
    }
    int step = 1;
    while (!in_worklist.size() == 0) {
        VertexT w_size = in_worklist.size();
        // #pragma omp parallel for schedule(dynamic)
        for (VertexT i = 0; i < w_size; i++) {
            src = in_worklist[i];
            start = vlist[src];
            end = vlist[src + 1];
            for (eid = start; eid < end; eid++) {
                dst = elist[eid];
                if(order_ptr[dst] == INIT && dst >= vertex_cnt){
                    VertexT ruleID = dst-vertex_cnt;
                    in_degree[ruleID]--;
                    if(in_degree[ruleID] == 0){
                        order_ptr[dst] = step;
                        out_worklist.emplace_back(dst);
                    }
                }
            }
        }
        step++;
        std::swap(in_worklist, out_worklist);
        out_worklist.clear();
    }
    return {order, step - 1};
}



std::tuple<py::array_t<bool>, int>
gen_mask_vertex(const py::array_t<VertexT> &np_src, const py::array_t<VertexT> &np_dst,
                     VertexT vertex_cnt, VertexT rule_cnt, std::string graph_type="csc") {
    assert(graph_type != "csc" && graph_type != "csr");

    if(graph_type == "csc"){
        auto result = gen_vertex_order_csc(np_src, np_dst, vertex_cnt, rule_cnt);
        py::array_t<int> depth = std::get<0>(result);
        int m_depth = std::get<1>(result);
        py::buffer_info depth_buf = depth.request();
        int *depth_ptr = (int *)depth_buf.ptr;
        int size = vertex_cnt + rule_cnt;
        auto mask = py::array_t<bool>(size * m_depth);
        mask.resize({m_depth, size});
        py::buffer_info mask_buf = mask.request();
        bool *mask_ptr = (bool *)mask_buf.ptr;
        size_t s;
        int d;
        for (d = 0; d < m_depth; ++d) {
            for (s = 0; s < size; s++) {
                mask_ptr[d * size + s] = depth_ptr[s] == d ? true : false;
            }
        }
        return {mask, m_depth};
    }
    if(graph_type == "csr"){
        auto result = gen_vertex_order_csr(np_src, np_dst, vertex_cnt, rule_cnt);
        py::array_t<int> depth = std::get<0>(result);
        int m_depth = std::get<1>(result);
        py::buffer_info depth_buf = depth.request();
        int *depth_ptr = (int *)depth_buf.ptr;
        int size = vertex_cnt + rule_cnt;
        auto mask = py::array_t<bool>(size * m_depth);
        mask.resize({m_depth, size});
        py::buffer_info mask_buf = mask.request();
        bool *mask_ptr = (bool *)mask_buf.ptr;
        size_t s;
        int d;
        for (d = 0; d < m_depth; ++d) {
            for (s = 0; s < size; s++) {
                mask_ptr[d * size + s] = depth_ptr[s] == d ? true : false;
            }
        }
        return {mask, m_depth};
    }
}