# mini-pca: Principal Component Analysis Demo

Reduce 2D data to 1D using PCA, with step-by-step visualization of the process.

## Overview

Given a set of 2D data points, PCA finds the direction (principal component)
along which the data varies the most. Projecting data onto this direction
gives a 1D representation that preserves the maximum possible variance.

## Mathematical Steps

### 1. Compute the Mean of Data Points

```
μ = (1/n) Σ x_i
```

The mean vector gives the centroid of the data cloud. For 2D data with n points:

```
μ_x = (1/n) Σ x_i    μ_y = (1/n) Σ y_i
```

### 2. Center the Data

```
x̂_i = x_i - μ
```

Subtract the mean from each data point so the centered data has zero mean.
This shifts the origin to the centroid, which is essential for PCA because
the principal components pass through the origin of the centered space.

### 3. Compute the Covariance Matrix

```
Σ = (1/n) Σ x̂_i x̂_i^T  (or 1/(n-1) for sample covariance)
```

For 2D data, the covariance matrix is 2×2:

```
Σ = [ σ_xx  σ_xy ]
    [ σ_xy  σ_yy ]

σ_xx = variance in x-direction (after centering)
σ_yy = variance in y-direction (after centering)
σ_xy = covariance between x and y
```

Σ is symmetric and positive semi-definite. Its diagonal entries are variances;
off-diagonal entries measure how the dimensions co-vary.

### 4. Find the Eigenvectors (Principal Components)

```
Σ v = λ v
```

Solve the eigenvalue problem for Σ (2×2 in this demo). The eigenvectors
point in the directions of maximum variance:
- **v_1** (first principal component): direction of maximum variance (λ_1 largest)
- **v_2** (second principal component): orthogonal to v_1, direction of
  remaining variance (λ_2)

These eigenvectors form an orthogonal basis for the data, rotated to align
with the natural spread of points.

### 5. Project onto the First k Components

```
Z = X̂ V_k
```

where V_k is the matrix whose columns are the top k eigenvectors (here k = 1).

For each data point:
```
z_i = v_1 · x̂_i   (scalar projection onto PC1)
```

The 2D point (x, y) becomes a 1D value z along the principal component axis.

## Variance Explained

The proportion of variance retained by k components:

```
var_explained = (λ_1 + ... + λ_k) / (λ_1 + ... + λ_d)
```

For 2D → 1D, this is λ_1 / (λ_1 + λ_2).

- If λ_1 ≈ λ_2 (circular data), PCA doesn't help much (~50% variance retained)
- If λ_1 ≫ λ_2 (elongated data), PCA works well (~99%+ variance retained)

## Example Data

```
Points: (2, 3), (3, 4), (4, 5), (5, 6), (6, 7)
Mean: (4, 5)
Center: (-2, -2), (-1, -1), (0, 0), (1, 1), (2, 2)
Covariance: [[2.5, 2.5],
             [2.5, 2.5]]
Eigenvalue 1: 5.0, eigenvector: (0.707, 0.707)^T
Eigenvalue 2: 0.0, eigenvector: (-0.707, 0.707)^T
Variance explained by PC1: 100%

Projecting onto PC1: multiply each centered point by (0.707, 0.707)^T
Results: -2.828, -1.414, 0, 1.414, 2.828
```

## C Implementation Outline

```c
// 1. Compute mean
Vector* mean = vec_create(2);
for (int i = 0; i < n; i++) {
    mean->data[0] += points[i][0];
    mean->data[1] += points[i][1];
}
vec_scale_inplace(mean, 1.0 / n);

// 2. Center data
Matrix* centered = mat_create(n, 2);
for (int i = 0; i < n; i++) {
    mat_set(centered, i, 0, points[i][0] - mean->data[0]);
    mat_set(centered, i, 1, points[i][1] - mean->data[1]);
}

// 3. Covariance matrix
Matrix* cov = mat_create(2, 2);
Matrix* XT = mat_transpose(centered);
Matrix* XTX = mat_multiply(XT, centered);
mat_scale_inplace(XTX, 1.0 / n);

// 4. Eigendecomposition of covariance
EigenResult* eig = eigen_qr_algorithm(cov, 2);

// 5. Project onto first principal component
Vector* pc1 = eig->vectors[0]; // eigenvector for largest eigenvalue
for (int i = 0; i < n; i++) {
    double projected = mat_get(centered, i, 0) * pc1->data[0]
                     + mat_get(centered, i, 1) * pc1->data[1];
    printf("Point %d projects to: %.4f\n", i, projected);
}
```

## Expected Output

```
=== PCA: 2D to 1D ===
Data points: 5
Mean: (4.0000, 5.0000)
Covariance matrix:
  [2.5000  2.5000]
  [2.5000  2.5000]
Eigenvalues: 5.0000, 0.0000
Principal component 1: (0.7071, 0.7071)
Principal component 2: (-0.7071, 0.7071)
Variance explained by PC1: 100.00%

Projections:
  (-2.0000, -2.0000) -> -2.8284
  (-1.0000, -1.0000) -> -1.4142
  ( 0.0000,  0.0000) ->  0.0000
  ( 1.0000,  1.0000) ->  1.4142
  ( 2.0000,  2.0000) ->  2.8284
```

## Notes

- This demo uses a small dataset for clarity. PCA is most useful when d is
  large (hundreds or thousands of dimensions).
- Alternative to covariance eigendecomposition: SVD of centered data matrix.
  The right singular vectors V from X̂ = U Σ V^T are identical to the
  eigenvectors of X̂^T X̂, and the singular values squared equal the eigenvalues.
  SVD is preferred numerically when the data matrix is ill-conditioned.
- For real applications, standardize features (subtract mean, divide by std
  deviation) when features have different units/scales.
