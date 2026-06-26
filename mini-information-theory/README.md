# mini-information-theory — Information Theory (C Implementation)

> Reference: Cover & Thomas "Elements of Information Theory" 2nd Ed. + MIT 6.441
> Entropy, mutual information, KL divergence, source coding, channel capacity, error correction, rate-distortion.

## Module Status: COMPLETE ✅

- **L1 Definitions**: Complete — 30+ structs/typedefs across 8 headers
- **L2 Core Concepts**: Complete — Shannon entropy, MI, KL, cross-entropy, JS, perplexity, Huffman, BSC/BEC/Z-channel, Hamming codes, BWT, MTF
- **L3 Engineering Structures**: Complete — Arithmetic coding state machine, LZW dictionary, LZ77 sliding window, AWGN channel simulator, convolutional encoder/decoder, compression pipeline, water-filling
- **L4 Standards/Theorems**: Complete — Kraft-McMillan, Shannon source/channel coding theorems, chain rule, DPI, Pinsker, Fano, AEP, Hamming/GV/Singleton/Plotkin bounds, channel converse
- **L5 Algorithms/Methods**: Complete — Huffman, Shannon-Fano, arithmetic coding, LZ77, LZW, RLE, Elias gamma/delta/omega, Golomb/Rice, Sardinas-Patterson, Tunstall, Blahut-Arimoto (capacity + R(D)), Viterbi, Reed-Muller RM(1,m), Lloyd-Max, CRC-8/16/32
- **L6 Canonical Problems**: Complete — Full compression pipeline (BWT+MTF+RLE), BER simulation with coding, AWGN capacity, water-filling, quantizer benchmark vs R(D) bound
- **L7 Applications**: Complete (6 end-to-end demos + 3 system demos)
- **L8 Advanced Topics**: Complete — R(D) via Blahut-Arimoto, water-filling, Rayleigh fading, transfer entropy, convolutional codes, Reed-Muller, f-divergence framework
- **L9 Industry Frontiers**: Partial — documented in docs/ (5G LDPC/Polar codes, AI/ML cross-entropy, quantum IT references)

## include/ + src/ Line Count: 4550 lines

| File | Lines |
|------|-------|
| include/channel.h | 121 |
| include/coding.h | 130 |
| include/compression.h | 88 |
| include/entropy.h | 91 |
| include/error_correction.h | 124 |
| include/kl_divergence.h | 79 |
| include/mutual_information.h | 69 |
| include/rate_distortion.h | 86 |
| src/channel.c | 474 |
| src/coding.c | 837 |
| src/compression.c | 518 |
| src/entropy.c | 415 |
| src/error_correction.c | 548 |
| src/kl_divergence.c | 272 |
| src/mutual_information.c | 424 |
| src/rate_distortion.c | 274 |
| **TOTAL** | **4550** |

## Core Theorems Verified

| Theorem | Formula | Implementation |
|---------|---------|---------------|
| Chain Rule | H(X,Y) = H(Y) + H(X\|Y) | entropy.c |
| Kraft-McMillan | Σ 2^{-l_i} ≤ 1 | coding.c |
| Source Coding | L ≥ H(X) | compression.c |
| Data Processing | X→Y→Z ⇒ I(X;Z) ≤ I(X;Y) | mutual_information.c |
| Pinsker | D(P\|\|Q) ≥ L1²/(2 ln 2) | kl_divergence.c |
| Fano | Pe ≥ (H(X\|Y)-1)/log\|X\| | entropy.c |
| AEP | (-1/n)log P(xⁿ) → H(X) | entropy.c |
| BSC Capacity | C = 1 - H(p) | channel.c |
| AWGN Capacity | C = ½ log₂(1 + SNR) | channel.c |
| Water-Filling | Pi = max(0, μ - Ni) | channel.c |
| Hamming Bound | A(n,d) ≤ 2ⁿ/V(n,t) | error_correction.c |
| Rate-Distortion | R(D) = ½ log₂(σ²/D) | rate_distortion.c |

## Course Mapping (9 Schools)

| School | Course | Coverage |
|--------|--------|----------|
| MIT | 6.441 Information Theory | Full |
| Stanford | EE 276 Information Theory | BSC/BEC/AWGN, water-filling |
| Berkeley | EE 229A Information Theory | AEP, typical sets, theorems |
| CMU | 18-796 Info Theory & Coding | Conv. codes, Viterbi, bounds |
| UT Austin | EE 381V Information Theory | Adv. topics (docs) |
| ETH | 227-0417 Information Theory | Rate-distortion |
| Cambridge | 3F4 Data Transmission | Error correction, CRC |
| Tsinghua | Basics of Info Theory | Entropy, Huffman, LZW |
| Georgia Tech | ECE 6605 Information Theory | Fading, outage |

## Build & Test

```bash
make test    # 47 tests, all passing
make demos   # 6 end-to-end demos
make clean
```
