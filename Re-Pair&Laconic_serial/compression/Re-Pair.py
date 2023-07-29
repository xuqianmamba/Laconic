import numpy as np
from rule_compress import compress_csr, merge, filter_csr, check_csr
import os
import tracemalloc
import mmap
import time
time_total=0
import argparse

def main(datasets):
    root_path = "../../benchmarks/"
    for data in datasets:
        print(data)

        vlist = np.load(root_path+data+"/origin/csr_vlist.npy")
        elist = np.load(root_path+data+"/origin/csr_elist.npy")
        print("vlist.size is  ",vlist.shape[0])
        print("elist.size is  ",elist.shape[0])
        print("vlist is  ",vlist)
        print("elist is  ",elist)
        compressed_vlist, compressed_elist, vertex_cnt, rule_cnt = compress_csr(
            vlist, elist, vlist.shape[0] - 1)
        print(
            "compression ratio: {} / {} = {:.4f}".format(
                vlist.shape[0] +
                elist.shape[0],
                compressed_vlist.shape[0] +
                compressed_elist.shape[0], (vlist.shape[0] + elist.shape[0]) / (compressed_vlist.shape[0] + compressed_elist.shape[0])))
        if not os.path.exists(root_path+data+"/rule_compressed"):
            os.mkdir(root_path+data+"/rule_compressed")
        np.save(root_path+data+"/Re-Pair_compressed/compressed_vlist.npy", compressed_vlist)
        np.save(root_path+data+"/Re-Pair_compressed/compressed_elist.npy", compressed_elist)
        compressed_vlist, compressed_elist, vertex_cnt, rule_cnt = filter_csr(
            compressed_vlist, compressed_elist, vertex_cnt, rule_cnt, 0)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('datasets', nargs='+', help='List of datasets')
    args = parser.parse_args()

    main(args.datasets)