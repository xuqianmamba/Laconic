#pragma once
#include "assert.h"
#include "util.h"
#include <cstdio>
#include <iostream>
#include <omp.h>
#include <vector>

/*
 * add self loop for csr format graph
 * @param np_input_vlist: input vlist
 * @param np_input_elist: input elist
 * Return:
 * @return_vlist
 * @return_elist
 */
std::tuple<py::array_t<VertexT>, py::array_t<VertexT>>
add_self_loop_csr(py::array_t<VertexT> &np_input_vlist,
                  py::array_t<VertexT> &np_input_elist) {
    std::vector<VertexT> vlist;
    std::vector<VertexT> elist;
    py::buffer_info vlist_buf = np_input_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    size_t vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_input_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    VertexT vid, eid, start, end, dst;
    vlist.emplace_back(0);
    for (vid = 0; vid < vertex_cnt; ++vid) {
        start = vlist_ptr[vid];
        end = vlist_ptr[vid + 1];
        for (eid = start; eid < end && elist_ptr[eid] < vid; ++eid)
            elist.emplace_back(elist_ptr[eid]);
        if (elist_ptr[eid] != vid || eid == end)
            elist.emplace_back(vid);
        for (; eid < end; ++eid)
            elist.emplace_back(elist_ptr[eid]);
        vlist.emplace_back(elist.size());
    }
    auto return_vlist = vector2numpy1D<VertexT>(vlist);
    auto return_elist = vector2numpy1D<VertexT>(elist);
    return {return_vlist, return_elist};
}

/*
 * add self loop for csr format graph
 * @param np_input_src: input vlist
 * @param np_input_dst: input elist
 * @param vertex_cnt: the total number of vertices
 * Return:
 * @return_src
 * @return_dst
 */
std::tuple<py::array_t<VertexT>, py::array_t<VertexT>>
add_self_loop_coo(const py::array_t<VertexT> &np_input_src,
                  const py::array_t<VertexT> &np_input_dst, VertexT vertex_cnt) {
    std::vector<VertexT> src;
    std::vector<VertexT> dst;
    py::buffer_info src_buf = np_input_src.request();
    VertexT *src_ptr = (VertexT *)src_buf.ptr;
    size_t edge_cnt = src_buf.size;
    std::cout << edge_cnt << std::endl;
    py::buffer_info dst_buf = np_input_dst.request();
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;
    VertexT vid, eid, src_id, dst_id;
    vid = 0;
    eid = 0;
    for (; eid < edge_cnt;) {
        src_id = src_ptr[eid];
        dst_id = dst_ptr[eid];
        while (src_id == vid && dst_id < vid) {
            src.emplace_back(src_id);
            dst.emplace_back(dst_id);
            eid++;
            src_id = src_ptr[eid];
            dst_id = dst_ptr[eid];
        }
        if (src_id == vid && dst_id == vid) {
            src.emplace_back(src_id);
            dst.emplace_back(dst_id);
            eid++;
            src_id = src_ptr[eid];
            dst_id = dst_ptr[eid];
        } else {
            src.emplace_back(vid);
            dst.emplace_back(vid);
        }
        while (src_id == vid) {
            src.emplace_back(src_id);
            dst.emplace_back(dst_id);
            eid++;
            src_id = src_ptr[eid];
            dst_id = dst_ptr[eid];
        }
        vid++;
        if (vid == vertex_cnt)
            break;
    }
    while (vid < vertex_cnt) {
        src.emplace_back(vid);
        dst.emplace_back(vid);
        vid++;
    }
    auto return_src = vector2numpy1D<VertexT>(src);
    auto return_dst = vector2numpy1D<VertexT>(dst);
    return {return_src, return_dst};
}
