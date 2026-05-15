# Mini Compressor

Multi-algorithm compression demo:
- Huffman coding (optimal prefix codes)
- Run-Length Encoding (repeated sequences)
- LZW (dictionary-based adaptive)

## Usage

```c
// Huffman: optimal prefix-free coding
HuffmanTree* tree = huffman_build(freqs, 256);
Codeword codes[256]; int n = 0;
huffman_generate_codes(tree, "", 0, codes, &n);

// RLE: simple runs
CompressedData* cd = rle_encode(data, len);
unsigned char* decoded = rle_decode(cd, &dlen);

// LZW: dictionary-based
CompressedData* lzw = lzw_encode(data, len);
unsigned char* out = lzw_decode(lzw, &out_len);
```

## Information-Theoretic Guarantee

For any lossless compressor:
```
compressed_size >= H(X) * n / 8  bytes
```

where H(X) is source entropy in bits/symbol and n is input length.
The demo compares actual compression ratio against this theoretical bound.
