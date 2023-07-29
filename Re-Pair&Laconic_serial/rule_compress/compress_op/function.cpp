#include "add_self_loop.h"
#include "check.h"
#include "compress.h"
#include "convert.h"
#include "depth_filter.h"
#include "filter.h"
#include "fusion.h"
#include "norm.h"
#include "gen_mask.h"
#include "splice.h"
#include "merge.h"
#include "graphIO.h"
#include "get_part_graph_info.h"
#include <pybind11/chrono.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


PYBIND11_MODULE(rule_compress, m) {
  m.def("compress_csr", &compress_csr, "Compressing graph with csr format");
  m.def("gen_edge_order", &gen_edge_order,
        "generate edge order for compressed data with coo format");
  m.def("gen_vertex_order_csr", &gen_vertex_order_csr,
        "generate vertex order for compressed data with csr format");
  m.def("gen_vertex_order_csc", &gen_vertex_order_csc,
        "generate vertex order for compressed data with csc format");
  m.def("gen_mask_edge", &gen_mask_edge,
        "generate mask for edge of compressed graph");
  m.def("gen_mask_vertex", &gen_mask_vertex,
        "generate mask for vertex of compressed graph");
  m.def("filter_csr", &filter_csr,
        "filter rule for compressed data with csr format");
  m.def("check_coo", &check_coo,
        "Check compressed graph correctness with coo format");
  m.def("check_csr", &check_csr,
        "Check compressed graph correctness with csr format");
  m.def("coo2csr", &coo2csr, "convert coo to csr");
  m.def("csr2coo", &csr2coo, "convert csr to coo");
  m.def("depth_filter_csr", &depth_filter_csr,
        "filter rule for compressed data with depth(CSR)");
  m.def("fusion_mask_csr", &fusion_mask_csr,
        "generate fusion mask with CSR format");
  m.def("fusion_mask_coo", &fusion_mask_coo,
        "generate fusion mask with COO format");
  m.def("csr2coo", &csr2coo, "convert csr to coo format");
  m.def("coo2csr", &coo2csr, "convert coo to csr format");
  m.def("add_self_loop_csr", &add_self_loop_csr, "add self loop to csr format");
  m.def("add_self_loop_coo", &add_self_loop_coo, "add self loop to coo format");
  m.def("gcn_norm_coo", &gcn_norm_coo, "gcn normlization for coo format");
  m.def("gcn_norm_csr", &gcn_norm_csr, "gcn normlization for csr format");
  m.def("get_norm_degree", &get_norm_degree, "get in degree of each vertex");
  m.def("gcn_norm_coo_compress", &gcn_norm_coo_compress, "gcn normalization for coo format of compressed graph");
  m.def("gcn_norm_csr_compress", &gcn_norm_csr_compress, "gcn normalization for csr format of compressed graph");
  m.def("splice_csr", &splice_csr, "splice csr format graph");
  m.def("splice_csr_condense_row", &splice_csr_condense_row, "splice csr with only active row for csr format graph");
  m.def("splice_csr_condense_col", &splice_csr_condense_col, "splice csr with only active col for csr format graph");
  m.def("splice_csr_condense_row_and_col", &splice_csr_condense_row_and_col, "splice csr with active row and active col for csr format graph");
  m.def("get_active_row_and_col", &get_active_row_and_col, "get active row num and col num");
  m.def("get_out_degree", &get_out_degree, "get total degree for a given vertex set");
  m.def("merge", &merge_, "merge subgraph");
  m.def("read_from_file", &read_from_file, "read");
  m.def("output", &output, "output");
}