#include "util.h"

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>>
coo2csr(py::array_t<VertexT> &np_src, py::array_t<VertexT> &np_dst,
        VertexT vertex_cnt) {
    py::buffer_info src_buf = np_src.request();
    py::buffer_info dst_buf = np_dst.request();
    VertexT *src_ptr = (VertexT *)src_buf.ptr;
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;
    size_t edge_cnt = src_buf.size;
    auto vlist = py::array_t<VertexT>(vertex_cnt + 1);
    auto elist = py::array_t<VertexT>(edge_cnt);
    py::buffer_info vlist_buf = vlist.request();
    py::buffer_info elist_buf = elist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    VertexT v, e, cur_v;
    VertexT e_size = 0;
    cur_v = 0;
    vlist_ptr[cur_v] = 0;
    for (e = 0; e < edge_cnt; e++) {
        v = src_ptr[e];
        while (cur_v != v) {
            vlist_ptr[++cur_v] = e_size;
        }
        elist_ptr[e] = dst_ptr[e];
        e_size++;
    }
    while (cur_v != vertex_cnt) {
        vlist_ptr[++cur_v] = e_size;
    }
    return {vlist, elist};
}

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>>
csr2coo(py::array_t<VertexT> &vlist, py::array_t<VertexT> &elist) {
    py::buffer_info vlist_buf = vlist.request();
    py::buffer_info elist_buf = elist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    size_t vertex_cnt = vlist_buf.size - 1;
    size_t edge_cnt = elist_buf.size;

    auto src = py::array_t<VertexT>(edge_cnt);
    auto dst = py::array_t<VertexT>(edge_cnt);
    py::buffer_info src_buf = src.request();
    py::buffer_info dst_buf = dst.request();
    VertexT *src_ptr = (VertexT *)src_buf.ptr;
    VertexT *dst_ptr = (VertexT *)dst_buf.ptr;

    VertexT v, e;
    VertexT start, end;
    for (v = 0; v < vertex_cnt; v++) {
        start = vlist_ptr[v];
        end = vlist_ptr[v + 1];
        for (e = start; e < end; e++) {
            src_ptr[e] = v;
            dst_ptr[e] = elist_ptr[e];
        }
    }
    return {src, dst};
}