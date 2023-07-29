#pragma once
#include "assert.h"
#include "util.h"
#include <cstdio>
#include <iostream>
#include <math.h>
#include <omp.h>
#include <vector>
#include <unordered_map>

std::tuple<VertexT, VertexT>
get_active_row_and_col(py::array_t<VertexT> np_vlist, py::array_t<VertexT> np_elist,
                    py::array_t<bool> &np_index){
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    VertexT vertex_cnt = vlist_buf.size - 1;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info index_buf = np_index.request();
    bool *index = (bool *)index_buf.ptr;
    VertexT vid, eid, start, end, dst;
    VertexT esize = 0;
    VertexT active_row = 0;
    VertexT active_col = 0;
    std::unordered_set<VertexT> col_set;
    for(vid = 0; vid < vertex_cnt; ++vid){
        if(index[vid] == true){
            active_row++;
            start = vlist_ptr[vid];
            end = vlist_ptr[vid+1];
            for(eid = start; eid < end; eid++){
                dst = elist_ptr[eid];
                if(col_set.find(dst) == col_set.end()){
                    col_set.insert(dst);
                    active_col++;
                }
            }
        }
    }
    return {active_row, active_col};
}

VertexT get_out_degree(py::array_t<VertexT> &np_vlist, py::array_t<VertexT> &np_elist,
                        py::array_t<VertexT> row_map){
    py::buffer_info vlist_buf = np_vlist.request();
    VertexT *vlist_ptr = (VertexT *)vlist_buf.ptr;
    py::buffer_info elist_buf = np_elist.request();
    VertexT *elist_ptr = (VertexT *)elist_buf.ptr;
    py::buffer_info row_buf = row_map.request();
    VertexT row_num = row_buf.size;
    VertexT *row_ptr = (VertexT *)row_buf.ptr;
    VertexT degree = 0;
    #pragma omp parallel for schedule(dynamic, 64) reduction(+:degree)
    for(VertexT row = 0; row < row_num; row++){
        VertexT vid = row_ptr[row];
        VertexT start = vlist_ptr[vid];
        VertexT end = vlist_ptr[vid+1];
        VertexT local_degree = end - start;
        degree += local_degree;
    }
    return degree;
}

