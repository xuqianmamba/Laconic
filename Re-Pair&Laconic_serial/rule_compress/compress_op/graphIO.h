#pragma once
#include <iostream>
#include <vector>
// #include <omp.h>
#include "assert.h"
#include "util.h"
#include <fstream>


std::tuple<py::array_t<int>, py::array_t<int>> read_from_file(std::string file_path)
{
    ifstream infile(file_path);
    if(!infile.good()){
        std::cout << "file not exist" << std::endl;
    }
    int vertex_cnt;
    int edge_cnt;
    infile >> vertex_cnt;
    infile >> edge_cnt;
    std::vector<int> back_vlist;
    std::vector<int> back_elist;
    back_vlist.push_back(0);
    int esize = 0;
    int tmp;
    while(infile >> tmp){
        if(tmp >= vertex_cnt){
            back_vlist.push_back(esize);
        }else{
            back_elist.push_back(tmp);
            esize++;
        }
    }
    while(back_vlist.size() < vertex_cnt){
        back_vlist.push_back(esize);
    }
    auto new_vlist = vector2numpy1D(back_vlist);
    auto new_elist = vector2numpy1D(back_elist);
    return {new_vlist, new_elist};
}


bool output(std::string file_path, py::array_t<int> np_input_vlist, py::array_t<int> np_input_elist, int vertex_cnt, int rule_cnt)
{
    ofstream outfile(file_path);
    if(!outfile.good()){
        std::cout << "file not exist" << std::endl;
        return false;
    }
    std::vector<int> vlist;
    std::vector<int> elist;
    numpy2vector1D(np_input_vlist, vlist);
    numpy2vector1D(np_input_elist, elist);
    outfile << vertex_cnt << " " << rule_cnt << std::endl;
    for(int vid = 0; vid < vlist.size()-1; vid++){
        int start = vlist[vid];
        int end = vlist[vid+1];
        for(int eid = start; eid < end; eid++){
            outfile << elist[eid] << " ";
        }
        outfile << endl;
    }
    return true;
}