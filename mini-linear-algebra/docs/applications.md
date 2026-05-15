# Applications of Linear Algebra with mini-linear-algebra

Practical applications demonstrating how the C library primitives are composed
to solve real-world problems.

---

## 1. Computer Graphics: 2D/3D Transformations

Linear transformations map points in space using matrices. Every rotation,
scale, shear, and translation is a matrix-vector multiplication.

### Rotation (2D)

```
Rotation by angle θ:
R = [ cos θ  -sin θ ]
    [ sin θ   cos θ ]

Point p = [x, y]^T  →  rotated p' = R p
```

```c
Matrix* R = mat_create(2, 2);
mat_set(R, 0, 0, cos(theta)); mat_set(R, 0, 1, -sin(theta));
mat_set(R, 1, 0, sin(theta)); mat_set(R, 1, 1,  cos(theta));

Vector* p_rotated = mat_vec_multiply(R, p);
```

### Scaling

```
S = [ sx  0  ]
    [ 0   sy ]
```

```c
Matrix* S = mat_create(2, 2);
mat_set(S, 0, 0, sx); mat_set(S, 0, 1, 0.0);
mat_set(S, 1, 0, 0.0); mat_set(S, 1, 1, sy);

Vector* p_scaled = mat_vec_multiply(S, p);
```

### Shear

```
H = [ 1  k ]
    [ 0  1 ]
```

```c
Matrix* H = mat_create(2, 2);
mat_set(H, 0, 0, 1.0); mat_set(H, 0, 1, k);
mat_set(H, 1, 0, 0.0); mat_set(H, 1, 1, 1.0);

Vector* p_sheared = mat_vec_multiply(H, p);
```

### Composition: Rotation + Scaling + Shear

Multiply transformation matrices before applying to points:
```
T = R * S * H
p' = T p
```

```c
Matrix* RS = mat_multiply(R, S);
Matrix* T  = mat_multiply(RS, H);
Vector* p_transformed = mat_vec_multiply(T, p);
```

**Demo:** `demos/mini-graphics-transform/`

---

## 2. Machine Learning: Principal Component Analysis (PCA)

PCA finds directions of maximum variance in data.

### Steps:

1. Compute mean of each feature: μ_j = (1/n) Σ_i x_ij
2. Center data: X̂ = X - 1 μ^T
3. Compute covariance matrix: Σ = (1/n) X̂^T X̂
4. Find eigenvectors of Σ (principal components)
5. Project onto top k components: Z = X̂ V_k

```c
// Mean-center
Vector* mean = mat_col_mean(X);
Matrix* X_centered = mat_subtract_mean_rows(X, mean);

// Covariance
Matrix* XT = mat_transpose(X_centered);
Matrix* cov = mat_multiply(XT, X_centered);
mat_scale_inplace(cov, 1.0 / (n - 1));

// Eigendecomposition
EigenResult* eig = eigen_qr_algorithm(cov, d);

// Project onto k principal components
Matrix* V_k = eigen_top_k_vectors(eig, k);
Matrix* Z = mat_multiply(X_centered, V_k);
// Z is now n × k — dimensionality reduced from d to k
```

**Demo:** `demos/mini-pca/`

---

## 3. PageRank: Eigenvalue Problem for Markov Matrices

PageRank models the web as a directed graph. The PageRank vector is the
stationary distribution of a random walk.

### Mathematical Formulation

1. Adjacency matrix A: A_ij = 1 if page i links to page j
2. Stochastic matrix M: M_ij = A_ij / (out-degree of page i)
3. Dangling pages: rows of zeros → replace with 1/n
4. Google matrix: G = α M + (1-α) (1/n) 1 1^T  (α = 0.85 typically)
5. PageRank vector r: r = G^T r, i.e., r is the dominant eigenvector of G^T

```c
// Build stochastic matrix from adjacency
Matrix* M = mat_create(n, n);
for (int i = 0; i < n; i++) {
    double out_deg = 0;
    for (int j = 0; j < n; j++) out_deg += A[i][j];
    for (int j = 0; j < n; j++) {
        mat_set(M, i, j, A[i][j] / out_deg);
    }
}

// Google matrix: G = α M + (1-α)/n * ones
Matrix* G = mat_scale(M, alpha);
for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
        mat_set(G, i, j, mat_get(G, i, j) + (1-alpha)/n);

// Power iteration on G^T
Matrix* GT = mat_transpose(G);
EigenPair* pr = eigen_power_iteration(GT, n);
// pr->vector is the PageRank vector
```

**Demo:** `demos/mini-page-rank/`

---

## 4. Recommender Systems: SVD Matrix Factorization

Collaborative filtering via SVD decomposes the user-item rating matrix into
latent factors.

### Mathematical Formulation

1. R (m × n): User-item rating matrix (sparse, mostly NaN)
2. SVD: R ≈ U_k Σ_k V_k^T  (rank-k approximation)
3. Predicted rating: r̂_ui = (U_k Σ_k^{1/2})_u · (Σ_k^{1/2} V_k^T)_i
4. Fill missing entries with r̂_ui
5. Recommend: for each user, items with highest predicted ratings (not already rated)

```c
// SVD decomposition
SVDResult* svd = svd_decompose(R);

// Low-rank approximation (k factors)
int k = 10;
Matrix* R_approx = svd_low_rank_approx(svd, k);

// Recommend top-N for user u
int* recommendations = recommender_top_n(R_approx, original_R, user_id, N);
// Finds highest predicted ratings among unrated items
```

### Why SVD Works

The rank-k approximation captures latent structure:
- Users with similar taste have similar row vectors in U_k
- Items with similar appeal have similar column vectors in V_k
- The inner product U_k Σ_k V_k^T predicts missing entries

**Demo:** `demos/mini-recommender/`

---

## 5. Least Squares: Curve Fitting

Fit a polynomial y = a_0 + a_1 x + a_2 x^2 + ... to data points.

### Formulation as Linear System

Given m data points (x_i, y_i) and polynomial of degree d:

```
Vandermonde matrix V (m × (d+1)):
V_ij = x_i^j   for j = 0, 1, ..., d

Unknown coefficients: c = [a_0, a_1, ..., a_d]^T

Normal equations: V^T V c = V^T y
```

```c
// Build Vandermonde matrix
Matrix* V = mat_create(m, d + 1);
for (int i = 0; i < m; i++)
    for (int j = 0; j <= d; j++)
        mat_set(V, i, j, pow(x[i], j));

// Solve normal equations
Matrix* VT = mat_transpose(V);
Matrix* VTV = mat_multiply(VT, V);
Vector* VTy = mat_vec_multiply(VT, y);
Vector* coeffs = linalg_solve(VTV, VTy);

// Evaluate polynomial at new point x_new
double y_pred = 0;
for (int j = 0; j <= d; j++)
    y_pred += coeffs->data[j] * pow(x_new, j);
```

### More Stable: Use QR

```c
QRResult* qr = decomp_qr_householder(V);
Vector* coeffs = linalg_lstsq_qr(qr, y); // solve R c = Q^T y via back sub
```

**Why QR is better than normal equations:** Normal equations square the
condition number: κ(V^T V) = κ(V)^2. For κ(V) = 10^8, normal equations have
κ = 10^16, losing all precision in double arithmetic. QR preserves κ(V).

---

## 6. Neural Networks: Weight Matrices = Linear Transformations

Each fully-connected layer is a matrix multiplication followed by bias
addition and element-wise activation.

### Single Layer Forward Pass

```
Layer l: z^(l) = W^(l) a^(l-1) + b^(l)
          a^(l) = σ(z^(l))
```

where σ is an activation function applied element-wise.

```c
// Forward pass for one fully-connected layer
// Input: a_prev (d_in × batch_size)
// Weights: W (d_out × d_in)
// Bias: b (d_out)

Vector* z = mat_vec_multiply(W, a_prev);     // z = W a_prev
vec_add_inplace(z, b);                       // z = W a_prev + b

// Activation (element-wise)
void vec_apply_relu(Vector* v) {
    for (int i = 0; i < v->size; i++)
        if (v->data[i] < 0) v->data[i] = 0;
}

void vec_apply_sigmoid(Vector* v) {
    for (int i = 0; i < v->size; i++)
        v->data[i] = 1.0 / (1.0 + exp(-v->data[i]));
}

vec_apply_relu(z);  // a = ReLU(z)
```

### Multi-Layer Network

```c
typedef struct {
    int num_layers;
    Matrix** W;       // weight matrices
    Vector** b;       // bias vectors
} NeuralNet;

Vector* forward_pass(NeuralNet* net, Vector* input) {
    Vector* a = vec_clone(input);
    for (int l = 0; l < net->num_layers; l++) {
        Vector* z = mat_vec_multiply(net->W[l], a);
        vec_add_inplace(z, net->b[l]);
        if (l < net->num_layers - 1)
            vec_apply_relu(z);      // hidden layers: ReLU
        else
            vec_apply_sigmoid(z);   // output layer: sigmoid (binary classification)
        vec_free(a);
        a = z;
    }
    return a;
}
```

**Demo:** `demos/mini-neural-layer/`

---

## 7. Bonus: Image Compression via SVD

Store a grayscale image as a matrix (rows = pixel rows, columns = pixel
columns, values = intensity 0-255).

Truncated SVD at rank k gives compressed representation:
- Original: m × n values
- Compressed: k(m + n + 1) values (U_k, Σ_k, V_k)
- Compression ratio: k(m + n + 1) / (m n)

```c
// Load image matrix (m × n)
Matrix* img = mat_load_image("photo.png");
SVDResult* svd = svd_decompose(img);

// Rank-50 approximation
Matrix* compressed = svd_low_rank_approx(svd, 50);

// Quality: ||A - A_k||_F / ||A||_F = σ_{k+1} / σ_1
```

---

## 8. Bonus: Graph Laplacian for Clustering

```
L = D - A
D = degree matrix (diagonal of row sums of A)
A = adjacency matrix (weighted or unweighted)

Fiedler vector = eigenvector corresponding to second-smallest eigenvalue of L.
Spectral clustering: use Fiedler vector to bisect the graph.
```

```c
Matrix* L = mat_graph_laplacian(A);
EigenResult* eig = eigen_qr_algorithm(L, n);
// Second-smallest eigenvalue → Fiedler vector
Vector* fiedler = eig->vectors[n - 2]; // sorted ascending
// Partition: nodes with fiedler[i] >= 0 vs fiedler[i] < 0
```
