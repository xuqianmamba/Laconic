#pragma once
#include <iostream>
#include <vector>
// #include <omp.h>
#include "assert.h"
#include "util.h"


std::tuple<py::array_t<int>, py::array_t<int>, int, int> merge_(std::vector<py::array_t<int>> vlist_map,
                                                    std::vector<py::array_t<int>> elist_map,
                                                    std::vector<int> vertex_map,
                                                    std::vector<int> rule_map
                                                    ) {
    int part_num = vlist_map.size();
    std::cout << part_num << std::endl;
    int vertex_cnt = 0;
    int rule_cnt = 0;
    for(int i = 0; i < part_num; ++i){
        vertex_cnt += vertex_map[i];
        rule_cnt += rule_map[i];
    }
    std::cout << vertex_cnt << std::endl;
    std::cout << rule_cnt << std::endl;
    std::vector<std::vector<int>> graph(vertex_cnt+rule_cnt);
    int pos = 0, vertex_offset = 0, rule_offset = 0;
    for(int i; i < part_num; ++i){
        py::buffer_info vlist_buf = vlist_map[i].request();
        int vsize = vlist_buf.size;
        int *vlist = (int *)vlist_buf.ptr;
        py::buffer_info elist_buf = elist_map[i].request();
        int esize = elist_buf.size;
        int *elist = (int *)elist_buf.ptr;
        for(int vid = 0; vid < vertex_map[i]; vid++){ // 顶点
            int start = vlist[vid];
            int end = vlist[vid+1];
            for(int j = start; j < end; j++){
                int dst = elist[j];
                if(dst >= vertex_cnt){
                    if(dst+rule_offset > vertex_cnt+rule_cnt) std::cout << dst+rule_offset << std::endl;
                    graph[vid+vertex_offset].push_back(dst+rule_offset);
                }else{
                    graph[vid+vertex_offset].push_back(dst);
                }
            }
        }
        for(int rid = 0; rid < rule_map[i]; ++rid){
            int start = vlist[rid+vertex_map[i]];
            int end = vlist[rid+1+vertex_map[i]];
            for(int j = start; j < end; j++){
                int dst = elist[j];
                if(dst >= vertex_cnt){
                    if(dst+rule_offset > vertex_cnt+rule_cnt) std::cout << dst+rule_offset << std::endl;
                    graph[vertex_cnt+rid+rule_offset].push_back(dst+rule_offset);
                }else{
                    graph[vertex_cnt+rid+rule_offset].push_back(dst);
                }
            }
        }
        vertex_offset += vertex_map[i];
        rule_offset += rule_map[i];
    }
    std::vector<int> re_vlist;
    std::vector<int> re_elist;
    int esize = 0;
    re_vlist.emplace_back(0);
    for(int i = 0; i < vertex_cnt+rule_cnt; ++i){
        int vsize = graph[i].size();
        for(int j = 0; j < vsize; j++){
            re_elist.push_back(graph[i][j]);
        }
        esize += vsize;
        re_vlist.push_back(esize);
    }
    auto new_vlist = vector2numpy1D(re_vlist);
    auto new_elist = vector2numpy1D(re_elist);
    return {new_vlist, new_elist, vertex_cnt, rule_cnt};
}