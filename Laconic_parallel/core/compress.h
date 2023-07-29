#pragma once
#include <omp.h>

#include <iostream>
#include <libcuckoo/cuckoohash_map.hh>
#include <mutex>
#include <thread>
#include <vector>

#include "../deps/abseil-cpp/absl/container/flat_hash_map.h"
#include "atomic.h"
#include "check.h"
#include "common.h"
#include "filter.h"
#include "hash.h"
#include <chrono>

namespace repair {

using HashMap = absl::flat_hash_map<std::size_t, ITYPE>;
std::vector<HashMap> hash_table, convert_id;
std::vector<std::vector<int>> new_edge_sequence, new_vertex_sequence;
std::vector<std::vector<DTYPE>> role_vertex, role_edge;
size_t thread_num = 40, threshold =4;
std::vector<size_t> new_start_id(thread_num);

class processor {
 public:
  static void processpairs(const std::vector<size_t>& pairs,
                           absl::flat_hash_map<std::size_t, ITYPE>& table) {
    for (size_t pair : pairs) {
      auto iter = table.find(pair);
      if (iter != table.end()) {
        // std::cout << "find" << std::endl;
        iter->second++;
      } else {
        table.insert(std::make_pair(pair, 1));
      }
    }
    absl::erase_if(table, [](std::pair<std::size_t, ITYPE> kvp) {
      return kvp.second < threshold;
    });
  }

  // record the role edge id
  static void prepare_new_id(HashMap& table, size_t& start_id, HashMap& convert,
                             std::vector<DTYPE>& temp_role_edge,
                             std::vector<DTYPE>& temp_role_vertex) {
    size_t cur_v_offset = 0;
    int left, right;
    for (auto iter = table.begin(); iter != table.end(); iter++) {
      // std::cout << iter->first << std::endl;
      convert.insert(std::make_pair(iter->first, start_id));
      start_id++;
      temp_role_vertex.push_back(cur_v_offset);
      cur_v_offset += 2;
      left = (int)(iter->first >> 32);
      right = (int)(iter->first);
      temp_role_edge.push_back(left);
      temp_role_edge.push_back(right);
    }
  }

  static void replace(const std::vector<int>& origin_edge_sequence,
                      std::vector<int>& origin_vertex_sequence,
                      std::vector<int>& new_edge_sequence,
                      std::vector<int>& new_vertex_sequence) {
    int vidx, idx, len = origin_edge_sequence.size(), table_id, start,
                   vsize = 0, offset = origin_vertex_sequence[0];
    for (vidx = 0; vidx < origin_vertex_sequence.size(); vidx++) {
      origin_vertex_sequence[vidx] -= offset;
    }
    for (vidx = 0; vidx < origin_vertex_sequence.size() - 1; vidx++) {
      new_vertex_sequence.push_back(vsize);
      for (idx = origin_vertex_sequence[vidx];
           idx < origin_vertex_sequence[vidx + 1] - 1; idx++) {
        table_id = origin_edge_sequence[idx] % thread_num;
        auto pair = std::make_pair(origin_edge_sequence[idx],
                                   origin_edge_sequence[idx + 1]);
        auto hash_value = pair_hash(pair);
        if (hash_table[table_id].find(hash_value) !=
            hash_table[table_id].end()) {
          new_edge_sequence.push_back(convert_id[table_id][hash_value]);
          idx++;
        } else {
          new_edge_sequence.push_back(origin_edge_sequence[idx]);
        }
        vsize++;
      }
      // if last neighbor not be a pair
      if (idx < origin_vertex_sequence[vidx + 1]) {
        new_edge_sequence.push_back(
            origin_edge_sequence[origin_vertex_sequence[vidx + 1] - 1]);
        vsize++;
      }
    }

    new_vertex_sequence.push_back(vsize);
    for (idx = origin_vertex_sequence[vidx];
         idx < origin_edge_sequence.size() - 1; idx++) {
      table_id = origin_edge_sequence[idx] % thread_num;
      auto pair = std::make_pair(origin_edge_sequence[idx],
                                 origin_edge_sequence[idx + 1]);
      auto hash_value = pair_hash(pair);
      if (hash_table[table_id].find(hash_value) != hash_table[table_id].end()) {
        new_edge_sequence.push_back(convert_id[table_id][hash_value]);
        idx++;
      } else {
        new_edge_sequence.push_back(origin_edge_sequence[idx]);
      }
      vsize++;
    }
    if (idx < origin_edge_sequence.size()) {
      new_edge_sequence.push_back(
          origin_edge_sequence[origin_edge_sequence.size() - 1]);
      vsize++;
    }
  };
};

// start new id for each frenq rule
class Repair {
 public:
  using Trecord = struct record;
  Repair(){};
  Repair(std::vector<DTYPE>& input_edge, std::vector<DTYPE>& input_vertex,
         DTYPE max_symbol) {
    std::vector<DTYPE> input_edge1 = input_edge;
    std::vector<DTYPE> input_vertex1 = input_vertex;
    ITYPE origin_edge_len = input_edge.size();
    ITYPE origin_vertex_len =
        input_vertex.size();  // should be max_v_id +3, v_id : 0,1,2...max_v_id
    if (input_vertex.size() != max_symbol + 2) {
      fprintf(stderr,
              "bad input! max symbol is : %d, input_vertex.size() is %ld\n",
              max_symbol, input_vertex.size());
    }
    else {
      fprintf(stderr,"input success \n");
    }
    ITYPE len_edge = input_edge.size();      // this turns size
    ITYPE len_vertex = input_vertex.size();  // this turns size
    DTYPE max_id = max_symbol;
    size_t i, idx, idy;
    std::vector<std::vector<size_t>> pair_buckets(thread_num);
    std::vector<std::vector<int>> buckets(thread_num),
        vertex_buckets(thread_num);
    std::vector<std::thread> threads;
    // compress in many times,to catch deep role
    size_t compress_rate = 3;
    // reserve space for hash_table and id_convert_table
    for (idx = 0; idx < thread_num; idx++) {
      HashMap temp_map1(80000000), temp_map2(40000000);
      hash_table.push_back(temp_map1);
      convert_id.push_back(temp_map2);
    }
    for (size_t compress_turns = 1; compress_turns <= compress_rate;
         compress_turns++) {
      std::cout << "the " << compress_turns << "turns :" << std::endl;
      // if (input_vertex[input_vertex.size() - 1] - 1 != input_edge.size()) {
      //   std::cout << "error in conpress_turns " << compress_turns <<
      //   std::endl; std::cout << "v_end  is " <<
      //   input_vertex[input_vertex.size() - 1] - 1
      //             << std::endl;
      //   std::cout << "input_edge size is " << input_edge.size() <<
      //   std::endl;
      // }
      // std::cout << "max_id is " << max_id << std::endl;

      // std::cout << "buckets.size()" << buckets.size() << std::endl;
      // first parallel allocate pairs to each buckets
      auto start = std::chrono::high_resolution_clock::now();
      for (idy = 0; idy < input_vertex.size() - 1; idy++) {
        // Guarantees that no neighbors across pair appears
        for (idx = input_vertex[idy]; idx < input_vertex[idy + 1] - 1; idx++) {
          auto pair = std::make_pair(input_edge[idx], input_edge[idx + 1]);
          auto hash_value = pair_hash(pair);
          int thread_id = input_edge[idx] % thread_num;
          pair_buckets[thread_id].push_back(hash_value);
        }
      }

      std::cout << "**********" << std::endl;
      // count times, erase low frenq
      for (idx = 0; idx < thread_num; ++idx) {
        threads.emplace_back(&processor::processpairs,
                             std::ref(pair_buckets[idx]),
                             std::ref(hash_table[idx]));
      }

      for (auto& thread : threads) {
        thread.join();
      }

      uint64_t total_size = 0;
      for (int i = 0; i < thread_num; i++) {
        total_size += hash_table[i].size();
        // for (auto iter = hash_table[i].begin(); iter != hash_table[i].end();
        //      iter++) {
        //   fprintf(stderr, "hashmap item:%u,%u\n", uint32_t(iter->first >>
        //   32),
        //           uint32_t(iter->first));
        // }
      }
      fprintf(stderr, "hashmap_total_size:%lu\n", total_size);

      threads.clear();

      // second parallel allocate new id for roles
      new_start_id[0] = max_id + 1;
      std::vector<DTYPE> temp_role_edge, temp_role_vertex;
      role_edge.push_back(temp_role_edge);
      role_vertex.push_back(temp_role_edge);
      for (idx = 1; idx < thread_num; ++idx) {
        std::vector<DTYPE> temp_role_edge, temp_role_vertex;
        role_edge.push_back(temp_role_edge);
        role_vertex.push_back(temp_role_edge);
        new_start_id[idx] = new_start_id[idx - 1];
        new_start_id[idx] += hash_table[idx - 1].size();
      }
      // allocate ids for new roles parallely
      // size_t thread_size = input_edge.size() / thread_num;
      // for (idx = 0; idx < thread_num - 1; ++idx) {
      //   buckets[idx].clear();
      //   buckets[idx] =
      //       std::vector<size_t>(input_edge.begin() + idx * thread_size,
      //                           input_edge.begin() + (idx + 1) *
      //                           thread_size);
      // }

      for (idx = 0; idx < thread_num; ++idx) {
        threads.emplace_back(
            &processor::prepare_new_id, std::ref(hash_table[idx]),
            std::ref(new_start_id[idx]), std::ref(convert_id[idx]),
            std::ref(role_edge[idx]), std::ref(role_vertex[idx]));
      }

      for (auto& thread : threads) {
        thread.join();
      }


      size_t check_role_v_num = 0, check_role_e_num = 0;

      // // ok,no bug
      // for (idx = 0; idx < thread_num; ++idx) {
      //   check_role_v_num += role_vertex[idx].size();
      //   check_role_e_num += role_edge[idx].size();
      // }size_tl;

      // std::cout << "id assign turns  " << compress_turns << "  completed"
      //           << std::endl;
      // // nobug

      threads.clear();

      fprintf(stderr, "attribute done:\n");
      // last parallel split origin to parallely use re-pair compress
      size_t thread_size = len_vertex / thread_num;
      for (idx = 0; idx < thread_num - 1; ++idx) {
        std::vector<int> thread_sequence, temp_vertex_bucket, new_vertex_bucket;
        new_edge_sequence.push_back(thread_sequence);
        new_vertex_sequence.push_back(new_vertex_bucket);
        // each bucket contains thread_num vertexes
        buckets[idx].clear();
        vertex_buckets[idx] =
            std::vector<int>(input_vertex.begin() + idx * thread_size,
                             input_vertex.begin() + (idx + 1) * thread_size);
        buckets[idx] = std::vector<int>(
            input_edge.begin() + input_vertex[idx * thread_size],
            input_edge.begin() + input_vertex[(idx + 1) * thread_size]);
      }
      buckets[thread_num - 1] = std::vector<int>(
          input_edge.begin() + input_vertex[(thread_num - 1) * thread_size],
          input_edge.end());

      vertex_buckets[thread_num - 1] = std::vector<int>(
          input_vertex.begin() + (thread_num - 1) * thread_size,
          input_vertex.end() - 1);

      std::vector<int> thread_sequence, new_vertex_bucket;
      new_edge_sequence.push_back(thread_sequence);
      new_vertex_sequence.push_back(new_vertex_bucket);
      // std::cout << buckets.size() << std::endl;
      // std::cout << vertex_buckets.size() << std::endl;
      // std::cout << new_edge_sequence.size() << std::endl;
      // std::cout << new_vertex_sequence.size() << std::endl;
      for (idx = 0; idx < thread_num; ++idx) {
        threads.emplace_back(&processor::replace, std::ref(buckets[idx]),
                             std::ref(vertex_buckets[idx]),
                             std::ref(new_edge_sequence[idx]),
                             std::ref(new_vertex_sequence[idx]));
      }
      for (auto& thread : threads) {
        thread.join();
      }
      // check_role_v_num = 0, check_role_e_num = 0;

      // //  no bug
      // for (idx = 0; idx < thread_num; ++idx) {
      //   check_role_v_num += new_vertex_sequence[idx].size();
      //   check_role_e_num += new_edge_sequence[idx].size();
      // }
      // std::cout << "check_role_e_num" << check_role_e_num << std::endl;
      // std::cout << "check_role_v_num" << check_role_v_num << std::endl;

      // // no bug

      threads.clear();
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration = end - start;
      double seconds = duration.count();


      std::cout << "time comsumption" << seconds << " s" << std::endl;


      // cal new sequence size
      input_edge.clear();
      input_vertex.clear();
      std::vector<size_t> batch_offset;
      for (size_t batch_id = 0; batch_id < new_edge_sequence.size();
           batch_id++) {
        batch_offset.push_back(input_edge.size());
        input_edge.insert(input_edge.end(), new_edge_sequence[batch_id].begin(),
                          new_edge_sequence[batch_id].end());
      }
      for (size_t vertex_id = 0; vertex_id < thread_num; vertex_id++) {
        auto& vector = new_vertex_sequence[vertex_id];
        for (auto& vertex : vector) {
          vertex += batch_offset[vertex_id];
        }
        input_vertex.insert(input_vertex.end(), vector.begin(), vector.end());
        vector.clear();
      }
      // input_vertex.pop_back();
      // fprintf(stderr, "origin input_vertex.size() :%ld\n", input_vertex.size());
      // fprintf(stderr, "origin input_edge.size() :%ld\n", input_edge.size());
      // merge role e and v
      for (idx = 0; idx < thread_num; ++idx) {
        size_t edge_num = input_edge.size();
        for (size_t role_edge_id = 0; role_edge_id < role_vertex[idx].size();
             role_edge_id++) {
          role_vertex[idx][role_edge_id] += edge_num;
        }
        input_vertex.insert(input_vertex.end(), role_vertex[idx].begin(),
                            role_vertex[idx].end());
        input_edge.insert(input_edge.end(), role_edge[idx].begin(),
                          role_edge[idx].end());
      }

      input_vertex.push_back(input_edge.size());
      max_id = input_vertex.size() - 2;

      // fprintf(stderr, "input_vertex.size() :%ld\n", input_vertex.size());
      // fprintf(stderr, "input_edge.size() :%ld\n", input_edge.size());

      // fprintf(stderr, "compress ratio:%lf\n",
      //         ((double)origin_edge_len + (double)origin_vertex_len) /
      //             ((double)input_edge.size() + (double)input_vertex.size()));
      len_edge = input_edge.size();
      for (idx = 0; idx < thread_num; ++idx) {
        hash_table[idx].clear();
        convert_id[idx].clear();
        role_vertex[idx].clear();
        role_edge[idx].clear();
        new_edge_sequence.clear();
        new_vertex_sequence.clear();
      }
    }

    // // drop pairs which occurs less than 3 times,is it necessary?
  }

  ~Repair() {}

  void compress() {}
};
};  // namespace repair