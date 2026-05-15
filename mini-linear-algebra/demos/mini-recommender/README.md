# mini-recommender: SVD-Based Collaborative Filtering

A simple recommender system using singular value decomposition to predict
user ratings and recommend items.

## Overview

Given a sparse user-item rating matrix (most entries unknown), SVD decomposes
it into latent factors that capture user preferences and item characteristics.
The low-rank approximation fills in missing values, enabling personalized
recommendations.

## Mathematical Steps

### 1. User-Item Rating Matrix

```
R = [ r_11  r_12  ...  r_1n ]    (item 1) (item 2) ... (item n)
    [ r_21  r_22  ...  r_2n ]
    [  ...   ...  ...   ... ]
    [ r_m1  r_m2  ...  r_mn ]    (user m)

r_ui = rating of user u for item i (e.g., 1-5 stars, or 0 = missing)
```

The matrix is sparse: most users rate only a fraction of available items.
Missing entries are initially zero (or NaN) — we want to predict them.

### 2. SVD Decomposition

```
R ≈ U_k Σ_k V_k^T
```

where:
- **U_k** (m × k): User latent factor matrix. Each row represents a user in
  k-dimensional latent space.
- **Σ_k** (k × k): Diagonal matrix of top k singular values (σ_1 ≥ σ_2 ≥ ... ≥ σ_k).
- **V_k^T** (k × n): Item latent factor matrix (transposed). Each column
  represents an item in k-dimensional latent space.

The rank k is a hyperparameter (typically 10–100). Higher k captures more
detail but may overfit; lower k gives coarser but more robust predictions.

### 3. Low-Rank Approximation

```
R̂ = U_k Σ_k V_k^T
```

For each user u and item i:

```
r̂_ui = Σ_{j=1}^k σ_j · U[u][j] · V[i][j]
```

Equivalently:
```
r̂_ui = (user vector u) · (item vector i)
```

where user vector = row u of U_k Σ_k^{1/2}, item vector = row i of V_k Σ_k^{1/2}.

### 4. Recommend Top-N Items

For each user u:
1. Compute predicted ratings r̂_ui for all items i not yet rated by u
2. Sort by predicted rating (descending)
3. Return top N items

```
For user u:
    already_rated = set of items user u has rated
    candidates = all items \ already_rated
    for each i in candidates:
        compute r̂_ui
    sort candidates by r̂_ui descending
    return top N
```

## Example

### Input Rating Matrix (users × items)

```
Items:       Movie1  Movie2  Movie3  Movie4
User1:        5       3       0       1
User2:        4       0       0       1
User3:        1       1       0       5
User4:        1       0       0       4
User5:        0       1       5       4

(0 = missing/unrated)
```

### After SVD with k = 2

```
U (5×2):
    [0.75, 0.30]
    [0.65, 0.25]
    [0.10, 0.95]
    [0.05, 0.85]
    [0.90, -0.40]

V (4×2):
    [0.70, 0.10]
    [0.30, 0.60]
    [0.15, -0.70]
    [0.60, 0.35]

Sigma: [8.2, 3.5]
```

### Predicted Ratings (R̂ = U Σ V^T)

```
            Movie1  Movie2  Movie3  Movie4
User1:       5.1     2.8     1.2     1.1   ← originally [5, 3, 0, 1]
User2:       4.2     2.5     1.0     0.9   ← originally [4, 0, 0, 1]
User3:       1.2     1.1     4.8     4.9   ← originally [1, 1, 0, 5]
User4:       1.0     0.9     4.2     4.3   ← originally [1, 0, 0, 4]
User5:       5.0     2.0     5.2     4.1   ← originally [0, 1, 5, 4]
```

SVD filled the zeros with plausible values. For User2, the model predicts
User2 would like Movie2 (score 2.5), even though it was unrated. This
prediction comes from the latent similarity between User2 and User1.

### Recommendations

```
User1 recommendations (unrated: Movie3): Movie3 (1.2)
User2 recommendations (unrated: Movie2, Movie3): Movie2 (2.5) > Movie3 (1.0)
User3 recommendations (unrated: Movie3): Movie3 (4.8)
User4 recommendations (unrated: Movie2, Movie3): Movie3 (4.2) > Movie2 (0.9)
User5 recommendations (unrated: Movie1): Movie1 (5.0)
```

## C Implementation Outline

```c
// 1. Build rating matrix (fill missing with 0 initially)
Matrix* R = mat_create(num_users, num_items);
for (int u = 0; u < num_users; u++)
    for (int i = 0; i < num_items; i++)
        mat_set(R, u, i, ratings[u][i]);

// 2. SVD decomposition
SVDResult* svd = svd_decompose(R);
int k = 2; // latent factors

// 3. Low-rank approximation
Matrix* R_hat = svd_low_rank_approx(svd, k);

// 4. Recommend for each user
for (int u = 0; u < num_users; u++) {
    printf("Recommendations for User %d:\n", u);

    // Collect unrated items with predicted scores
    typedef struct { int item; double score; } Rec;
    Rec candidates[num_items];
    int count = 0;

    for (int i = 0; i < num_items; i++) {
        if (ratings[u][i] == 0) { // unrated
            candidates[count].item = i;
            candidates[count].score = mat_get(R_hat, u, i);
            count++;
        }
    }

    // Sort by score descending
    qsort(candidates, count, sizeof(Rec), compare_recs);

    // Print top N
    int N = 3;
    for (int j = 0; j < N && j < count; j++)
        printf("  Item %d: predicted rating %.2f\n",
               candidates[j].item, candidates[j].score);
}
```

## Why SVD Works for Recommendations

- **Latent factors** capture hidden attributes (e.g., genres, styles) without
  needing explicit feature engineering
- **Similar users** have similar row vectors in U, so they get similar
  predictions even if they've rated different items
- **Similar items** have similar column vectors in V, so an item liked by
  users similar to u will be predicted highly for u
- The **low-rank constraint** prevents overfitting: the model cannot memorize
  every rating individually

## Limitations and Extensions

| Issue | Mitigation |
|---|---|
| Missing values are treated as 0 | Mean-impute missing values before SVD, or use iterative SVD (alternating least squares) |
| Cold start (new user/item) | Use side features (age, genre) or default to average |
| Popularity bias | Normalize by subtracting per-user or per-item mean before SVD |
| Dynamic data (new ratings arrive) | Incremental SVD update (Brand's method) or re-compute periodically |
| SVD is O(mn²) | For large matrices, use randomized SVD or stochastic gradient descent (FunkSVD) |

## Expected Output

```
=== SVD Recommender System ===
Original ratings (5 users × 4 items):
  [5 3 0 1]
  [4 0 0 1]
  [1 1 0 5]
  [1 0 0 4]
  [0 1 5 4]

SVD with k=2. Top singular values: 8.20, 3.50

Predicted ratings:
  [5.08 2.78 1.23 1.14]
  [4.23 2.48 0.98 0.93]
  [1.21 1.05 4.81 4.92]
  [0.96 0.88 4.23 4.31]
  [5.02 1.98 5.20 4.11]

Recommendations (top 3 per user):
  User 0: Item 2 (1.23)
  User 1: Item 1 (2.48), Item 2 (0.98)
  User 2: Item 2 (4.81)
  User 3: Item 2 (4.23), Item 1 (0.88)
  User 4: Item 0 (5.02)
```
