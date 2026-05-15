# Mini Dependency Resolver — Topological Sort for Build Systems

## Overview

A build dependency resolver using topological sort on partial order relations.
Models packages as elements of a poset where `A ≤ B` means "A must be processed
before B" (A is a dependency of B). The topological sort gives a valid build order.

## Use Case

Build systems (Make, Ninja, Bazel, Cargo) need to determine the order in which
to compile packages. If package B depends on package A, then A must be compiled
first. This forms a directed acyclic graph (DAG) of dependencies.

## Architecture

```
Dependency Manifest → Relation Matrix → Partial Order → Topological Sort → Build Order
```

- **Input**: A list of dependencies, e.g., `B → A`, `C → A`, `C → B`, `D → C`
- **Output**: A linear order in which to build, e.g., `[A, B, C, D]`

## Implementation Sketch

### Step 1: Define the Dependency Model

```c
#include "relation.h"
#include "graph.h"
#include <stdio.h>
#include <string.h>

#define MAX_PACKAGES 32

typedef struct {
    char name[32];
    int deps[MAX_PACKAGES];  // indices of packages this one depends on
    int ndeps;
} Package;

typedef struct {
    Package packages[MAX_PACKAGES];
    int n_packages;
} BuildManifest;
```

### Step 2: Populate the Partial Order Relation

A partial order `≤` is defined: `dep ≤ target` means "dep must be built before target."

```c
void build_dependency_relation(BuildManifest* manifest, Relation* R) {
    relation_init(R, manifest->n_packages);

    // Reflexive: every package ≤ itself
    for (int i = 0; i < manifest->n_packages; i++)
        relation_add_pair(R, i, i);

    // Transitive dependency edges: dep ≤ target
    for (int i = 0; i < manifest->n_packages; i++) {
        for (int d = 0; d < manifest->packages[i].ndeps; d++) {
            int dep_idx = manifest->packages[i].deps[d];
            relation_add_pair(R, dep_idx, i);  // dep must come before target
        }
    }

    // Compute transitive closure so the relation is a proper partial order
    *R = relation_transitive_closure(R);
}
```

### Step 3: Topological Sort to Get Build Order

```c
int* resolve_build_order(BuildManifest* manifest) {
    Relation R;
    build_dependency_relation(manifest, &R);

    if (!relation_is_partial_order(&R)) {
        printf("ERROR: Dependency graph is not a partial order (cycle detected?)\n");
        return NULL;
    }

    return topological_sort(&R);
}
```

### Step 4: Print the Build Plan

```c
void print_build_plan(BuildManifest* manifest, int* order) {
    printf("Build Order:\n");
    for (int i = 0; i < manifest->n_packages; i++) {
        int idx = order[i];
        printf("  [%d] %s", i + 1, manifest->packages[idx].name);
        if (manifest->packages[idx].ndeps > 0) {
            printf("  (depends on: ");
            for (int d = 0; d < manifest->packages[idx].ndeps; d++)
                printf("%s%s",
                       manifest->packages[manifest->packages[idx].deps[d]].name,
                       d < manifest->packages[idx].ndeps - 1 ? ", " : "");
            printf(")");
        }
        printf("\n");
    }
}
```

## Example: A Realistic Build Dependency Graph

```
Project dependencies:
  libcrypto  → (none)
  libssl     → libcrypto
  libhttp    → libssl, libcrypto
  webserver  → libhttp, libssl
  tools      → libcrypto
```

### Modeling as a Partial Order

```
Elements: {0=crypto, 1=ssl, 2=http, 3=webserver, 4=tools}
Edges (dep ≤ target):
  crypto ≤ ssl
  crypto ≤ http,  ssl ≤ http
  ssl ≤ webserver, http ≤ webserver
  crypto ≤ tools
```

Topological sort yields: `[crypto, ssl, tools, http, webserver]` (or any valid linear extension).

### Code for This Example

```c
void example_build_system(void) {
    BuildManifest manifest = { .n_packages = 5 };
    strcpy(manifest.packages[0].name, "libcrypto");
    manifest.packages[0].ndeps = 0;

    strcpy(manifest.packages[1].name, "libssl");
    manifest.packages[1].deps[0] = 0;  // depends on libcrypto
    manifest.packages[1].ndeps = 1;

    strcpy(manifest.packages[2].name, "libhttp");
    manifest.packages[2].deps[0] = 1;  // depends on libssl
    manifest.packages[2].deps[1] = 0;  // depends on libcrypto
    manifest.packages[2].ndeps = 2;

    strcpy(manifest.packages[3].name, "webserver");
    manifest.packages[3].deps[0] = 2;  // depends on libhttp
    manifest.packages[3].deps[1] = 1;  // depends on libssl
    manifest.packages[3].ndeps = 2;

    strcpy(manifest.packages[4].name, "tools");
    manifest.packages[4].deps[0] = 0;  // depends on libcrypto
    manifest.packages[4].ndeps = 1;

    int* order = resolve_build_order(&manifest);
    if (order) {
        print_build_plan(&manifest, order);
        free(order);
    }
}
```

## Cycle Detection

Before computing topological sort, verify the graph is a DAG:

```c
void check_for_cycles(BuildManifest* manifest) {
    // Use the graph module for cycle detection
    Graph g;
    graph_init(&g, manifest->n_packages, true);  // directed

    for (int i = 0; i < manifest->n_packages; i++)
        for (int d = 0; d < manifest->packages[i].ndeps; d++)
            graph_add_edge(&g, manifest->packages[i].deps[d], i, 1);

    if (has_cycle(&g)) {
        printf("CYCLE DETECTED in dependency graph!\n");

        // Find and report the cycle (using DFS parent tracking)
        // Applications: cargo, pip, npm all detect and report circular deps
    } else {
        printf("No cycles — dependency graph is a valid DAG.\n");
    }
}
```

## Alternative: Graph-Based Topological Sort

The `graph.h` module provides `topological_sort_kahn()` which is equivalent:

```c
int* resolve_with_graph(BuildManifest* manifest) {
    Graph g;
    graph_init(&g, manifest->n_packages, true);

    for (int i = 0; i < manifest->n_packages; i++)
        for (int d = 0; d < manifest->packages[i].ndeps; d++)
            graph_add_edge(&g, manifest->packages[i].deps[d], i, 1);

    if (has_cycle(&g)) return NULL;
    return topological_sort_kahn(&g);
}
```

## Parallel Build Scheduling

Given a topological order, determine which packages can be built in parallel
(packages at the same "level" with no dependencies on each other):

```c
void compute_build_levels(BuildManifest* manifest, int* order) {
    int level[MAX_PACKAGES] = {0};
    for (int i = 0; i < manifest->n_packages; i++) {
        int pkg = order[i];
        int max_dep_level = 0;
        for (int d = 0; d < manifest->packages[pkg].ndeps; d++) {
            int dep = manifest->packages[pkg].deps[d];
            if (level[dep] + 1 > max_dep_level)
                max_dep_level = level[dep] + 1;
        }
        level[pkg] = max_dep_level;
    }

    printf("\nParallel build plan:\n");
    int max_level = 0;
    for (int i = 0; i < manifest->n_packages; i++)
        if (level[i] > max_level) max_level = level[i];

    for (int lv = 0; lv <= max_level; lv++) {
        printf("  Level %d: ", lv);
        for (int i = 0; i < manifest->n_packages; i++)
            if (level[i] == lv)
                printf("%s ", manifest->packages[i].name);
        printf("\n");
    }
}
```

## Building

```bash
cd mini-discrete-math
gcc -I include -o bin/dep-resolver \
    demos/mini-dependency-resolver/dep_resolver.c \
    src/relation.c src/graph.c src/set.c -lm
```

## Dependencies

- `relation.h` / `relation.c` — Partial order relation, transitive closure, topological sort
- `graph.h` / `graph.c` — Alternative graph-based topological sort (Kahn), cycle detection
- `set.h` / `set.c` — Used by relation module for domain representation
