# Coding Theory Notes

## Source Coding

Goal: encode source symbols with minimal expected code length.

**Shannon's Source Coding Theorem**: For any source with entropy H,
there exists a uniquely decodable code with expected length L satisfying:
H ≤ L < H + 1

## Huffman Coding

Optimal prefix code construction (greedy):
1. Sort symbols by frequency
2. Merge two smallest frequencies into internal node
3. Repeat until one node remains
4. Assign 0/1 along tree paths

**Optimality**: Huffman produces minimum expected length among prefix codes.

## Kraft Inequality

For a prefix code with lengths l₁,...,lₙ:
Σ 2^(-lᵢ) ≤ 1

Conversely, any set of lengths satisfying Kraft can be realized as a prefix code.

## Coding Efficiency

η = H / L  (0 ≤ η ≤ 1)

Where H = source entropy, L = actual average code length.

## Shannon-Fano Coding

Top-down recursive splitting:
1. Sort symbols by probability
2. Split into two groups with ~equal total probability
3. Assign 0 to left group, 1 to right
4. Recurse

Slightly suboptimal compared to Huffman but simpler analysis.
