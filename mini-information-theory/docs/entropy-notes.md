# Entropy Notes

## Shannon Entropy H(X)

H(X) = -Σ p(x) log₂ p(x)

Measures uncertainty: how many bits (on average) needed to encode an outcome.

**Properties:**
- H(X) ≥ 0
- H(X) ≤ log₂(n) for n outcomes (max at uniform)
- H(X,Y) = H(X) + H(Y|X) (chain rule)

## Joint Entropy H(X,Y)

H(X,Y) = -Σ Σ p(x,y) log₂ p(x,y)

H(X,Y) ≥ max(H(X), H(Y))
H(X,Y) ≤ H(X) + H(Y) (equality iff independent)

## Conditional Entropy H(Y|X)

H(Y|X) = Σ p(x) H(Y|X=x) = H(X,Y) - H(X)

H(Y|X) ≤ H(Y) (conditioning reduces uncertainty)

## Binary Entropy Function

h(p) = -p log₂ p - (1-p) log₂(1-p)

h(0) = h(1) = 0
h(0.5) = 1 (maximum)
Symmetric: h(p) = h(1-p)

## Entropy Rate

h̄ = lim(n→∞) H(X₁,...,Xₙ)/n

For i.i.d. sources: h̄ = H(X)
For Markov sources: h̄ = H(X₂|X₁)
