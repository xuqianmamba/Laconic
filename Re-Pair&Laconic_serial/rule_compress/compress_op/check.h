#include <omp.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ostream>
#include <queue>
#include <sstream>
#include <stack>
#include <vector>

#include "util.h"
using namespace std;

vector<VertexT> get_ori_successors(vector<vector<VertexT>> &gp,
                                   VertexT vertexID) {
    vector<VertexT> re;
    for (VertexT i = 0; i < gp[vertexID].size(); i++) {
        re.push_back(gp[vertexID][i]);
    }
    return re;
}

void push_successors(vector<VertexT> &result, vector<vector<VertexT>> &cg,
                     stack<VertexT> &PQ, VertexT vertex_cnt) {
    while (!PQ.empty()) {
        VertexT ruleID = PQ.top();
        PQ.pop();
        for (VertexT i = 0; i < cg[ruleID].size(); i++) {
            VertexT dst = cg[ruleID][i];
            if (dst >= vertex_cnt) {
                PQ.push(dst);
            } else {
                result.push_back(dst);
            }
        }
    }
}

vector<VertexT> get_compress_successors(vector<vector<VertexT>> &cg,
                                        VertexT vertexID, VertexT vertex_cnt) {
    vector<VertexT> re;
    for (VertexT i = 0; i < cg[vertexID].size(); i++) {
        VertexT dst = cg[vertexID][i];
        if (dst >= vertex_cnt) {
            stack<VertexT> PQ;
            PQ.push(dst);
            push_successors(re, cg, PQ, vertex_cnt);
        } else {
            re.push_back(dst);
        }
    }
    return re;
}

bool compare(vector<VertexT> &a, vector<VertexT> &b) {
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    return a == b;
}

bool check_coo(std::vector<VertexT> &src, std::vector<VertexT> &dst,
               std::vector<VertexT> &compress_src,
               std::vector<VertexT> &compress_dst, VertexT vertex_cnt,
               VertexT rule_cnt) {
    std::vector<std::vector<VertexT>> graph(vertex_cnt);
    std::vector<std::vector<VertexT>> compressed_graph(vertex_cnt + rule_cnt);
    coo_convert(graph, src, dst, vertex_cnt);
    coo_convert(compressed_graph, compress_src, compress_dst,
                vertex_cnt + rule_cnt);
    VertexT count = 0;
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT v = 0; v < vertex_cnt; v++) {
        vector<VertexT> ori_successors = get_ori_successors(graph, v);
        vector<VertexT> compress_successors =
            get_compress_successors(compressed_graph, v, vertex_cnt);
        if (compare(ori_successors, compress_successors) != true) {
            // #pragma omp atomic
            // count++;
            cout << "VertexID:" << v << endl;
            cout << "ori successors:";
            for (VertexT i = 0; i < ori_successors.size(); i++) {
                cout << ori_successors[i] << " ";
            }
            cout << "\nCompress successors:";
            for (VertexT i = 0; i < compress_successors.size(); i++) {
                cout << compress_successors[i] << " ";
            }
            cout << endl;
            return false;
            break;
        }
    }
    return count == 0;
}

bool check_csr(std::vector<VertexT> &vlist, std::vector<VertexT> &elist,
               std::vector<VertexT> &compress_vlist,
               std::vector<VertexT> &compress_elist, VertexT vertex_cnt,
               VertexT rule_cnt) {
    std::vector<std::vector<VertexT>> graph(vertex_cnt);
    std::vector<std::vector<VertexT>> compressed_graph(vertex_cnt + rule_cnt);
    csr_convert(graph, vlist, elist, vertex_cnt);
    csr_convert(compressed_graph, compress_vlist, compress_elist,
                vertex_cnt + rule_cnt);
    VertexT count = 0;
    // #pragma omp parallel for schedule(dynamic)
    for (VertexT v = 0; v < vertex_cnt; v++) {
        // std::cout << v << std::endl;
        vector<VertexT> ori_successors = get_ori_successors(graph, v);
        vector<VertexT> compress_successors =
            get_compress_successors(compressed_graph, v, vertex_cnt);
        if (compare(ori_successors, compress_successors) != true) {
            // #pragma omp atomic
            // count++;
            cout << "VertexID:" << v << endl;
            cout << "ori successors:";
            for (VertexT i = 0; i < ori_successors.size(); i++) {
                cout << ori_successors[i] << " ";
            }
            cout << "\nCompress successors:";
            for (VertexT i = 0; i < compress_successors.size(); i++) {
                cout << compress_successors[i] << " ";
            }
            cout << endl;
            return false;
        }
    }
    return count == 0;
}