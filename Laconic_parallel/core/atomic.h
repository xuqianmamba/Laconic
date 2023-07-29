#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tbb/scalable_allocator.h>

#include <atomic>
#include <mutex>
#include <thread>

template <typename T>
inline uint64_t atomic_append(std::vector<T> &array,
                              std::atomic_uint64_t &length, const T &data,
                              std::mutex &mutex) {
  uint64_t idx = length.fetch_add(1, std::memory_order_acquire);
  if (idx >= array.size()) {
    std::lock_guard<std::mutex> lock(mutex);
    if (idx >= array.size()) {
      if (array.size() < 1024)
        array.resize(1024);
      else
        array.resize(array.size() * 2);
    }
    std::atomic_thread_fence(std::memory_order_release);
  }
  array[idx] = data;
  return idx;
}

template <class T>
inline bool cas(T *ptr, T old_val, T new_val) {
  if (sizeof(T) == 8) {
    return __sync_bool_compare_and_swap((long *)ptr, *((long *)&old_val),
                                        *((long *)&new_val));
  } else if (sizeof(T) == 4) {
    return __sync_bool_compare_and_swap((int *)ptr, *((int *)&old_val),
                                        *((int *)&new_val));
  } else {
    assert(false);
  }
}

template <class T>
inline bool write_min(T *ptr, T val) {
  volatile T curr_val;
  bool done = false;
  do {
    curr_val = *ptr;
  } while (curr_val > val && !(done = cas(ptr, curr_val, val)));
  return done;
}

template <class T>
inline void write_add(T *ptr, T val) {
  volatile T new_val, old_val;
  do {
    old_val = *ptr;
    new_val = old_val + val;
  } while (!cas(ptr, old_val, new_val));
}

inline void *myTBBMalloc(long long n) {
  void *p;
  if (n == 0) return NULL;
  p = scalable_malloc(n);
  if (p == NULL) {
    fprintf(stderr, "Error: myTBBMalloc failed\n");
    exit(1);
  }
  return p;
}

inline void *myTBBRealloc(void *p, long long n) {
  if (n == 0) {
    free(p);
    return NULL;
  }
  if (p == NULL) return myTBBMalloc(n);
  void *new_p = scalable_realloc(p, n);
  if (p == NULL) {
    fprintf(stderr, "Error: myTBBRealloc failed\n");
    exit(1);
  }
  return p;
}