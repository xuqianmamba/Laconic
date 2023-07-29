#pragma once
#include <bits/stdc++.h>
#include <omp.h>

#include <iostream>
#include <vector>

std::vector<uint32_t> decompress(uint32_t max_id, std::vector<uint32_t> compressed_vec,
                            const std::vector<uint32_t> &compress_v,
                            const std::vector<uint32_t> &compress_e) {
  std::vector<uint32_t> temp_vec = {};
  for (uint32_t i = 0; i < compressed_vec.size(); i++) {
    if (compressed_vec[i] > max_id) {
      std::vector<uint32_t> new_vec(compress_e.begin() + (compressed_vec[i]),
                               compress_e.begin() + (compressed_vec[i + 1]));
      std::vector<uint32_t> new_role = decompress(
          max_id, new_vec, std::ref(compress_v), std::ref(compress_e));
      temp_vec.insert(temp_vec.end(), new_role.begin(), new_role.end());
    } else
      temp_vec.push_back(compressed_vec[i]);
  }
  return temp_vec;
}
void check(std::vector<uint32_t> origin_v, std::vector<uint32_t> origin_e,
           std::vector<uint32_t> compress_v, std::vector<uint32_t> compress_e,
           uint32_t max_id) {
  uint32_t flag = 0;
  for (uint32_t j = 0; j < origin_v.size() - 2; j++) {
    std::vector<uint32_t> temp = std::vector<uint32_t>(
        origin_e.begin() + origin_v[j], origin_e.begin() + origin_v[j + 1]);
    std::vector<uint32_t> compress_temp =
        std::vector<uint32_t>(compress_e.begin() + compress_v[j],
                         compress_e.begin() + compress_v[j + 1]);
    compress_temp = decompress(max_id, compress_temp, std::ref(compress_v),
                               std::ref(compress_e));
    if (temp.size() != compress_temp.size()) {
      std::cout << j << "  differs!  origin:";
      for (uint32_t i = 0; i < temp.size(); i++) {
        std::cout << temp[i] << " ";
      }
      std::cout << std::endl;
      std::cout << "compressed:";
      for (uint32_t i = 0; i < compress_temp.size(); i++) {
        std::cout << compress_temp[i] << " ";
      }
      break;
    } else {
      for (uint32_t k = 0; k < temp.size(); k++) {
        if (temp[k] != compress_temp[k]) {
          std::cout << j << "  differs! origin:";
          for (uint32_t i = 0; i < temp.size(); i++) {
            std::cout << temp[i] << " ";
          }
          std::cout << std::endl;
          std::cout << "compressed:";
          for (uint32_t i = 0; i < compress_temp.size(); i++) {
            std::cout << compress_temp[i] << " ";
          }
          flag = 1;
        }
      }
      if (flag) break;
    }
  }
  return;
}

#include <omp.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ostream>
#include <queue>
#include <sstream>
#include <stack>
#include <vector>

using namespace std;

vector<uint32_t> get_ori_successors(vector<vector<uint32_t>> &gp, uint32_t vertexID) {
  vector<uint32_t> re;
  for (uint32_t i = 0; i < gp[vertexID].size(); i++) {
    re.push_back(gp[vertexID][i]);
  }
  return re;
}

void push_successors(vector<uint32_t> &result, vector<vector<uint32_t>> &cg,
                     stack<uint32_t> &PQ, uint32_t vertex_cnt) {
  while (!PQ.empty()) {
    uint32_t ruleID = PQ.top();
    PQ.pop();
    for (uint32_t i = 0; i < cg[ruleID].size(); i++) {
      uint32_t dst = cg[ruleID][i];
      if (dst >= vertex_cnt) {
        PQ.push(dst);
      } else {
        result.push_back(dst);
      }
    }
  }
}

vector<uint32_t> get_compress_successors(vector<vector<uint32_t>> &cg, uint32_t vertexID,
                                    uint32_t vertex_cnt) {
  vector<uint32_t> re;
  for (uint32_t i = 0; i < cg[vertexID].size(); i++) {
    uint32_t dst = cg[vertexID][i];
    if (dst >= vertex_cnt) {
      stack<uint32_t> PQ;
      PQ.push(dst);
      push_successors(re, cg, PQ, vertex_cnt);
    } else {
      re.push_back(dst);
    }
  }
  return re;
}

bool compare(vector<uint32_t> &a, vector<uint32_t> &b) {
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  return a == b;
}

void csr_convert(std::vector<std::vector<uint32_t>> &graph, std::vector<uint32_t> &vlist,
                 std::vector<uint32_t> &elist, uint32_t vertex_cnt) {
  // #pragma omp parallel for schedule(dynamic)
  for (uint32_t v = 0; v < vertex_cnt; v++) {
    uint32_t start = vlist[v];
    uint32_t end = vlist[v + 1];
    for (uint32_t e = start; e < end; e++) {
      graph[v].push_back(elist[e]);
    }
  }
}

bool check_csr(std::vector<uint32_t> &vlist, std::vector<uint32_t> &elist,
               std::vector<uint32_t> &compress_vlist,
               std::vector<uint32_t> &compress_elist, uint32_t vertex_cnt, uint32_t rule_cnt) {
  std::vector<std::vector<uint32_t>> graph(vertex_cnt);
  std::vector<std::vector<uint32_t>> compressed_graph(vertex_cnt + rule_cnt);
  csr_convert(graph, vlist, elist, vertex_cnt);
  csr_convert(compressed_graph, compress_vlist, compress_elist,
              vertex_cnt + rule_cnt);
  uint32_t count = 0;
  // #pragma omp parallel for schedule(dynamic)
  for (uint32_t v = 0; v < vertex_cnt; v++) {
    if (v % 10000 == 0) fprintf(stderr, "goes to %d ", v);
    vector<uint32_t> ori_successors = get_ori_successors(graph, v);
    vector<uint32_t> compress_successors =
        get_compress_successors(compressed_graph, v, vertex_cnt);
    if (compare(ori_successors, compress_successors) != true) {
      // #pragma omp atomic
      // count++;
      fprintf(stderr, " \n\nVertexID: %d \n", v);
      fprintf(stderr, "  ori successors: \n");
      for (uint32_t i = 0; i < ori_successors.size(); i++) {
        fprintf(stderr, "%d ", ori_successors[i]);
      }
      fprintf(stderr, "\n\n Compress successors: \n");
      for (uint32_t i = 0; i < compress_successors.size(); i++) {
        fprintf(stderr, "%d ", compress_successors[i]);
      }
      return false;
      count++;
    }
  }
  fprintf(stderr, " wrong vertex num: %d \n", count);
  return count == 0;
}