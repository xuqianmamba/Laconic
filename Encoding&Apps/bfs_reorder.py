import numpy as np

# with open('csr_vlist.txt') as file:
#     vlist = np.genfromtxt(file, dtype=int, delimiter='\n')
# with open('csr_elist.txt') as file:
#     elist = np.genfromtxt(file, dtype=int, delimiter='\n')
data="uk-2007-05@100000"
vlist = np.load("../benchmarks/"+data+"/Laconic_rule_compressed/rule_compressed_vlist.npy")
elist = np.load("../benchmarks/"+data+"/Laconic_rule_compressed/rule_compressed_elist.npy")

print("len_v:",len(vlist))
print("max_e:",np.max(elist))


# Initialize variables
n = len(vlist)
bfs_queue = []  # start BFS from where
visited = np.zeros(n-1, dtype=bool)
new_id = 0

new_vertex_id = np.zeros(n-1, dtype=int)#a hash map between old and new id   new_vertex_id[old_id]=new_id

reverse_id = np.zeros(n-1, dtype=int)#a hash map between old and new id   new_vertex_id[new_id]=old_id

def bfs():
    # BFS
    global new_id,visited,new_vertex_id,reverse_id,bfs_queue,n
    while bfs_queue:
        curr_node = bfs_queue.pop()
        
        #make hash_table
        new_vertex_id[new_id]=curr_node
        reverse_id[curr_node]=new_id
        
        new_id += 1

        for i in range(vlist[curr_node], vlist[curr_node+1]):
            neighbor = elist[i]
            if not visited[neighbor]:
                visited[neighbor] = True
                bfs_queue.append(neighbor)

print("new_id",new_id)
# Map remaining unvisited vertices to new IDs
for i in range(n-1):
    if not visited[i]:
        bfs_queue=[i]
        visited[i] = True
        bfs()
print("new_id",new_id)


count=0
with open("../benchmarks/"+data+"/reorder/reordered_vlist.txt", 'w') as f1, open("../benchmarks/"+data+"/reorder/reordered_elist.txt", 'w') as f2:
    f1.write(str(count)+'\n')
    for i in range(n-1):
        if (new_vertex_id[i]+1>=len(vlist)):
            end_pos=len(elist)
        else :
            end_pos= vlist[new_vertex_id[i]+1]
        for j in range(vlist[new_vertex_id[i]], end_pos):
            f2.write(str(reverse_id[elist[j]]) + '\n')
            count+=1
        f1.write(str(count)+'\n')
