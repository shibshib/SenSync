from multiprocessing import Process
import time
import numpy as np
import matplotlib.pyplot as plt
import networkx as nx
from Node import Node
import math

# Time conversion unit, C should be expressed in same units as cur_time
# in this case, milliseconds.
# C = time unit in milliseconds (e.g. day in milliseconds, hour in milliseconds, etc.)
# For now, year in milliseconds
C = 3.6e+10;

def find_neighbors(G, nodes, node):
    neighbors = G.neighbors(node);
    neighbor_nodes = [];
    for i in range(0, len(neighbors)):
        neighbor_nodes.append(nodes[neighbors[i]]);

    return neighbor_nodes;

def get_neighbor_offsets(neighbors, ref_node):
    total_offsets = 0;
    for i in neighbors:
        total_offsets += abs(i.offset - ref_node.offset);
    return total_offsets;

def get_neighbor_skews(neighbors, ref_node):
    total_skews = 0;
    for i in neighbors:
        total_skews += math.log(i.skew/ref_node.skew);
    return total_skews;

def get_neighbor_offset_diff(node, neighbors):
    total_offset = 0;
    for i in neighbors:
        total_offset += abs((node.offset - i.offset));
    return total_offset

def get_neighbor_skew(node, neighbors):
    total_skew = 0;
    for i in neighbors:
        total_skew += abs((math.log(node.skew)/i.skew));
    return total_skew;

def sync(DG, node_index, nodes, node, ref_node):
    # i+1 here because graph nodes start at 1
    neighbors = find_neighbors(DG, nodes, node_index);

    # Now that we have the neighbors, we can apply the optimization solution
    v_i = (get_neighbor_offsets(neighbors, ref_node) + get_neighbor_offset_diff(node, neighbors)) / len(neighbors);
    u_i = (get_neighbor_skews(neighbors, ref_node) + get_neighbor_skew(node, neighbors)) / len(neighbors);

    return v_i, u_i

def weigh_edges(DG, nodes, start_time):
    edges = DG.edges();
    curtime = lambda: int(round(time.time() * 1000));
    # First, remove all edges
    for e in edges:
        DG.remove_edge(e[0],e[1]);

    # Next, find all weights for edges and re-add them
    for n in range(0, len(nodes)-1):
        weight = (curtime() - nodes[n].sync_time) / C / nodes[n].stderr;
        DG.add_edge(edges[n][0], edges[n][1], weight=weight);
        print("Added edge with weight: ", weight);

    return DG;
if __name__ == '__main__':
    start_time = lambda: int(round(time.time() * 1000));
    # 10 nodes, each running with their own clock
    # Each node is skewed and offset from the original computer clock
    # by a random skew and offset
    N = 6;

    nodes = [];
    count = 0;
    print("Start time: ", start_time());
    for i in range(0, N):
        n = Node();
        print("Node", count+1, " created, Skew = ", n.skew, " offset = ", n.offset)
        count = count + 1;
        nodes.append(n);

    nodes = np.asarray(nodes);

    # Generate a directed graph with 10 nodes
    DG=nx.gnc_graph(N);
    nx.draw(DG);

    # Assign weights to edges
    DG = weigh_edges(DG, nodes, start_time());

#    vi, ui = sync(DG, 1, nodes, nodes[0], nodes[1]);
#    print(vi)
#    print(ui)

    plt.show();
