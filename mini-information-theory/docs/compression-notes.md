# Compression Notes

## Entropy Lower Bound

Shannon proved: the minimum average code length for lossless compression = H (bits/symbol).
Any compression below this is impossible (information-theoretically).

## Run-Length Encoding (RLE)

Simplest compression: replace repeats with (count, value) pairs.
- Good for: text with long runs (AAABBB, bitmap images)
- Bad for: random data (actually expands!)

Complexity: O(n)

## Lempel-Ziv-Welch (LZW)

Dictionary-based compression:
1. Start with single-char dictionary entries (0-255)
2. Scan input, find longest dictionary match
3. Output dictionary index
4. Add match + next char to dictionary
5. Repeat

- Used in: GIF, TIFF, Unix compress
- Complexity: O(n) with efficient lookup
- Asymptotically achieves entropy: for ergodic sources, LZ compression → H

## Compression Ratio

ratio = original_size / compressed_size
saving = 1 - compressed_size / original_size

A ratio of 2:1 means 50% space savings.

## Practical Limits

| Method | Best Case | Worst Case |
|--------|-----------|------------|
| RLE | 255:1 (all repeats) | 0.5:1 (expansion!) |
| LZW | ~4:1 typical | 1:1 (random data) |
| Huffman | ~H/8 compression | Same size |
| Theoretical | 1 - H/8 saving | 0% |
