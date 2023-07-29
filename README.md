Laconic: Enabling Efficient Graph Analytics without Decompression in Memory Constrained Environment
======================

## Introduction
Laconic, a novel hybrid-based graph processing solution that overcomes the challenges of restricted memory and impractical compression time faced by existing approaches. 

Laconic, for the first time, ensures minimal memory overhead during compression and significantly reduces graph sizes, thus also reducing peak memory demand during
computations. By employing an efficient parallel compression algorithm, Laconic achieves a remarkable reduction in compression time. 

This directory contains code of the three modules implemented in Laconic. The code is used for reproduce the results shown in the Laconic paper. 

It contains scripts that allow you to compress data from scratch and perform manipulation and analytic tasks. 

## Organization of Supplementary Materials
- **Re-Pair&Lacibuc_serial**: `The rule compression module discussed in the Laconic paper`.
- **Encoding&Apps**: `The encoding compression module and applications discussed in the paper`.
- **Laconic_parallel**: `The implementation of the serial algorithm mentioned in the paper based on OpenMP`.
- **Results**: Contains the experimental results of the original dataset and various metrics mentioned in the paper, including 1)Compression results of the rule and encoding stages: `origin/`, 2)Memory and time overhead during rule compression, encoding compression, and execution of computational tasks: `peak_memory`.

## Experimental Setup

We conduct a standard performance evaluation of Laconic on a CPU server equipped with an Intel Core i7-12700K CPU, featuring 12 cores and 20 threads. The server has 128GB of global memory, and the operating system used is Ubuntu 20.04.01. To demonstrate Laconic's superiority in handling large graphs such as uk-2014 and clueweb12, we also measure Laconic's performance on another server with an Intel Xeon Gold 6230 CPU @ 2.10GHz, featuring 40 cores and 80 threads. The server has 300GB of global memory, and the operating system used is Ubuntu 20.04.6

## Run example


The flexibility of Laconic compression is excellent, as the results of each compression layer can be directly deployed into graph computing tasks. Therefore, we have organized Laconic's compression results into two or three directories to support compression in different environments. 

Run the code as follows.

1. **Laconic's serial_rule_compression.**

   ## Compile
   We have implemented the serial versions of Laconic and Re-Pair algorithms based on C++ in the directory "Laconic/Re-Pair&Laconic_serial/rule_compress/compress_op".Additionally, we used pybind11 to wrap the C++ code into Python modules.
   
   Run
   
   ```shell
   cd ./Re-Pair&Laconic_serial/rule_compress
   python3 setup.py install
   ```

   to install dependencies and package rule-based compression into a module.

   ## Run
   We have placed the implementations of Laconic's serial rule compression and Re-Pair compression in the "Laconic/Re-Pair&Laconic_serial/compression" directory. You can execute the compression and monitor peak memory usage separately by running either

   ```shell
   monitor_Re-Pair_mem.sh
   ```
   or

   ```shell
   monitor_Laconic_serial_mem.sh
   ```

   You can modify the "datasets" variable in both scripts to test their performance on different datasets.

   For Laconic's serial compression, you can adjust the "batch_nums" to achieve different levels of data partitioning, thereby obtaining varying degrees of reduction in peak memory usage.

2. **Laconic's parallel_rule_compression.** 

   ## Complie
   We have implemented serial rule-based compression in the "Laconic/Laconic_parallel" directory, and can be easily compiled.In "Laconic_parallel directory",run

   ```
   mkdir build && cd build
   cmake ..
   make -j
   ```
   to finish compile

   ## Run 
   
   In the "Laconic/Laconic_parallel/script/" directory,run 

   ```
   cd script
   bash run.sh
   ```

   can test the performance of parallelized rule-based compression in Laconic.The variables "v_dataset" and "e_dataset" in the script represent the paths to CSR_vlist and CSR_elist, respectively.

3. **Laconic's encoding_compression and graph analytic tasks.** 

   In the "Laconic/Encoding&Apps" directory, we have implemented Laconic's encoding compression, where the reordering submodule is accomplished through "bfs_reorder.py". After reordering, Laconic's rule-based compression results achieve significant improvements in encoding compression.

   In the "Encoding&Apps" directory, executing the following commands to complete the reordering submodule.
   ```shell
   python3 bfs_reorder.py
   python3 gen_csr.py
   ```
   Laconic's encoding compression module is built on top of Ligra+. As a result, Laconic's compressed results can be seamlessly utilized by Ligra for computations. To compile Ligra+ in the "/Encoding&Apps/ligra/apps" directory, execute

   ```
   make -j  -openmp=1
   ```

   you can modify the "datasets" variable and different execution statements in /ligra/apps/monitor_ligra.sh to test encoding and graph analysis tasks on different datasets.

