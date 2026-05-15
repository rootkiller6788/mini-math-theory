# Mini Channel Simulator

Models communication over a Binary Symmetric Channel (BSC):
- Input bit flips with probability ε
- Capacity C = 1 - H(ε) = 1 + ε log₂ ε + (1-ε) log₂(1-ε)

## Usage

```c
BinarySymmetricChannel bsc;
bsc_init(&bsc, 0.1);  // 10% error rate
double cap = bsc_capacity(&bsc);  // 0.5310 bits/channel use

bool sent[1000], received[1000];
bsc_transmit(&bsc, sent, 1000, received);
```

## Channel Coding Theorem

Reliable communication is possible at any rate R < C, where C = channel capacity.

For BSC(0.1): C ≈ 0.531, so we can transmit ~531 bits of information
reliably through 1000 channel uses, using error-correcting codes.

## Blahut-Arimoto Algorithm

General channel capacity via iterative optimization:
`channel_capacity_blahut(channel, epsilon, max_iter)`
