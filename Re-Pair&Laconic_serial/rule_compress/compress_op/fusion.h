#pragma once
#include <cstdio>
#include <iostream>
#include <vector>
// #include <omp.h>
#include "assert.h"
#include "util.h"

bool cond_propagate(std::vector<int> &PC, std::vector<int> &AC,
                    std::vector<int> &in_degree, VertexT *vlist, VertexT *elist,
                    VertexT vid) {
    if (AC[vid] < in_degree[vid])
        return false;
    if (PC[vid] == 0)
        return true;
    VertexT start = vlist[vid];
    VertexT end = vlist[vid + 1];
    VertexT eid;
    for (eid = start; eid < end; eid++) {
        VertexT dst = elist[eid];
        if (PC[vid] > PC[dst])
            return false;
    }
    return true;
}

std::tuple<py::array_t<int>, int>
gen_fusion_vertex_order(py::array_t<VertexT> &np_vlist,
                        py::array_t<VertexT> &np_elist, VertexT vertex_cnt,
                        VertexT rule_cnt, int num_p) {
    auto order = py::array_t<int>(num_p * (vertex_cnt + rule_cnt));
    order.resize({num_p, int(vertex_cnt + rule_cnt)});
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    std::vector<int> PC(vertex_cnt + rule_cnt, 0);
    std::vector<int> AC(vertex_cnt + rule_cnt, 0);
    std::vector<int> tmp_AC(vertex_cnt + rule_cnt, 0);
    std::vector<int> tmp_PC(vertex_cnt + rule_cnt, 0);
    VertexT active_count = 0;
    for (VertexT i = 0; i < num_p * (vertex_cnt + rule_cnt); i++) {
        order_ptr[i] = INIT;
    }
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist = (VertexT *)elist_buf.ptr;
    VertexT edge_cnt = elist_buf.size;
    std::vector<int> in_degree(vertex_cnt + rule_cnt, 0);
    for (VertexT i = 0; i < edge_cnt; ++i) {
        VertexT dst = elist[i];
        in_degree[dst]++;
    }
    int step = 0;
    VertexT vid;
    VertexT eid;
    std::vector<VertexT> active_list;
    VertexT old_act = 0;
    while (active_count < num_p * (vertex_cnt + rule_cnt)) {
        std::cout << "Step" << step << std::endl;
        for (vid = 0; vid < vertex_cnt + rule_cnt; vid++) {
            if ((step == 0 && vid < vertex_cnt) ||
                (cond_propagate(PC, AC, in_degree, vlist, elist, vid) &&
                 PC[vid] < num_p)) {
                active_list.emplace_back(vid);
                active_count++;
                order_ptr[PC[vid] * (vertex_cnt + rule_cnt) + vid] = step;
                tmp_PC[vid]++;
            }
        }
        for (auto it = active_list.begin(); it != active_list.end(); it++) {
            vid = *it;
            tmp_AC[vid] = 0;
        }
        for (auto it = active_list.begin(); it != active_list.end(); it++) {
            vid = *it;
            VertexT start = vlist[vid];
            VertexT end = vlist[vid + 1];
            for (eid = start; eid < end; ++eid) {
                VertexT dst = elist[eid];
                tmp_AC[dst]++;
            }
        }
        active_list.resize(0);
        std::cout << "Active count:" << active_count - old_act << std::endl;
        old_act = active_count;
        std::copy(tmp_AC.begin(), tmp_AC.end(), AC.begin());
        std::copy(tmp_PC.begin(), tmp_PC.end(), PC.begin());
        step++;
    }
    return {order, step};
}

std::tuple<py::array_t<bool>, int>
fusion_mask_csr(py::array_t<VertexT> &np_vlist, py::array_t<VertexT> &np_elist,
                VertexT vertex_cnt, VertexT rule_cnt, int num_p) {
    auto result = gen_fusion_vertex_order(np_vlist, np_elist, vertex_cnt,
                                          rule_cnt, num_p);
    py::array_t<int> order = std::get<0>(result);
    int depth = std::get<1>(result);
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    int size = vertex_cnt + rule_cnt;
    auto mask = py::array_t<bool>(size * depth);
    mask.resize({depth, size});
    py::buffer_info mask_buf = mask.request();
    bool *mask_ptr = (bool *)mask_buf.ptr;
    VertexT s;
    int d;
    memset(mask_ptr, false, sizeof(bool) * depth * size);
    for (d = 0; d < depth; ++d) {
        for (s = 0; s < size * num_p; s++) {
            if (order_ptr[s] == d) {
                mask_ptr[d * size + s % size] = true;
            }
        }
    }
    return {mask, depth};
}

std::tuple<py::array_t<bool>, int>
fusion_mask_coo(py::array_t<VertexT> &np_vlist, py::array_t<VertexT> &np_elist,
                VertexT vertex_cnt, VertexT rule_cnt, int num_p) {
    auto result = gen_fusion_vertex_order(np_vlist, np_elist, vertex_cnt,
                                          rule_cnt, num_p);
    py::array_t<int> order = std::get<0>(result);
    int depth = std::get<1>(result);
    py::buffer_info order_buf = order.request();
    int *order_ptr = (int *)order_buf.ptr;
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_elist.request();
    // VertexT* elist = (VertexT*)elist_buf.ptr;
    VertexT edge_cnt = elist_buf.size;
    VertexT size = vertex_cnt + rule_cnt;
    auto mask = py::array_t<bool>(edge_cnt * depth);
    mask.resize({depth, int(edge_cnt)});
    py::buffer_info mask_buf = mask.request();
    bool *mask_ptr = (bool *)mask_buf.ptr;
    VertexT s, vid, eid, start, end;
    int d;
    memset(mask_ptr, false, sizeof(bool) * depth * edge_cnt);
    for (d = 0; d < depth; ++d) {
        for (s = 0; s < size * num_p; s++) {
            if (order_ptr[s] == d) {
                vid = s % size;
                start = vlist[vid];
                end = vlist[vid + 1];
                for (eid = start; eid < end; ++eid) {
                    mask_ptr[d * edge_cnt + eid] = true;
                }
            }
        }
    }
    return {mask, depth};
}
