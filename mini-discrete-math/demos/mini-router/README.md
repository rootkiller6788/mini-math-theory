# Mini Router — Dijkstra / Bellman-Ford Network Routing Simulation

## Overview

A network routing simulator using shortest-path algorithms to compute optimal
routes through a network topology. Supports link weights (cost, latency, bandwidth),
negative weights (via Bellman-Ford), and dynamic topology changes.

## Use Case

Link-state routing protocols (OSPF, IS-IS) use Dijkstra's algorithm to compute
shortest-path trees. Distance-vector protocols (RIP) use distributed Bellman-Ford.
This demo simulates the centralized computation that each router performs.

## Architecture

```
Network Topology → Weighted Graph → Dijkstra / Bellman-Ford → Routing Table
                                     ↓
                               Shortest-Path Tree (per router)
```

- **Nodes**: Routers (numbered 0..N-1)
- **Edges**: Links between routers with a cost metric (delay, bandwidth, hop count)
- **Output**: Per-router forwarding table mapping destination → next hop + total cost

## Implementation Sketch

### Step 1: Define Router and Topology

```c
#include "graph.h"
#include <stdio.h>
#include <string.h>

#define MAX_ROUTERS 16
#define MAX_LINKS   64

typedef struct {
    char name[16];     // e.g., "R1", "core-1"
    char ip[16];       // e.g., "10.0.0.1"
    int  as_number;    // Autonomous System (for inter-domain)
} Router;

typedef struct {
    int from;          // source router index
    int to;            // destination router index
    int cost;          // link metric
} Link;

typedef struct {
    Router routers[MAX_ROUTERS];
    int n_routers;
    Link links[MAX_LINKS];
    int n_links;
} NetworkTopology;
```

### Step 2: Build the Graph

```c
void topology_to_graph(NetworkTopology* topo, Graph* g) {
    graph_init(g, topo->n_routers, false);  // undirected for symmetric links

    for (int i = 0; i < topo->n_links; i++) {
        graph_add_edge(g, topo->links[i].from,
                       topo->links[i].to,
                       topo->links[i].cost);
    }
}
```

### Step 3: Compute Routing Table for a Single Router (Dijkstra)

```c
void compute_routing_table(Graph* g, int router_id, NetworkTopology* topo) {
    int dist[MAX_VERTICES], prev[MAX_VERTICES];
    shortest_path_dijkstra(g, router_id, dist, prev);

    printf("\n========================================\n");
    printf("Routing Table — %s (%s)\n",
           topo->routers[router_id].name,
           topo->routers[router_id].ip);
    printf("========================================\n");
    printf("%-12s %-12s %-12s %s\n", "Destination", "Next Hop", "Cost", "Path");

    for (int dest = 0; dest < g->nvertices; dest++) {
        if (dest == router_id) continue;
        if (dist[dest] == INF) {
            printf("%-12s %-12s %-12s %s\n",
                   topo->routers[dest].name, "—", "∞", "unreachable");
            continue;
        }

        // Find next hop by backtracking to router_id's neighbor
        int next_hop = dest;
        while (prev[next_hop] != router_id && prev[next_hop] != -1)
            next_hop = prev[next_hop];

        // Reconstruct full path
        int path[MAX_VERTICES], plen = 0, cur = dest;
        while (cur != -1) {
            path[plen++] = cur;
            cur = prev[cur];
        }

        printf("%-12s %-12s %-12d ",
               topo->routers[dest].name,
               topo->routers[next_hop].name,
               dist[dest]);
        for (int p = plen - 1; p >= 0; p--)
            printf("%s%s", topo->routers[path[p]].name, p > 0 ? "→" : "");
        printf("\n");
    }
}
```

### Step 4: Full Network Routing (All-Pairs with Floyd-Warshall)

```c
void compute_all_routes(Graph* g, NetworkTopology* topo) {
    int dist[MAX_VERTICES][MAX_VERTICES];
    shortest_path_floyd(g, dist);

    printf("\n=== Network Distance Matrix ===\n");
    printf("%-8s", "");
    for (int j = 0; j < g->nvertices; j++)
        printf("%-8s", topo->routers[j].name);
    printf("\n");

    for (int i = 0; i < g->nvertices; i++) {
        printf("%-8s", topo->routers[i].name);
        for (int j = 0; j < g->nvertices; j++) {
            if (dist[i][j] == INF)
                printf("%-8s", "∞");
            else
                printf("%-8d", dist[i][j]);
        }
        printf("\n");
    }
}
```

### Step 5: Bellman-Ford for Networks with Negative Weights

Some routing scenarios involve negative weights (e.g., cost offsets for preferred paths):

```c
void compute_routes_bellman_ford(Graph* g, int router_id, NetworkTopology* topo) {
    int dist[MAX_VERTICES], prev[MAX_VERTICES];
    shortest_path_bellman_ford(g, router_id, dist, prev);

    // Check for negative-weight cycles
    // (In routing, a negative cycle would mean infinite cost reduction = broken metric)
    for (int u = 0; u < g->nvertices; u++) {
        for (EdgeNode* e = g->adj[u]; e; e = e->next) {
            if (dist[u] != INF && dist[u] + e->weight < dist[e->to]) {
                printf("WARNING: Negative-weight cycle detected "
                       "at router %s — routing metric is invalid!\n",
                       topo->routers[u].name);
                return;
            }
        }
    }

    // Print routing table (same format as Dijkstra version)
    compute_routing_table(g, router_id, topo);  // reuse the printer
}
```

## Example: Internet-like Topology

```
    [R1: core-nyc] ——10ms—— [R2: core-lon]
         |      \                /      |
       5ms      8ms            6ms    12ms
         |         \          /        |
    [R3: edge-bos]  [R4: peer-paris]  [R5: edge-tko]
         |                               |
       3ms                             7ms
         |                               |
    [R6: cust-a]                   [R7: cust-b]
```

### Building This Topology

```c
void build_internet_topology(NetworkTopology* topo) {
    topo->n_routers = 7;
    strcpy(topo->routers[0].name, "core-nyc");  strcpy(topo->routers[0].ip, "10.0.0.1");
    strcpy(topo->routers[1].name, "core-lon");  strcpy(topo->routers[1].ip, "10.0.0.2");
    strcpy(topo->routers[2].name, "edge-bos");  strcpy(topo->routers[2].ip, "10.0.1.1");
    strcpy(topo->routers[3].name, "peer-par");  strcpy(topo->routers[3].ip, "10.0.2.1");
    strcpy(topo->routers[4].name, "edge-tko");  strcpy(topo->routers[4].ip, "10.0.3.1");
    strcpy(topo->routers[5].name, "cust-a");    strcpy(topo->routers[5].ip, "192.168.1.1");
    strcpy(topo->routers[6].name, "cust-b");    strcpy(topo->routers[6].ip, "192.168.2.1");

    Link links[] = {
        {0, 1, 10},   // nyc ↔ lon
        {0, 2, 5},    // nyc ↔ bos
        {0, 3, 8},    // nyc ↔ paris
        {1, 3, 6},    // lon ↔ paris
        {1, 4, 12},   // lon ↔ tokyo
        {2, 5, 3},    // bos ↔ cust-a
        {4, 6, 7},    // tokyo ↔ cust-b
    };
    topo->n_links = 7;
    for (int i = 0; i < topo->n_links; i++)
        topo->links[i] = links[i];
}

// Expected shortest path cust-a → cust-b:
// cust-a → edge-bos → core-nyc → core-lon → edge-tko → cust-b
// cost = 3+5+10+12+7 = 37ms
```

## Link Failure Simulation

Simulate a link going down and recompute routes:

```c
void simulate_link_failure(Graph* g, NetworkTopology* topo,
                           int failed_from, int failed_to) {
    printf("\n!!! LINK FAILURE: %s ↔ %s !!!\n",
           topo->routers[failed_from].name,
           topo->routers[failed_to].name);

    // Rebuild graph without the failed link
    graph_init(g, topo->n_routers, false);
    for (int i = 0; i < topo->n_links; i++) {
        if ((topo->links[i].from == failed_from && topo->links[i].to == failed_to) ||
            (topo->links[i].from == failed_to && topo->links[i].to == failed_from))
            continue;  // skip failed link
        graph_add_edge(g, topo->links[i].from, topo->links[i].to, topo->links[i].cost);
    }

    // Recompute all routing tables
    for (int r = 0; r < topo->n_routers; r++)
        compute_routing_table(g, r, topo);
}
```

## Load Balancing via Equal-Cost Multi-Path (ECMP)

If multiple paths have the same minimum cost, distribute traffic across them:

```c
void find_equal_cost_paths(Graph* g, int src, int dst, NetworkTopology* topo) {
    int dist[MAX_VERTICES], prev[MAX_VERTICES];
    shortest_path_dijkstra(g, src, dist, prev);

    printf("Equal-cost paths from %s to %s (cost=%d):\n",
           topo->routers[src].name, topo->routers[dst].name, dist[dst]);

    // For ECMP, we need to enumerate all shortest paths, not just one.
    // This requires a modified Dijkstra that tracks all predecessors
    // that yield the same minimum distance.
    // (Simplified: just print the primary path computed by Dijkstra)
    int path[MAX_VERTICES], plen = 0, cur = dst;
    while (cur != -1) {
        path[plen++] = cur;
        cur = prev[cur];
    }
    printf("  Primary path: ");
    for (int p = plen - 1; p >= 0; p--)
        printf("%s%s", topo->routers[path[p]].name, p > 0 ? " → " : "");
    printf("\n");
}
```

## Comparing Dijkstra vs Bellman-Ford

```c
void compare_algorithms(Graph* g, int src, NetworkTopology* topo) {
    int d_dijkstra[MAX_VERTICES], d_bellman[MAX_VERTICES];
    int p1[MAX_VERTICES], p2[MAX_VERTICES];

    shortest_path_dijkstra(g, src, d_dijkstra, p1);
    shortest_path_bellman_ford(g, src, d_bellman, p2);

    printf("\n=== Dijkstra vs Bellman-Ford (from %s) ===\n",
           topo->routers[src].name);
    printf("%-12s %12s %12s %s\n", "Dest", "Dijkstra", "Bellman-Ford", "Match");
    for (int i = 0; i < g->nvertices; i++) {
        if (i == src) continue;
        printf("%-12s %12d %12d %s\n",
               topo->routers[i].name,
               d_dijkstra[i], d_bellman[i],
               d_dijkstra[i] == d_bellman[i] ? "✓" : "✗");
    }
    printf("(Dijkstra is O(V^2) but requires non-negative weights; "
           "Bellman-Ford is O(VE) but handles negatives)\n");
}
```

## Building

```bash
cd mini-discrete-math
gcc -I include -o bin/router \
    demos/mini-router/router.c \
    src/graph.c -lm
```

## Dependencies

- `graph.h` / `graph.c` — Dijkstra, Bellman-Ford, Floyd-Warshall shortest paths
  - Also used: BFS for connectivity checks, cycle detection for topology validation

## Advanced Topics

- **OSPF Areas**: Hierarchical routing reduces Dijkstra complexity by partitioning the network.
- **BGP Path Vector**: Inter-domain routing with policy-based path selection (not shortest-path).
- **Segment Routing**: Source routing where the path is encoded in packet headers — can be computed offline using shortest paths.
- **SDN Controllers**: Centralized computation of all routes (Floyd-Warshall) in software-defined networks.
