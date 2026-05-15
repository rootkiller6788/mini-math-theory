# mini-graphics-transform: 2D/3D Linear Transformations

Demonstrates basic affine transformations using matrix multiplication:
rotation, scaling, shear, and composition.

## Overview

In computer graphics, each transformation is represented by a matrix.
Applying the transformation to a point is a matrix-vector multiplication.
Multiple transformations are composed by multiplying their matrices before
applying to points, which is far more efficient than transforming
point-by-point through each step.

## Transformation Matrices

### Rotation Matrix (2D)

Rotates points counterclockwise by angle θ around the origin:

```
R(θ) = [ cos θ  -sin θ ]
       [ sin θ   cos θ ]
```

Properties:
- det(R) = cos²θ + sin²θ = 1 (area-preserving)
- R^{-1}(θ) = R(-θ) = R^T(θ) (orthogonal matrix)
- R(α) R(β) = R(α + β)

For 3D rotation around the z-axis:
```
R_z(θ) = [ cos θ  -sin θ   0 ]
         [ sin θ   cos θ   0 ]
         [   0       0     1 ]
```

### Scaling Matrix

Scales x by factor sx and y by factor sy:

```
S(sx, sy) = [ sx   0  ]
            [ 0   sy  ]
```

- sx = sy: uniform scaling (preserves aspect ratio)
- sx > 1, sy > 1: enlargement
- 0 < sx < 1: shrinkage
- sx < 0: reflection + scaling
- det(S) = sx · sy (area scaling factor)

For 3D:
```
S(sx, sy, sz) = [ sx   0    0  ]
                [ 0   sy   0  ]
                [ 0    0   sz ]
```

### Shear Matrix (2D)

Shears x proportional to y (horizontal shear):

```
H_x(k) = [ 1   k ]
         [ 0   1 ]

(x, y) → (x + ky, y)
```

Vertical shear (shears y proportional to x):
```
H_y(k) = [ 1   0 ]
         [ k   1 ]

(x, y) → (x, y + kx)
```

### Composition: Multiply Matrices

To apply rotation then scaling then shear to every point:
```
T = H · S · R
p' = T · p     (applied right-to-left: rotate first, then scale, then shear)
```

Matrix multiplication is associative but NOT commutative:
```
H · S ≠ S · H   (order matters!)
```

Correct order for graphics: apply transformations from local to world
coordinates, reading right to left in the multiplication chain.

## C Implementation

### Rotation

```c
#include <math.h>
#include "matrix.h"
#include "vector.h"

Matrix* rotation_2d(double theta) {
    Matrix* R = mat_create(2, 2);
    mat_set(R, 0, 0, cos(theta));
    mat_set(R, 0, 1, -sin(theta));
    mat_set(R, 1, 0, sin(theta));
    mat_set(R, 1, 1, cos(theta));
    return R;
}
```

### Scaling

```c
Matrix* scale_2d(double sx, double sy) {
    Matrix* S = mat_create(2, 2);
    mat_set(S, 0, 0, sx);
    mat_set(S, 0, 1, 0.0);
    mat_set(S, 1, 0, 0.0);
    mat_set(S, 1, 1, sy);
    return S;
}
```

### Shear

```c
Matrix* shear_2d(double k) {
    Matrix* H = mat_create(2, 2);
    mat_set(H, 0, 0, 1.0);
    mat_set(H, 0, 1, k);
    mat_set(H, 1, 0, 0.0);
    mat_set(H, 1, 1, 1.0);
    return H;
}
```

### Compose and Apply

```c
// Build composite transform: T = H · S · R
Matrix* R = rotation_2d(M_PI / 4);      // rotate 45°
Matrix* S = scale_2d(2.0, 0.5);          // scale x by 2, y by 0.5
Matrix* H = shear_2d(0.5);              // horizontal shear, k=0.5

Matrix* SR = mat_multiply(S, R);        // Scale then rotate
Matrix* T  = mat_multiply(H, SR);       // Shear then (Scale then rotate)
// Equivalent: T = H * (S * R)

// Apply to points
Vector* p = vec_from_array((double[]){1.0, 1.0}, 2);
Vector* p_transformed = mat_vec_multiply(T, p);

printf("Original: (1.0, 1.0)\n");
printf("Transformed: (%.4f, %.4f)\n", p_transformed->data[0], p_transformed->data[1]);

// Cleanup
mat_free(R); mat_free(S); mat_free(H); mat_free(SR); mat_free(T);
vec_free(p); vec_free(p_transformed);
```

### Transform Multiple Points at Once

For efficiency when transforming many points, package points as columns of a
matrix and multiply once:

```c
// Points matrix: each column is a point
// P = [ p1  p2  ...  pk ]  (2 × k)
Matrix* P = mat_from_points(points, num_points);
Matrix* P_transformed = mat_multiply(T, P);
// Each column of P_transformed is the transformed point
```

## 3D Extensions

### 3D Rotation Around Arbitrary Axis (Rodrigues' Formula)

```
Rotating vector v by angle θ around unit axis k:

v_rot = v cos θ + (k × v) sin θ + k (k · v) (1 - cos θ)
```

```c
Vector* rotate_3d_axis(const Vector* v, const Vector* axis, double theta) {
    double c = cos(theta), s = sin(theta);
    double dot = vec_dot(axis, v);
    Vector* cross = vec_cross3(axis, v);

    Vector* term1 = vec_scale(v, c);
    Vector* term2 = vec_scale(cross, s);
    Vector* term3 = vec_scale(axis, dot * (1.0 - c));

    Vector* tmp = vec_add(term1, term2);
    Vector* result = vec_add(tmp, term3);

    vec_free(cross); vec_free(term1); vec_free(term2); vec_free(term3); vec_free(tmp);
    return result;
}
```

## Example: Transform a Square

```
Original square corners: (0,0), (1,0), (1,1), (0,1)

Transform T = Scale(2, 2) * Rotate(45°) * Shear(0.5):

T = [1 0.5]   [2 0]   [ 0.707 -0.707]   = [ 1.414  0.0  ]
    [0  1 ] * [0 2] * [ 0.707  0.707]     [ 1.414  1.414]

Transformed corners:
(0, 0)   → (0.000,  0.000)
(1, 0)   → (1.414,  1.414)
(1, 1)   → (1.414,  2.828)
(0, 1)   → (0.000,  1.414)
```

## Expected Output

```
=== 2D Graphics Transformations ===

Original points:
  (1.00, 0.00)
  (0.00, 1.00)
  (1.00, 1.00)
  (0.00, 0.00)

After rotation (45°):
  (0.71, 0.71)
  (-0.71, 0.71)
  (0.00, 1.41)
  (0.00, 0.00)

After scaling (sx=2.0, sy=0.5):
  (1.41, 0.35)
  (-1.41, 0.35)
  (0.00, 0.71)
  (0.00, 0.00)

After shear (k=0.5):
  (1.59, 0.35)
  (-1.24, 0.35)
  (0.35, 0.71)
  (0.00, 0.00)

Composite transform T = H · S · R:
  [0.354 -0.854]
  [0.707  0.707]

Applied to original points:
  (0.35, 0.71)
  (-0.85, 0.71)
  (-0.50, 1.41)
  (0.00, 0.00)
```
