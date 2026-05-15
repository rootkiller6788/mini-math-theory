# Mini Token Efficiency

Information-theoretic analysis of tokenization:

## Token Entropy

Given a tokenizer vocabulary V and text distribution, compute:
- H(token) = entropy of token distribution
- Perplexity per token = 2^H

## Key Metrics

| Metric | Good | Bad |
|--------|------|-----|
| H(bytes) | ~4-5 bits | ~2-3 bits |
| H(tokens) | ~10-12 bits | ~5-6 bits |
| Perplexity | Low (<20) | High (>100) |

## Token Efficiency Ratio

EFF = H(tokens) / H(bytes) × (bytes_per_token)

Better tokenizers maximize EFF:
- Each token carries more information
- Fewer tokens needed for same context
- Larger effective context window

## BPE Analysis

BPE (Byte-Pair Encoding) tokens have:
- Higher entropy per token than char-level (more information)
- Lower token count for same text (better compression)
- Subword merges → entropy per token converges to natural language entropy (~1-2 bits/char)

Reference: "Information-Theoretic Properties of Byte-Pair Tokenization" (arXiv)
