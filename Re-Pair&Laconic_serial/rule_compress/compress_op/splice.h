#pragma once
#include "assert.h"
#include "util.h"
#include <cstdio>
#include <iostream>
#include <math.h>
#include <omp.h>
#include <vector>
#include <unordered_map>

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>, py::array_t<float>>
splice_csr(py::array_t<VertexT> np_vlist, py::array_t<VertexT> np_elist,
           py::array_t<float> &np_weight, py::array_t<bool> &np_index) {
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info weight_buf = np_weight.request();
    float *weight_ptr = (float *)weight_buf.ptr;
    py::buffer_info index_buf = np_index.request();
    bool *index = (bool *)index_buf.ptr;
    std::vector<VertexT> back_vlist;
    std::vector<VertexT> back_elist;
    std::vector<float> back_weight;
    back_vlist.emplace_back(0);
    VertexT vid, eid, start, end, dst;
    VertexT esize = 0;
    for (vid = 0; vid < vertex_cnt; ++vid) {
        if (index[vid] == false) {
            back_vlist.emplace_back(esize);
            continue;
        } else {
            start = vlist_ptr[vid];
            end = vlist_ptr[vid + 1];
            for (eid = start; eid < end; ++eid) {
                dst = elist_ptr[eid];
                back_elist.emplace_back(dst);
                back_weight.emplace_back(weight_ptr[eid]);
                esize++;
            }
            back_vlist.emplace_back(esize);
        }
    }
    auto new_vlist = vector2numpy1D(back_vlist);
    auto new_elist = vector2numpy1D(back_elist);
    auto new_weight = vector2numpy1D(back_weight);
    return {new_vlist, new_elist, new_weight};
}


std::tuple<py::array_t<VertexT>, py::array_t<VertexT>, py::array_t<float>, py::array_t<VertexT>>
splice_csr_condense_row(py::array_t<VertexT> np_vlist, py::array_t<VertexT> np_elist,
           py::array_t<float> &np_weight, py::array_t<bool> &np_index) {
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info weight_buf = np_weight.request();
    float *weight_ptr = (float *)weight_buf.ptr;
    py::buffer_info index_buf = np_index.request();
    bool *index = (bool *)index_buf.ptr;
    std::vector<VertexT> back_vlist;
    std::vector<VertexT> back_elist;
    std::vector<float> back_weight;
    std::vector<VertexT> active_vertex;
    back_vlist.emplace_back(0);
    VertexT vid, eid, start, end, dst;
    VertexT esize = 0;
    for (vid = 0; vid < vertex_cnt; ++vid) {
        if(index[vid] == true){
            active_vertex.emplace_back(vid);
            start = vlist_ptr[vid];
            end = vlist_ptr[vid+1];
            for(eid = start; eid < end; ++eid){
                dst = elist_ptr[eid];
                back_elist.emplace_back(dst);
                back_weight.emplace_back(weight_ptr[eid]);
                esize++;
            }
            back_vlist.emplace_back(esize);
        }
    }
    auto new_vlist = vector2numpy1D(back_vlist);
    auto new_elist = vector2numpy1D(back_elist);
    auto new_weight = vector2numpy1D(back_weight);
    auto active_row = vector2numpy1D(active_vertex);
    return {new_vlist, new_elist, new_weight, active_row};
}

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>, py::array_t<float>, py::array_t<VertexT>>
splice_csr_condense_col(py::array_t<VertexT> np_vlist, py::array_t<VertexT> np_elist,
           py::array_t<float> &np_weight, py::array_t<bool> &np_index) {
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info weight_buf = np_weight.request();
    float *weight_ptr = (float *)weight_buf.ptr;
    py::buffer_info index_buf = np_index.request();
    bool *index = (bool *)index_buf.ptr;
    std::vector<VertexT> back_vlist;
    std::vector<VertexT> back_elist;
    std::vector<float> back_weight;
    std::unordered_map<VertexT, VertexT> col_map;
    std::vector<VertexT> active_vertex;
    back_vlist.emplace_back(0);
    VertexT vid, eid, start, end, dst;
    VertexT esize = 0;
    VertexT maxID = 0;
    for (vid = 0; vid < vertex_cnt; ++vid) {
        if(index[vid] == true){
            start = vlist_ptr[vid];
            end = vlist_ptr[vid+1];
            for(eid = start; eid < end; ++eid){
                dst = elist_ptr[eid];
                if(col_map.find(dst) == col_map.end()){
                    col_map.insert(std::make_pair(dst, maxID++));
                    active_vertex.emplace_back(dst);
                }
                back_elist.emplace_back(col_map[dst]);
                back_weight.emplace_back(weight_ptr[eid]);
                esize++;
            }
            back_vlist.emplace_back(esize);
        }else{
            back_vlist.emplace_back(esize);
            continue;
        }
    }
    
    auto new_vlist = vector2numpy1D(back_vlist);
    auto new_elist = vector2numpy1D(back_elist);
    auto new_weight = vector2numpy1D(back_weight);
    auto active_col = vector2numpy1D(active_vertex);
    return {new_vlist, new_elist, new_weight, active_col};
}

std::tuple<py::array_t<VertexT>, py::array_t<VertexT>, py::array_t<float>, py::array_t<VertexT>, py::array_t<VertexT>>
splice_csr_condense_row_and_col(py::array_t<VertexT> np_vlist, py::array_t<VertexT> np_elist,
           py::array_t<float> &np_weight, py::array_t<bool> &np_index) {
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info weight_buf = np_weight.request();
    float *weight_ptr = (float *)weight_buf.ptr;
    py::buffer_info index_buf = np_index.request();
    bool *index = (bool *)index_buf.ptr;
    std::vector<VertexT> back_vlist;
    std::vector<VertexT> back_elist;
    std::vector<float> back_weight;
    std::unordered_map<VertexT, VertexT> col_map;
    std::vector<VertexT> active_vertex;
    std::vector<VertexT> active_neighbor;
    VertexT maxID = 0;
    back_vlist.emplace_back(0);
    VertexT vid, eid, start, end, dst;
    VertexT esize = 0;
    for (vid = 0; vid < vertex_cnt; ++vid) {
        if(index[vid] == true){
            active_vertex.emplace_back(vid);
            start = vlist_ptr[vid];
            end = vlist_ptr[vid + 1];
            for (eid = start; eid < end; ++eid) {
                dst = elist_ptr[eid];
                if(col_map.find(dst) == col_map.end()){
                    col_map.insert(std::make_pair(dst, maxID++));
                    active_neighbor.emplace_back(dst);
                }
                back_elist.emplace_back(col_map[dst]);
                back_weight.emplace_back(weight_ptr[eid]);
                esize++;
            }
            back_vlist.emplace_back(esize);
        }
    }
    auto new_vlist = vector2numpy1D(back_vlist);
    auto new_elist = vector2numpy1D(back_elist);
    auto new_weight = vector2numpy1D(back_weight);
    auto active_row = vector2numpy1D(active_vertex);
    auto active_col = vector2numpy1D(active_neighbor);
    return {new_vlist, new_elist, new_weight, active_row, active_col};
}