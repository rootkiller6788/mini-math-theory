# mini-information-theory — 信息论 (C 语言实现)

> 参考 MIT 6.441 Information Theory + Cover & Thomas
> 熵、互信息、KL 散度、信源编码、信道容量、纠错码——信息时代的数学底层。

## 模块与课程对照

| 模块 | MIT 6.441 章节 | CS / AI 应用 |
|------|-------------|------------|
| `entropy` | Week 1-2: Entropy, AEP | 数据压缩下限、随机数质量评估 |
| `mutual_information` | Week 2-3: Mutual Info | 特征选择、InfoGAN、InfoNCE loss |
| `kl_divergence` | Week 3-4: Divergence | Cross-entropy loss、VAE、蒸馏 |
| `coding` | Week 4-6: Source Coding | Huffman、Shannon-Fano、Kraft 不等式 |
| `compression` | Week 5-7: Compression | RLE、LZW、压缩率 vs 熵下界 |
| `channel` | Week 8-10: Channel Coding | BSC 信道仿真、Blahut-Arimoto 容量 |
| `error_correction` | Week 10-12: Error Correction | Hamming(7,4)、SEC-DED、重复码 |

## 目录

```
mini-information-theory/
├── README.md, Makefile
├── docs/
│   ├── course-alignment.md        MIT 6.441 逐章对照
│   ├── entropy-notes.md           熵的 7 条核心性质
│   ├── coding-theory-notes.md     信源编码定理 + Huffman/Shannon-Fano
│   ├── compression-notes.md       RLE/LZW + 压缩下界
│   └── ai-information-map.md      Cross-entropy/KL/Perplexity 在 AI 的应用
├── include/         7 个头文件
├── src/             7 个实现
├── examples/        6 个演示
├── demos/           4 个系统 demo
├── tests/ benches/
└── bin/
```

## 编译运行

```bash
make all    # 编译并运行全部 6 个 demo
```

## 关键信息论结果 (均已实现)

| 结果 | 公式 | 实现 |
|------|------|------|
| 熵下界 | L ≥ H(X) | `coding.c`: efficiency check |
| BSC 容量 | C = 1 - h(p) | `channel.c`: bsc_capacity() |
| Kraft 不等式 | Σ 2^(-lᵢ) ≤ 1 | `coding.c`: kraft_inequality_check() |
| 交叉熵 = H + D_KL | H(P,Q) = H(P) + D_KL(P||Q) | `kl_divergence.c` |
| 数据处理不等式 | I(X;Z) ≤ I(X;Y) | `mutual_information.c` |
