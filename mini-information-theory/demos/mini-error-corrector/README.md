# Mini Error Corrector

Hamming(7,4) code: 4 data bits → 7 encoded bits, corrects any single-bit error.

## How It Works

```
Bit positions: p1 p2 d1 p3 d2 d3 d4
                 1  2  3  4  5  6  7

p1 = d1 ⊕ d2 ⊕ d4  (covers bits 1,3,5,7)
p2 = d1 ⊕ d3 ⊕ d4  (covers bits 2,3,6,7)
p3 = d2 ⊕ d3 ⊕ d4  (covers bits 4,5,6,7)
```

## Usage

```c
bool data[4] = {1,0,1,1};
bool encoded[7];
hamming74_encode(data, encoded);

// Transmit through noisy channel...
bool noisy[7];
add_bit_errors(encoded, 7, 0.05, noisy);

// Decode and correct
bool decoded[4];
int err_pos = hamming74_decode(noisy, decoded);
// err_pos = 0: no error, 1-7: corrected, -1: uncorrectable
```

## Hamming(8,4) SEC-DED

Adds overall parity bit for Single Error Correction, Double Error Detection.

## Code Rate

R = k/n = 4/7 ≈ 0.57 (Hamming 7,4)
R = k/n = 4/8 = 0.50 (Hamming 8,4 SEC-DED)

Higher rate = less overhead but less protection.
