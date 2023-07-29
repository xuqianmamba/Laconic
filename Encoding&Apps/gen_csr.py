import numpy as np


data="uk-2007-05@100000"
with open("../benchmarks/"+data+"/reorder/reordered_vlist.txt") as file:
    vlist = np.genfromtxt(file, dtype=np.int64, delimiter=' ')
with open("../benchmarks/"+data+"/reorder/reordered_elist.txt") as file:
    elist = np.genfromtxt(file, dtype=np.int64, delimiter=' ')

# vlist = np.load('compressed_vlist.npy')
# elist = np.load('compressed_elist.npy')

print("load finish")
with open("../benchmarks/"+data+"/reorder/unencoded.txt", 'w') as f:
    f.write("AdjacencyGraph" + '\n')

    f.write(str(len(vlist)) + '\n')

    f.write(str(len(elist)) + '\n')

    for v in vlist:
        f.write(str(v) + '\n')

    for i, e in enumerate(elist):
        if i == len(elist) - 1:
            f.write(str(e))
        else:
            f.write(str(e) + '\n')