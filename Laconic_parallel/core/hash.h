#pragma once
#include <iostream>
#include <libcuckoo/cuckoohash_map.hh>

template <typename T, typename U>
size_t pair_hash(const std::pair<T, U>& x) {
  return (size_t)x.first << 32 | x.second;
}
