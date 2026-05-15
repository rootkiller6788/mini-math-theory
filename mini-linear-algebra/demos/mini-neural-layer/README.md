# mini-neural-layer: Fully-Connected Neural Network Layer

Demonstrates that a fully-connected layer is fundamentally a matrix-vector
multiplication followed by bias addition and element-wise activation.

## Overview

The core computation of a neural network layer is:

```
z = W x + b
a = σ(z)
```

where:
- **x** ∈ R^(d_in): Input vector (from previous layer or raw features)
- **W** ∈ R^(d_out × d_in): Weight matrix (learnable parameters)
- **b** ∈ R^d_out: Bias vector (learnable parameters)
- **z** ∈ R^d_out: Pre-activation output (raw linear combination)
- **σ**: Activation function applied element-wise
- **a** ∈ R^d_out: Activated output (passed to next layer)

## Why This Is Linear Algebra

The weight matrix W defines a linear transformation from R^d_in to R^d_out:

```
Each row of W is the weights connecting all inputs to one output neuron
Each column of W is the weights connecting one input to all output neurons
```

The matrix-vector product Wx computes d_out linear combinations of the d_in
inputs. This is the fundamental building block of deep learning.

### Geometric Interpretation

- **W** transforms the input space
- Each row of W projects x onto a direction in input space
- The bias b shifts the result
- The activation σ warps the result nonlinearly

Without activation functions, stacking layers would collapse to a single
linear transformation: W_2 (W_1 x) = (W_2 W_1) x, no more powerful than
one layer.

## Activation Functions

### ReLU (Rectified Linear Unit)

```
ReLU(z) = max(0, z)
```

```c
void vec_apply_relu(Vector* v) {
    for (int i = 0; i < v->size; i++)
        if (v->data[i] < 0) v->data[i] = 0.0;
}
```

Properties:
- Computationally cheap (no exponentials)
- Sparsity: negative values become 0
- Not differentiable at 0 (subgradient = 0 or 1)
- No saturation for z > 0 (vanishing gradient avoided)

### Sigmoid

```
σ(z) = 1 / (1 + e^{-z})
```

```c
void vec_apply_sigmoid(Vector* v) {
    for (int i = 0; i < v->size; i++)
        v->data[i] = 1.0 / (1.0 + exp(-v->data[i]));
}
```

Properties:
- Output range: (0, 1), interpretable as probability
- Saturates for |z| large: gradient → 0 (vanishing gradient problem)
- Historically popular for output layers (binary classification)

### Tanh

```
tanh(z) = (e^{z} - e^{-z}) / (e^{z} + e^{-z}) = 2σ(2z) - 1
```

```c
void vec_apply_tanh(Vector* v) {
    for (int i = 0; i < v->size; i++)
        v->data[i] = tanh(v->data[i]);
}
```

Properties:
- Output range: (-1, 1), zero-centered (better than sigmoid)
- Still saturates for large |z|

## Full Layer Implementation

```c
#include <math.h>
#include "matrix.h"
#include "vector.h"

typedef struct {
    Matrix* W;      // d_out × d_in
    Vector* b;      // d_out
} LinearLayer;

LinearLayer* layer_create(int d_in, int d_out) {
    LinearLayer* layer = malloc(sizeof(LinearLayer));
    layer->W = mat_random(d_out, d_in, -0.1, 0.1);  // Xavier/He init
    layer->b = vec_create(d_out);                    // zeros
    return layer;
}

Vector* layer_forward(LinearLayer* layer, const Vector* x, const char* activation) {
    // z = Wx
    Vector* z = mat_vec_multiply(layer->W, x);

    // z = Wx + b
    for (int i = 0; i < z->size; i++)
        z->data[i] += layer->b->data[i];

    // a = σ(z)
    if (strcmp(activation, "relu") == 0)
        vec_apply_relu(z);
    else if (strcmp(activation, "sigmoid") == 0)
        vec_apply_sigmoid(z);
    else if (strcmp(activation, "tanh") == 0)
        vec_apply_tanh(z);
    // "linear" → no activation

    return z;
}
```

## Multi-Layer Network

```c
typedef struct {
    int num_layers;
    LinearLayer** layers;
} MLP;

Vector* mlp_forward(MLP* net, Vector* input) {
    Vector* a = vec_clone(input);
    for (int l = 0; l < net->num_layers; l++) {
        Vector* out = layer_forward(net->layers[l], a,
            (l < net->num_layers - 1) ? "relu" : "sigmoid");
        vec_free(a);
        a = out;
    }
    return a;
}
```

## Example: XOR Problem

XOR is not linearly separable — requires at least one hidden layer.

```
Network: 2 → 4 → 1
Hidden layer: 4 neurons, ReLU activation
Output layer: 1 neuron, Sigmoid activation

Input (x1, x2) → Hidden (h1..h4) → Output (y)

Input       Desired     Predicted
(0, 0)  →     0      →   0.02
(0, 1)  →     1      →   0.98
(1, 0)  →     1      →   0.97
(1, 1)  →     0      →   0.03
```

## Batch Processing

For efficiency, process multiple inputs simultaneously by replacing vector
x with matrix X (d_in × batch_size):

```
Z = W X + b    (b is broadcast across columns)
A = σ(Z)
```

This maps to a matrix-matrix multiply, which is highly optimized (BLAS,
cache-friendly).

```c
Matrix* layer_forward_batch(LinearLayer* layer, Matrix* X, const char* activation) {
    // Z = W X  (d_out × d_in) * (d_in × batch_size) = (d_out × batch_size)
    Matrix* Z = mat_multiply(layer->W, X);

    // Add bias to each column
    for (int col = 0; col < Z->cols; col++)
        for (int row = 0; row < Z->rows; row++)
            mat_set(Z, row, col, mat_get(Z, row, col) + layer->b->data[row]);

    // Apply activation element-wise
    for (int i = 0; i < Z->rows * Z->cols; i++)
        Z->data[i] = activate(Z->data[i], activation);

    return Z;
}
```

## Expected Output

```
=== Single Neural Network Layer ===

Input vector x (3 elements):
  [1.00, 2.00, 3.00]

Weight matrix W (4x3):
  [0.50, -0.20, 0.10]
  [0.30,  0.40, -0.10]
  [-0.20, 0.10, 0.50]
  [0.10, -0.30, 0.20]

Bias vector b (4 elements):
  [0.10, 0.00, -0.05, 0.02]

Pre-activation z = Wx + b:
  [0.50, 1.40, 1.40, -0.18]

Activation (ReLU):
  [0.50, 1.40, 1.40, 0.00]

Activation (Sigmoid):
  [0.6225, 0.8022, 0.8022, 0.4551]

Activation (Tanh):
  [0.4621, 0.8854, 0.8854, -0.1781]
```
