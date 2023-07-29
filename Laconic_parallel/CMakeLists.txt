cmake_minimum_required(VERSION 3.1)
project(Repair_Impl CXX)

set(CMAKE_CXX_STANDARD 17)
set(DABSL_BUILD_TESTING ON)
set(DABSL_USE_GOOGLETEST_HEAD ON)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_CXX_FLAGS "-fopenmp -LLIBDIR -O3")
endif()

# find_package(absl REQUIRED)

set(PROJECT_DEPS_DIR ${PROJECT_SOURCE_DIR}/deps)
add_subdirectory(${PROJECT_DEPS_DIR}/abseil-cpp)
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_DEPS_DIR} ${PROJECT_DEPS_DIR}/abseil-cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(compress src/compress.cc)
target_link_libraries(compress PRIVATE absl::flat_hash_map) 