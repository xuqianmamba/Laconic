#pragma once
#include <fstream>
#include <iostream>
#include <istream>
#include <vector>

#include "common.h"

/*
 * load file and return the max sombol
 */

DTYPE load_edge_file(const std::string path, std::vector<DTYPE> &sequence) {
  DTYPE max_symbol = 0;
  DTYPE len;
  DTYPE tmp;
  std::ifstream infile(path);
  if (!infile.good()) {
    fprintf(stderr, "file not exist:%s\n", path.c_str());
    exit(0);
  }
  while (infile >> tmp) {
    sequence.emplace_back(tmp);
    max_symbol = std::max(tmp, max_symbol);
  }
  return max_symbol;
}

void load_vertex_file(const std::string path, std::vector<DTYPE> &sequence) {
  DTYPE max_symbol = 0;
  DTYPE len;
  DTYPE tmp;
  std::ifstream infile(path);
  if (!infile.good()) {
    fprintf(stderr, "file not exist:%s\n", path.c_str());
    exit(0);
  }
  while (infile >> tmp) {
    sequence.emplace_back(tmp);
  }
  return;
}