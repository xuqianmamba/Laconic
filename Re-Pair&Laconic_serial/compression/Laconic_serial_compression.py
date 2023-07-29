import numpy as np
from rule_compress import compress_csr, merge, filter_csr
import os
import tracemalloc
import mmap
import time
time_total=0
import argparse


#txt version
def read_nth_int64_element_txt(filename, n):
    with open(filename, 'r') as file:
        # Skip to the n-th line by reading and discarding lines
        for _ in range(n):
            next(file)
        
        # Read the n-th line and extract the integer element
        element = np.int64(next(file).strip())

    return element

def get_num_int64_elements_txt(filename):
    with open(filename, 'r') as file:
        # Count the number of lines in the file
        num_elements = sum(1 for _ in file)

    return num_elements

def read_range_from_txt(filename, start_index, end_index):
    elements = []
    with open(filename, 'r') as file:
        # Skip to the start_index by reading and discarding lines
        for _ in range(start_index):
            next(file)
        
        # Read the range of lines and append the integer elements to the list
        for _ in range(start_index, end_index + 1):
            line = next(file)
            elements.append(int(line.strip()))

    return elements

#npy version
def read_nth_int64_element(filename, n):
    with open(filename, 'rb') as file:
        # Skip the npy file header (128 bytes)
        file.seek(128)

        # Read n+1 np.int64 elements and extract the nth element
        elements = np.fromfile(file, dtype=np.int64, count=n+1)
        element = elements[n]

    return element

def npy_get_num_int64_elements(filename):
    file_size = os.path.getsize(filename)
    num_bytes = file_size - 128  # Subtract the size of the npy file header (128 bytes)
    num_elements = num_bytes // 8  # Each np.int64 element occupies 8 bytes
    return num_elements

def read_range_from_npy(filename, start_index, end_index):
    with open(filename, 'rb') as file:
        # Skip the npy file header (128 bytes)
        file.seek(128 + start_index * 8)  # Each np.int64 element occupies 8 bytes

        # Calculate the count based on the range
        count = (end_index - start_index + 1)

        # Read the range of np.int64 elements
        elements = np.fromfile(file, dtype=np.int64, count=count)

    return elements




def Laconic_serial(v_filename, e_filename,part_num):
    global time_total
    
    # for txt input file
    # vertex_cnt =get_num_int64_elements_txt (v_filename_txt)-1
    vertex_cnt = npy_get_num_int64_elements(v_filename)-1
    print("vertex_cnt: ",vertex_cnt)

    tile = int(vertex_cnt / part_num) + 1
    c_vlist = []
    c_elist = []
    c_vertex_cnt = []
    c_rule_cnt = []

    for i in range(part_num):
        
        print("part ",i," starts")
        start = i * tile
        
        end = (i + 1) * tile
        if end > vertex_cnt:
            end = vertex_cnt
            
        # for txt input file
        # v_start=read_nth_int64_element_txt(v_filename_txt,start)
        v_start=read_nth_int64_element(v_filename,start)


        # for txt input file
        # v_end=read_nth_int64_element_txt(v_filename_txt,end)
        v_end=read_nth_int64_element(v_filename,end)
        
        # for txt input file
        # part_vlist = read_range_from_txt(v_filename_txt,start,end ) - v_start
        part_vlist = read_range_from_npy(v_filename,start,end ) - v_start
        
        # for txt input file
        # part_vlist = vlist[start:end + 1] - vlist[start]

        # for txt input file
        # part_elist = read_range_from_txt(e_filename_txt,v_start,v_end)
        part_elist = read_range_from_npy(e_filename,v_start,v_end)

        start_time = time.time()
        compressed_vlist, compressed_elist, new_vertex_cnt, new_rule_cnt = compress_csr(
            part_vlist, part_elist, vertex_cnt)
        end_time = time.time()

        run_time = end_time - start_time
        time_total+=run_time

        c_vlist.append(compressed_vlist)
        c_elist.append(compressed_elist)
        c_vertex_cnt.append(new_vertex_cnt)
        c_rule_cnt.append(new_rule_cnt)
    
    return c_vlist, c_elist, c_vertex_cnt, c_rule_cnt

def main(datasets,part_num):
    root_path = "../../benchmarks/"

    
    for data in datasets:
        print(data)
        
        # for txt input file
        # with open('csr_vlist.txt') as file:
        #     vlist = np.genfromtxt(file, dtype=np.int64, delimiter=' ')
        # with open('csr_elist.txt') as file:
        #     elist = np.genfromtxt(file, dtype=np.int64, delimiter=' ')

        v_filename = root_path+data+"/origin/csr_vlist.npy"
        e_filename = root_path+data+"/origin/csr_elist.npy"
        
        compressed_vlist, compressed_elist, vertex_map, rule_map = Laconic_serial(
            v_filename, e_filename,part_num)
        compressed_vlist, compressed_elist, new_vertex_cnt, new_rule_cnt = merge(
            compressed_vlist, compressed_elist, vertex_map, rule_map)

        compressed_vlist, compressed_elist, new_vertex_cnt, new_rule_cnt = filter_csr(
            compressed_vlist, compressed_elist, new_vertex_cnt, new_rule_cnt, 0)  
        
        if not os.path.exists(root_path+data+"/rule_compressed"):
            os.mkdir(root_path+data+"/rule_compressed")
        np.save(root_path+data+"/rule_compressed/rule_compressed_vlist.npy", compressed_vlist)
        np.save(root_path+data+"/rule_compressed/rule_compressed_elist.npy", compressed_elist)
        print("time_total is ",time_total)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('datasets', nargs='+', help='List of datasets')
    parser.add_argument('--part_num', type=int, default=100, help='Value of part_num (default: 100)')
    args = parser.parse_args()

    main(args.datasets,args.part_num)