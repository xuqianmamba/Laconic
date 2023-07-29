#pragma once
#include <omp.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>

#include "common.h"

typedef struct rule_info {  // rule infomation
  int len;
  int freq;
  rule_info() {
    len = 0;
    freq = 0;
  }
} rule_info;

int threshold = 0;

inline bool judgeRule(int len, int freq) {
  return (freq - 1) * (len - 1) - 1 <= threshold;
}
// graphT 规则出现的地点
void mergeRule(DTYPE rule_id, std::vector<std::vector<DTYPE>> &graph,
               std::vector<std::vector<DTYPE>> &graphT,
               std::vector<rule_info> &rif, std::vector<bool> &merge_flag,
               DTYPE vertex_cnt, DTYPE rule_cnt) {
            
  DTYPE ID = rule_id + vertex_cnt;
  // if(rule_id==797726)   cout<<"in merge rule"<<endl;
  // if(rule_id==797726) cout<<"ID is "<<ID<<endl;

  // insert childnode to parent node,replace rule node with its neighbors
  if(rule_id==797726) cout<<"graphT[ID].size() is "<<graphT[ID].size()<<endl;
  for (DTYPE i = 0; i < graphT[ID].size(); ++i) {
    // if(rule_id==797726)   cout<<"in for (DTYPE i = 0; i < graphT[ID].size(); ++i)"<<endl;
    DTYPE node = graphT[ID][i];
    std::vector<DTYPE>::iterator it =
        find(graph[node].begin(), graph[node].end(), ID);
    std::vector<DTYPE>::iterator pos = graph[node].erase(it);
    graph[node].insert(pos, graph[ID].begin(), graph[ID].end());
    // update len of parent node if parent node is rule
    if (node >= vertex_cnt) {
      rif[node - vertex_cnt].len += (graph[ID].size() - 1);
      merge_flag[node - vertex_cnt] =
          judgeRule(rif[node - vertex_cnt].len, rif[node - vertex_cnt].freq);
    }
  }
  
  // update child node idx
  for (DTYPE i = 0; i < graph[ID].size(); ++i) {
    DTYPE child_id = graph[ID][i];
  std::vector<DTYPE>::iterator it =
        find(graphT[child_id].begin(), graphT[child_id].end(), ID);
    // delete origin idx in ruleID
    std::vector<DTYPE>::iterator pos = graphT[child_id].erase(it);
    // insert new idx to child id
    graphT[child_id].insert(pos, graphT[ID].begin(), graphT[ID].end());
    // update freq of child node if the child node is rule
    if (child_id >= vertex_cnt) {
      rif[child_id - vertex_cnt].freq += graphT[child_id].size();
      merge_flag[child_id - vertex_cnt] = judgeRule(
          rif[child_id - vertex_cnt].len, rif[child_id - vertex_cnt].freq);
    }
  }
}

void initInfoForRule(std::vector<std::vector<DTYPE>> &graph,
                     std::vector<std::vector<DTYPE>> &graphT,
                     std::vector<rule_info> &rif, std::vector<bool> &merge_flag,
                     DTYPE vertex_cnt,
                     DTYPE rule_cnt) {  // init len&freq for each rule
  for (DTYPE i = 0; i < rule_cnt; ++i) {
    rif[i].freq = graphT[i + vertex_cnt].size();
    rif[i].len = graph[i + vertex_cnt].size();
    // if(i==797726) cout<<rif[i].len<<"  ***  "<<rif[i].freq<<endl;
    merge_flag[i] = judgeRule(rif[i].len, rif[i].freq);
  }
  // printf("\n");
  // cout<<"merge_flag[797726] is "<<  merge_flag[797726]<<endl;
  // printf("\n");
}

/*
 *  convert csr to double vector out-edge
 */
void csr_convert_filter(std::vector<std::vector<DTYPE>> &graph,
                        std::vector<DTYPE> &vlist, std::vector<DTYPE> &elist,
                        DTYPE vertex_cnt) {
  // #pragma omp parallel for schedule(dynamic)
  for (DTYPE v = 0; v < vertex_cnt; v++) {
    DTYPE start = vlist[v];
    DTYPE end = vlist[v + 1];
    for (DTYPE e = start; e < end; e++) {
      // if(elist[e]==1485) fprintf(stderr, "\n ****  %d ",v);
      graph[v].push_back(elist[e]);
    }
  }
}

void csr_convert_filter_idx(std::vector<std::vector<DTYPE>> &graph,
                            std::vector<DTYPE> &vlist,
                            std::vector<DTYPE> &elist, DTYPE vertex_cnt) {
  // #pragma omp parallel for schedule(dynamic)
  for (DTYPE v = 0; v < vertex_cnt; v++) {
    DTYPE start = vlist[v];
    DTYPE end = vlist[v + 1];
    for (DTYPE e = start; e < end; e++) {
      // if (elist[e] == 1797726 ) std::cout<<endl << "!!!!" << std::endl;
      graph[elist[e]].push_back(v);
    }
  }
  // for(int ii=0;ii<graph[1797726].size();ii++)
  //  fprintf(stderr, "\n ******  %d ",graph[1797726][ii]);
}

void genNewGraphCSR(std::vector<DTYPE> &vlist, std::vector<DTYPE> &elist,
                    std::vector<std::vector<DTYPE>> &graph,
                    std::vector<DTYPE> &newRuleId,
                    std::vector<bool> &merge_flag, DTYPE vertex_cnt) {
  DTYPE g_size = graph.size();
  vlist.push_back(0);
  DTYPE e_size = 0;
  for (DTYPE v = 0; v < g_size; ++v) {
    DTYPE srcID = v;
    DTYPE e_size = graph[srcID].size();
    if (srcID >= vertex_cnt && merge_flag[srcID - vertex_cnt] == true) continue;
    for (DTYPE e = 0; e < e_size; e++) {
      DTYPE dstID = graph[srcID][e];
      // if(dstID==1486) fprintf(stderr, "\n %d ",srcID);
      DTYPE n_dstID =
          dstID >= vertex_cnt ? newRuleId[dstID - vertex_cnt] : dstID;
      elist.push_back(n_dstID);
    }
    e_size = elist.size();
    vlist.push_back(e_size);
  }
}

void genNewIdForRule(std::vector<DTYPE> &newRuleId, DTYPE &newRule_cnt,
                     std::vector<bool> &merge_flag, DTYPE vertex_cnt,
                     DTYPE rule_cnt) {
  DTYPE cur = vertex_cnt;
  for (DTYPE i = 0; i < rule_cnt; i++) {
    newRuleId[i] = cur;
    if (merge_flag[i] == false) {
      cur++;
    }
  }
  newRule_cnt = cur - vertex_cnt;  // now rule count
}

std::tuple<std::vector<DTYPE>, std::vector<DTYPE>, DTYPE, DTYPE> filter_csr(
    std::vector<DTYPE> &vlist, std::vector<DTYPE> &elist, DTYPE vertex_cnt,
    DTYPE rule_cnt, int _threshold = 16) {
  threshold = _threshold;
  // fprintf(stderr, "filter start...\n");
  //   double start_time = timestamp();
  std::vector<std::vector<DTYPE>> graph(vertex_cnt + rule_cnt);
  csr_convert_filter(graph, vlist, elist, vertex_cnt + rule_cnt);
  std::vector<std::vector<DTYPE>> graphT(vertex_cnt + rule_cnt);
  csr_convert_filter_idx(graphT, vlist, elist, vertex_cnt + rule_cnt);
  std::vector<rule_info> rif;
  rif.resize(rule_cnt);
  std::vector<bool> merge_flag(rule_cnt);
  // cout<<endl<<"********************************************"<<endl;
  // for(int ii=0;ii<graph[1797726].size();ii++)
  //  fprintf(stderr, "\n ******  %d ",graph[1797726][ii]);
  
  // cout<<endl<<"********************************************"<<endl;
  initInfoForRule(graph, graphT, rif, merge_flag, vertex_cnt, rule_cnt);
  for (DTYPE i = 0; i < rule_cnt; i++) {
    if (merge_flag[i] == true) {
      // if(i==797726) cout<<"#################in !!!! true in"<<endl;
      mergeRule(i, graph, graphT, rif, merge_flag, vertex_cnt, rule_cnt);
    }
  }
  // fprintf(stderr, "\n ******************** \n");
  // gen new ID for rule
  DTYPE newRule_cnt;
  std::vector<DTYPE> newRuleId(rule_cnt);
  genNewIdForRule(newRuleId, newRule_cnt, merge_flag, vertex_cnt, rule_cnt);
  std::vector<DTYPE> new_vlist;
  std::vector<DTYPE> new_elist;
  genNewGraphCSR(new_vlist, new_elist, graph, newRuleId, merge_flag,
                 vertex_cnt);
  // fprintf(stderr, "\n ******************** \n");
  //   double end_time = timestamp();
  //   fprintf(stderr, "filter time:%lf\n", end_time - start_time);
  return {new_vlist, new_elist, vertex_cnt, newRule_cnt};
}
