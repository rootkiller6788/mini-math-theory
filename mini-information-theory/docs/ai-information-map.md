# AI / Information Theory Bridge

Information theory concepts critical for modern ML:

## Cross-Entropy Loss

Classification loss = H(P, Q) = -Σ p(x) log q(x)

Where P = true distribution (one-hot label), Q = model prediction.
Minimizing cross-entropy = minimizing D_KL(P||Q).

**Used in**: EVERY neural network classifier (PyTorch: `CrossEntropyLoss`)

## Perplexity (LLMs)

Perplexity = 2^(cross-entropy per token)

GPT/LLaMA evaluation metric. Lower perplexity = better prediction.
- Perfect model: perplexity = 1
- Random: perplexity = vocabulary size
- GPT-4: ~5-20 depending on text

## KL Divergence in ML

- **Variational Inference**: minimize D_KL(q(z|x) || p(z))
- **Knowledge Distillation**: student mimics teacher via KL
- **GANs**: Jensen-Shannon divergence (original GAN loss)

## Mutual Information in ML

- **Feature Selection**: max I(feature; target)
- **InfoGAN**: maximize I(code; generated_image)
- **Contrastive Learning**: maximize I(x; positive) via InfoNCE loss

## Tokenizer Entropy

Token efficiency = H(tokens) — how "surprising" tokens are.
Better tokenizers → lower entropy → better compression → shorter context windows.
(BPE tokenizer design informed by entropy analysis)
