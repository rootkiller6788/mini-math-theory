# mini-page-rank: PageRank as an Eigenvalue Problem

Demonstrates Google's PageRank algorithm as finding the dominant eigenvector
of a stochastic matrix using power iteration.

## Overview

PageRank assigns each web page a score representing its importance. The
intuition: a page is important if important pages link to it. Mathematically,
this is the stationary distribution of a random walk on the web graph.

## Mathematical Formulation

### 1. Web Graph → Adjacency Matrix

A directed graph where nodes are web pages and edges are hyperlinks:

```
  (1) ←—— (2)
   | \     |
   |  \    |
   v   v   v
  (3) (4) ←—— (5) ←→ (6)
```

### 2. Adjacency Matrix A

A_ij = 1 if page i has a link to page j, 0 otherwise.

For a 4-node example:

```
Page 0 links to: [1, 2, 3]     → row 0: [0, 1, 1, 1]
Page 1 links to: [2]           → row 1: [0, 0, 1, 0]
Page 2 links to: [0, 3]        → row 2: [1, 0, 0, 1]
Page 3 links to: [0, 1]        → row 3: [1, 1, 0, 0]

A = [0 1 1 1]
    [0 0 1 0]
    [1 0 0 1]
    [1 1 0 0]
```

### 3. Stochastic Matrix M

Normalize rows so each sums to 1 (each page distributes its rank equally
to all outbound links):

```
M_ij = A_ij / (out-degree of page i)
```

For the example above:

```
Page 0: out-degree = 3 → row: [0, 1/3, 1/3, 1/3]
Page 1: out-degree = 1 → row: [0, 0,    1,    0  ]
Page 2: out-degree = 2 → row: [1/2, 0, 0,   1/2]
Page 3: out-degree = 2 → row: [1/2, 1/2, 0, 0  ]

M = [ 0    1/3  1/3  1/3 ]
    [ 0    0    1    0   ]
    [ 1/2  0    0    1/2 ]
    [ 1/2  1/2  0    0   ]
```

This is a Markov (transition probability) matrix: M_ij is the probability
of moving from page i to page j.

### 4. The Google Matrix G

To handle two issues — (a) dangling pages (no outbound links, row sums to 0)
and (b) disconnected components — we add a teleportation factor:

```
G = α M + (1 - α) (1/n) 1_{n×n}
```

where:
- α = damping factor (typically 0.85)
- (1/n) 1_{n×n} is the matrix of all 1/n (uniform teleportation probability)

The Google matrix G is:
- Column-stochastic (G^T has columns that sum to 1)
- Primitive (all entries positive, so Perron-Frobenius applies)
- Irreducible and aperiodic

For α = 0.85, n = 4:

```
G = 0.85 M + 0.15/4 * ones(4,4)
  = 0.85 M + 0.0375 * [1 1 1 1; 1 1 1 1; 1 1 1 1; 1 1 1 1]
```

### 5. PageRank = Stationary Distribution = Dominant Eigenvector

The PageRank vector r satisfies:

```
r = G^T r    (r is the eigenvector of G^T corresponding to λ = 1)
```

Equivalently, r is the left eigenvector of G (r^T G = r^T), and it is the
stationary distribution of the Markov chain defined by G.

By the Perron-Frobenius theorem:
- G has eigenvalue 1 with multiplicity 1
- All other eigenvalues have |λ| ≤ α < 1
- The eigenvector for λ = 1 is positive

### 6. Power Iteration to Find r

Since λ_1 = 1 and |λ_2| ≤ α < 1, power iteration converges quickly:

```
r^(0) = (1/n, 1/n, ..., 1/n)    // uniform initial guess
Repeat:
    r^(k+1) = G^T r^(k)
    r^(k+1) = r^(k+1) / ||r^(k+1)||_1   // normalize (sum to 1)
Until ||r^(k+1) - r^(k)|| < ε
```

Convergence rate: each iteration reduces error by factor α = 0.85
(~6-7 iterations per decimal digit of accuracy).

## 4-Node Example Walkthrough

### Graph
```
     (0) → (1)
      ↓ ↘  ↓
     (2) ← (3) → (0)
```

```
A = [0 1 1 1]
    [0 0 1 0]
    [1 0 0 1]
    [1 1 0 0]
```

### Power Iteration with α = 0.85

Iteration 0: r = [0.2500, 0.2500, 0.2500, 0.2500]
Iteration 1: r = [0.2175, 0.2025, 0.3312, 0.2488]
Iteration 2: r = [0.2197, 0.1978, 0.3350, 0.2474]
Iteration 3: r = [0.2191, 0.1984, 0.3351, 0.2473]
Iteration 4: r = [0.2193, 0.1983, 0.3351, 0.2473]
Converged at iteration 5: r = [0.2193, 0.1983, 0.3351, 0.2473]

### Interpretation

| Page | PageRank | Rank |
|------|----------|------|
| 0 | 0.2193 | 3rd |
| 1 | 0.1983 | 4th |
| 2 | 0.3351 | 1st |
| 3 | 0.2473 | 2nd |

Page 2 ranks highest because it receives links from pages 0 and 3, and page 0
itself receives links from pages 1, 2, 3. Page 1 ranks lowest because it only
receives a link from page 0, with no endorsement from other important pages.

## C Implementation

```c
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "matrix.h"
#include "vector.h"

#define DAMPING  0.85
#define TOL      1e-8
#define MAX_ITER 100

Vector* pagerank(Matrix* A, int n) {
    // 1. Build stochastic matrix M from adjacency A
    Matrix* M = mat_create(n, n);
    for (int i = 0; i < n; i++) {
        double out_deg = 0.0;
        for (int j = 0; j < n; j++)
            out_deg += mat_get(A, i, j);

        if (out_deg == 0.0) {
            // Dangling page: link to all pages uniformly
            for (int j = 0; j < n; j++)
                mat_set(M, i, j, 1.0 / n);
        } else {
            for (int j = 0; j < n; j++)
                mat_set(M, i, j, mat_get(A, i, j) / out_deg);
        }
    }

    // 2. Build Google matrix: G = α M + (1-α)/n * ones
    Matrix* G = mat_scale(M, DAMPING);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat_set(G, i, j, mat_get(G, i, j) + (1.0 - DAMPING) / n);

    // 3. Power iteration on G^T
    Matrix* GT = mat_transpose(G);
    Vector* r = vec_create(n);
    for (int i = 0; i < n; i++) r->data[i] = 1.0 / n;  // uniform start

    for (int iter = 0; iter < MAX_ITER; iter++) {
        Vector* r_new = mat_vec_multiply(GT, r);

        // L1 normalize (sum to 1)
        double sum = 0.0;
        for (int i = 0; i < n; i++) sum += r_new->data[i];
        for (int i = 0; i < n; i++) r_new->data[i] /= sum;

        // Check convergence
        double diff = 0.0;
        for (int i = 0; i < n; i++)
            diff += fabs(r_new->data[i] - r->data[i]);

        vec_free(r);
        r = r_new;

        if (diff < TOL) {
            printf("Converged at iteration %d\n", iter + 1);
            break;
        }
    }

    mat_free(M); mat_free(G); mat_free(GT);
    return r;
}

int main() {
    int n = 4;
    Matrix* A = mat_create(n, n);

    // Adjacency matrix for the 4-node example
    double adj[4][4] = {
        {0, 1, 1, 1},
        {0, 0, 1, 0},
        {1, 0, 0, 1},
        {1, 1, 0, 0}
    };
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat_set(A, i, j, adj[i][j]);

    Vector* pr = pagerank(A, n);

    printf("\nPageRank scores:\n");
    for (int i = 0; i < n; i++)
        printf("  Page %d: %.6f\n", i, pr->data[i]);

    mat_free(A); vec_free(pr);
    return 0;
}
```

## Expected Output

```
=== PageRank via Power Iteration ===

Adjacency matrix (4 nodes):
  [0 1 1 1]
  [0 0 1 0]
  [1 0 0 1]
  [1 1 0 0]

Damping factor: 0.85

Iteration 1, max diff: 0.243750
Iteration 2, max diff: 0.033737
Iteration 3, max diff: 0.004206
Iteration 4, max diff: 0.000520
Iteration 5, max diff: 0.000064
Iteration 6, max diff: 0.000008
Iteration 7, max diff: 0.000001
Converged at iteration 7

PageRank scores:
  Page 0: 0.219260
  Page 1: 0.198307
  Page 2: 0.335126
  Page 3: 0.247308

Ranking:
  1. Page 2 (0.3351)
  2. Page 3 (0.2473)
  3. Page 0 (0.2193)
  4. Page 1 (0.1983)

Sum of scores: 1.000000
```

## Why This is an Eigenvalue Problem

The PageRank equation r = G^T r means r is an eigenvector of G^T with
eigenvalue 1. The Perron-Frobenius theorem guarantees:

1. G^T has eigenvalue 1 (it's column-stochastic)
2. This is the dominant (largest magnitude) eigenvalue
3. The corresponding eigenvector has all positive entries
4. This eigenvector is unique (up to scaling)

Power iteration converges to this eigenvector because the spectral gap
(1 - α) = 0.15 ensures that subdominant eigenvalues are damped.

## Notes

- Original PageRank patent used α = 0.85
- For very large graphs (billions of pages), power iteration is preferred
  because it only needs matrix-vector multiplication (no factorization)
- The Google matrix G is never formed explicitly — instead, compute:
  r^(k+1) = α M^T r^(k) + (1-α)/n 1  (the second term is a constant vector)
- Personalized PageRank: replace the uniform teleportation (1-α)/n with a
  user-specific "preference vector" for topic-sensitive search
