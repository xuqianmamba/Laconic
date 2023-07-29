#include <core/IO.h>
#include <core/check.h>
#include <core/common.h>
#include <core/compress.h>

#include <iostream>
#include <libcuckoo/cuckoohash_map.hh>

#include "../core/filter.h"

int main(int argc, char** argv) {
  std::string edge_file_path = argv[1];
  std::string vertex_file_path = argv[2];
  fprintf(stderr, "edge file:%s\n", edge_file_path.c_str());
  fprintf(stderr, "vertex file:%s\n", vertex_file_path.c_str());
  DTYPE max_symbol, vertex_cnt, newRule_cnt;
  std::vector<DTYPE> sequence_edge, sequence_vertex, origin_sequence_edge,
      origin_sequence_vertex, new_vlist, new_elist;
  double stime = timestamp();
  max_symbol = load_edge_file(edge_file_path, sequence_edge);
  load_vertex_file(vertex_file_path, sequence_vertex);


  origin_sequence_vertex = sequence_vertex;
  origin_sequence_edge = sequence_edge;

  double etime = timestamp();
  fprintf(stderr, "load file finish: %.4fs\n", etime - stime);
  stime = timestamp();

  repair::Repair R(sequence_edge, sequence_vertex, max_symbol);
  // std::cout << std::endl << "Vectors written to files start." << std::endl;

  // (void)!freopen("compressed_vertex.txt", "w", stdout);
  // for (int i = 0; i < sequence_vertex.size(); i++) {
  //   std::cout << sequence_vertex[i] << "\n";
  // }
  // fclose(stdout);
  // (void)!freopen("compressed_edge.txt", "w", stdout);

  // for (const int& element : sequence_edge) {
  //   std::cout << element << "\n";
  // }
  // fclose(stdout);

  // fprintf(stderr, "\n Vectors written to files successfully \n");

  // fprintf(stderr, "\n begin debug \n");

  // fprintf(stderr, "origin_sequence_vertex.size(): %ld\n",
  //         origin_sequence_vertex.size());
  // fprintf(stderr, "origin_sequence_edge.size(): %ld\n",
  //         origin_sequence_edge.size());
  // fprintf(stderr, "sequence_vertex.size(): %ld\n", sequence_vertex.size());
  // fprintf(stderr, "sequence_edge.size(): %ld\n", sequence_edge.size());
  // fprintf(stderr, "compress ratio:%lf\n",
  //         ((double)origin_sequence_vertex.size() +
  //          (double)origin_sequence_edge.size()) /
  //             ((double)sequence_vertex.size() + (double)sequence_edge.size()));

  // check_csr(origin_sequence_vertex, origin_sequence_edge, sequence_vertex,
  //           sequence_edge, origin_sequence_vertex.size() - 1,
  //           sequence_vertex.size() - origin_sequence_vertex.size());

  // std::tuple<std::vector<DTYPE>, std::vector<DTYPE>, DTYPE, DTYPE> result =
  //     filter_csr(sequence_vertex, sequence_edge,
  //                origin_sequence_vertex.size() - 1,
  //                sequence_vertex.size() - origin_sequence_vertex.size());
  // std::vector<DTYPE> filtered_vlist = std::get<0>(result);
  // std::vector<DTYPE> filtered_elist = std::get<1>(result);

  // std::cout << std::endl << "Vectors written to files start." << std::endl;

  // (void)!freopen("filtered_vertex.txt", "w", stdout);
  // for (int i = 0; i < filtered_vlist.size(); i++) {
  //   std::cout << filtered_vlist[i] << "\n";
  // }
  // fclose(stdout);
  // (void)!freopen("filtered_edge.txt", "w", stdout);

  // for (const int& element : filtered_elist) {
  //   std::cout << element << "\n";
  // }
  // fclose(stdout);

  // fprintf(stderr, "\n Vectors written to files successfully \n");

  // fprintf(stderr, "filtered_sequence_vertex.size(): %ld\n", sequence_vertex.size());
  // fprintf(stderr, "filtered_sequence_edge.size(): %ld\n", sequence_edge.size());
  // fprintf(stderr, "compress ratio after filter:%lf\n",
  //         ((double)origin_sequence_vertex.size() +
  //          (double)origin_sequence_edge.size()) /
  //             ((double)filtered_vlist.size() + (double)filtered_elist.size()));

  // check_csr(origin_sequence_vertex, origin_sequence_edge, filtered_vlist,
  //           filtered_elist, origin_sequence_vertex.size() - 1,
  //           filtered_vlist.size() - origin_sequence_vertex.size());

  etime = timestamp();
  fprintf(stderr, "\nFinish in : %.4f\n", etime - stime);
  return 0;
}
