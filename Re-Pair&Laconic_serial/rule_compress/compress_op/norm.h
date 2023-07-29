#pragma once
#include "assert.h"
#include "util.h"
#include <cstdio>
#include <iostream>
#include <math.h>
#include <omp.h>
#include <vector>

/*
 * get in degree for each vertex
 * in_degree = 1 / sqrt(in_degree)
 * Input:
 * @param dst: dst  for coo format or elist for csr format
 * @param vertex_cnt: the total number of vertices
 * Output:
 * @return_in_degree for each vertex
 */

py::array_t<float> get_norm_degree(py::array_t<VertexT> &np_dst,
                                   VertexT vertex_cnt) {
    auto in_degree = py::array_t<float>(vertex_cnt);
    py::buffer_info in_degree_buf = in_degree.request();
    float *in_degree_ptr = (float *)in_degree_buf.ptr;

    py::buffer_info dst_buf = np_dst.request();
    size_t edge_cnt = dst_buf.size;
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;

    VertexT vid, eid;

    for (vid = 0; vid < vertex_cnt; ++vid) {
        in_degree_ptr[vid] = INF;
    }

    for (eid = 0; eid < edge_cnt; ++eid) {
        in_degree_ptr[dst_ptr[eid]] += 1;
    }

    for (vid = 0; vid < vertex_cnt; ++vid) {
        in_degree_ptr[vid] = 1 / sqrt(in_degree_ptr[vid]);
    }

    return in_degree;
}

/*
 * gcn_norm for coo format
 * Input:
 * @param np_input_src
 * @param np_input_dst
 * @param in_degree_weight
 * @param vertex_cnt : the total number of vertices
 * Output:
 * @return_edge_weight : the egde_weight for each edge
 */
py::array_t<float> gcn_norm_coo(py::array_t<VertexT> &np_input_src,
                                py::array_t<VertexT> &np_input_dst,
                                py::array_t<float> &in_degree_weight,
                                VertexT vertex_cnt) {
    py::buffer_info src_buf = np_input_src.request();
    size_t edge_cnt = src_buf.size;
    VertexT *src_ptr = (VertexT *)src_buf.ptr;
    py::buffer_info dst_buf = np_input_dst.request();
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;
    py::buffer_info in_degree_buf = in_degree_weight.request();
    float *in_degree_ptr = (float *)in_degree_buf.ptr;

    auto edge_weight = py::array_t<float>(edge_cnt);
    py::buffer_info edge_weight_buf = edge_weight.request();
    float *edge_weight_ptr = (float *)edge_weight_buf.ptr;
    for (int i = 0; i < edge_cnt; ++i) {
        edge_weight_ptr[i] =
            in_degree_ptr[src_ptr[i]] * in_degree_ptr[dst_ptr[i]];
    }

    return edge_weight;
}

/*
 * gcn_norm for csr format
 * Input:
 * @param np_input_vlist
 * @param np_input_elist
 * @param in_degree_weight
 * @param vertex_cnt : the total number of vertices
 * Output:
 * @return_edge_weight : the egde_weight for each edge
 */
py::array_t<float> gcn_norm_csr(py::array_t<VertexT> &np_input_vlist,
                                py::array_t<VertexT> &np_input_elist,
                                py::array_t<float> &in_degree_weight,
                                VertexT vertex_cnt) {
    py::buffer_info vlist_buf = np_input_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_input_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    size_t edge_cnt = elist_buf.size;
    py::buffer_info in_degree_buf = in_degree_weight.request();
    float *in_degree_ptr = (float *)in_degree_buf.ptr;

    auto edge_weight = py::array_t<float>(edge_cnt);
    py::buffer_info edge_weight_buf = edge_weight.request();
    float *edge_weight_ptr = (float *)edge_weight_buf.ptr;
    VertexT vid, eid, start, end, dst;
#pragma omp parallel for
    for (vid = 0; vid < vertex_cnt; ++vid) {
        start = vlist_ptr[vid];
        end = vlist_ptr[vid + 1];
        for (eid = start; eid < end; ++eid) {
            dst = elist_ptr[eid];
            edge_weight_ptr[eid] = in_degree_ptr[vid] * in_degree_ptr[dst];
        }
    }
    return edge_weight;
}

/*
 * GCN normalization for compressed graph with COO format
 * Input:
 * @param np_input_src
 * @param np_input_dst
 * @param in_degree_weight
 * @param vertex_cnt: the total number of vertices
 * @param rule_cnt: the total number of rules
 * Output
 * @return edge_weight
 */
py::array_t<float> gcn_norm_coo_compress(py::array_t<VertexT> &np_input_src,
                                         py::array_t<VertexT> &np_input_dst,
                                         py::array_t<float> &in_degree_weight,
                                         VertexT vertex_cnt, VertexT rule_cnt) {
    double start = timestamp();
    py::buffer_info src_buf = np_input_src.request();
    size_t edge_cnt = src_buf.size;
    VertexT *src_ptr = (VertexT *)src_buf.ptr;
    py::buffer_info dst_buf = np_input_dst.request();
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;
    py::buffer_info in_degree_buf = in_degree_weight.request();
    float *in_degree_ptr = (float *)in_degree_buf.ptr;

    auto edge_weight = py::array_t<float>(edge_cnt);
    py::buffer_info edge_weight_buf = edge_weight.request();
    float *edge_weight_ptr = (float *)edge_weight_buf.ptr;

    VertexT eid, src_node, dst_node;
#pragma omp parallel for
    for (eid = 0; eid < edge_cnt; ++eid) {
        src_node = src_ptr[eid];
        dst_node = dst_ptr[eid];
        if (src_node < vertex_cnt && dst_node < vertex_cnt)
            edge_weight_ptr[eid] =
                in_degree_ptr[src_node] * in_degree_ptr[dst_node];
        if (src_node < vertex_cnt && dst_node >= vertex_cnt)
            edge_weight_ptr[eid] = in_degree_ptr[src_node];
        if (src_node >= vertex_cnt && dst_node < vertex_cnt)
            edge_weight_ptr[eid] = in_degree_ptr[dst_node];
        if (src_node >= vertex_cnt && dst_node >= vertex_cnt)
            edge_weight_ptr[eid] = 1;
    }
    double end = timestamp();
#ifdef TIME
    std::cout << "Normalize time:" << end - start << std::endl;
#endif
    return edge_weight;
}

/*
 * GCN normalization for compressed graph with CSR format
 * Input:
 * @param np_input_vlist
 * @param np_input_elist
 * @param in_degree_weight
 * @vertex_cnt: the total number of vertices
 * @rule_cnt: the total number of rules
 * Output
 * @return edge_weight
 */
py::array_t<float> gcn_norm_csr_compress(py::array_t<VertexT> &np_input_vlist,
                                         py::array_t<VertexT> &np_input_elist,
                                         py::array_t<float> &in_degree_weight,
                                         VertexT vertex_cnt, VertexT rule_cnt) {
    double start_time = timestamp();
    py::buffer_info vlist_buf = np_input_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_input_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    size_t edge_cnt = elist_buf.size;
    py::buffer_info degree_buf = in_degree_weight.request();
    float *in_degree_ptr = (float *)degree_buf.ptr;
    auto edge_weight = py::array_t<float>(edge_cnt);
    py::buffer_info edge_weight_buf = edge_weight.request();
    float *edge_weight_ptr = (float *)edge_weight_buf.ptr;
    VertexT vid, eid, dst, start, end;
#pragma omp parallel for
    for (vid = 0; vid < vertex_cnt + rule_cnt; ++vid) {
        start = vlist_ptr[vid];
        end = vlist_ptr[vid + 1];
        for (eid = start; eid < end; ++eid) {
            dst = elist_ptr[eid];
            if (vid < vertex_cnt && dst < vertex_cnt)
                edge_weight_ptr[eid] = in_degree_ptr[vid] * in_degree_ptr[dst];
            if (vid < vertex_cnt && dst >= vertex_cnt)
                edge_weight_ptr[eid] = in_degree_ptr[vid];
            if (vid >= vertex_cnt && dst < vertex_cnt)
                edge_weight_ptr[eid] = in_degree_ptr[dst];
            if (vid >= vertex_cnt && dst >= vertex_cnt)
                edge_weight_ptr[eid] = 1;
        }
    }
    double end_time = timestamp();
#ifdef TIME
    std::cout << "Normalize time:" << end_time - start_time << std::endl;
#endif
    return edge_weight;
}
