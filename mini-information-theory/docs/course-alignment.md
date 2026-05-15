# Course Alignment: MIT 6.441 Information Theory

## Module ⬄ Course Mapping

| This Module | MIT 6.441 Section | Topics |
|-------------|-------------------|--------|
| `entropy`       | Week 1-2: Entropy, AEP | Shannon entropy, joint/conditional entropy, entropy rate |
| `mutual_info`   | Week 2-3: Mutual Information | I(X;Y), data processing inequality, Fano's inequality |
| `kl_divergence` | Week 3-4: Divergence, Statistics | D_KL, cross-entropy, JS divergence, perplexity |
| `coding`        | Week 4-6: Source Coding | Huffman, Shannon-Fano, Kraft inequality, efficiency |
| `compression`   | Week 5-7: Lossless Compression | RLE, LZW, compression ratio, entropy bound |
| `channel`       | Week 8-10: Channel Coding | BSC, channel capacity, Blahut-Arimoto |
| `error_correction` | Week 10-12: Error Correction | Hamming(7,4), SEC-DED, repetition codes |

## Also Aligned With

- **Cover & Thomas**: Elements of Information Theory (the standard text)
- **MacKay**: Information Theory, Inference, and Learning Algorithms
- **Shannon 1948**: A Mathematical Theory of Communication

## Key Theorems

| Theorem | Implementation |
|---------|---------------|
| Shannon's Source Coding Theorem | `entropy.c`: H as lower bound |
| Kraft-McMillan Inequality | `coding.c`: kraft_inequality_check() |
| Data Processing Inequality | `mutual_information.c` |
| Channel Coding Theorem (BSC) | `channel.c`: C = 1 - H(p) |
| Singleton Bound | `error_correction.c`: Hamming code optimality |
