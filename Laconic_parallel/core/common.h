#pragma once
#include <stdint.h>
#include <sys/time.h>

#define DTYPE uint32_t
#define ITYPE uint64_t
#define THRESHOLD 4096

double timestamp() {
  timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + 1e-6 * t.tv_usec;
}