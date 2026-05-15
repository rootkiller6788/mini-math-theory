# Statistical Inference: Core Concepts

Fundamental concepts in statistical inference with formulas and C implementations.

---

## 1. Maximum Likelihood Estimation (MLE)

**Principle**: Choose parameter values that maximize the probability of
observing the data we actually saw.

**General form**:
```
θ̂_MLE = arg max L(θ | data) = arg max ∏ f(x_i | θ)
```

In practice, maximize the log-likelihood:
```
ℓ(θ) = log L(θ) = Σ log f(x_i | θ)
```

**Binomial MLE**: Given k successes in n trials, estimate p.
```
L(p) = C(n,k) p^k (1-p)^(n-k)
ℓ(p) = k log(p) + (n-k) log(1-p) + constant
dℓ/dp = k/p - (n-k)/(1-p) = 0  →  p̂ = k/n
```

```c
double inf_mle_binomial(int k, int n) {
    return (double)k / n;
}
```

**Normal MLE**: Given observations x₁, ..., xₙ.
```
ℓ(μ, σ²) = -(n/2) log(2π) - (n/2) log(σ²) - (1/(2σ²)) Σ (x_i - μ)²

μ̂ = (1/n) Σ x_i               // sample mean
σ̂² = (1/n) Σ (x_i - μ̂)²       // biased MLE (divide by n, not n-1)
```

```c
void inf_mle_gaussian(double *x, int n, double *mu_hat, double *sigma2_hat) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += x[i];
    *mu_hat = sum / n;

    double ssq = 0.0;
    for (int i = 0; i < n; i++) ssq += (x[i] - *mu_hat) * (x[i] - *mu_hat);
    *sigma2_hat = ssq / n;
}
```

---

## 2. Confidence Intervals

A (1 − α) × 100% confidence interval gives a range of plausible values for
the unknown parameter. Across repeated sampling, (1 − α) × 100% of such
intervals will contain the true parameter value.

### CI for Mean (σ Known)

Uses the Z-distribution (Central Limit Theorem):
```
CI = X̄ ± z_{α/2} · σ / √n
```

| Confidence Level | z_{α/2} |
|---|---|
| 90% | 1.645 |
| 95% | 1.96 |
| 99% | 2.576 |

```c
void inf_ci_mean_known_sigma(double x_bar, double sigma, int n,
                              double alpha, double *lower, double *upper) {
    double z = normal_quantile(1.0 - alpha / 2.0);
    double margin = z * sigma / sqrt(n);
    *lower = x_bar - margin;
    *upper = x_bar + margin;
}
```

### CI for Mean (σ Unknown)

Uses the t-distribution with n−1 degrees of freedom:
```
CI = X̄ ± t_{α/2, n−1} · s / √n
```

```c
void inf_ci_mean(double *x, int n, double alpha, double *lower, double *upper) {
    double x_bar = mean(x, n);
    double s = sample_stddev(x, n);
    double t = t_quantile(1.0 - alpha / 2.0, n - 1);
    double margin = t * s / sqrt(n);
    *lower = x_bar - margin;
    *upper = x_bar + margin;
}
```

### CI for Proportion

For large n (np̂ ≥ 10, n(1−p̂) ≥ 10):
```
CI = p̂ ± z_{α/2} · √(p̂(1−p̂) / n)
```

```c
void inf_ci_proportion(int k, int n, double alpha,
                       double *lower, double *upper) {
    double p_hat = (double)k / n;
    double se = sqrt(p_hat * (1.0 - p_hat) / n);
    double z = normal_quantile(1.0 - alpha / 2.0);
    *lower = p_hat - z * se;
    *upper = p_hat + z * se;
}
```

### CI for Difference of Means

Two independent samples, σ₁, σ₂ unknown but assumed equal:
```
pooled_variance = ((n₁−1)s₁² + (n₂−1)s₂²) / (n₁ + n₂ − 2)
SE = √(pooled_variance · (1/n₁ + 1/n₂))
CI = (X̄₁ − X̄₂) ± t_{α/2, n₁+n₂−2} · SE
```

```c
void inf_ci_diff_means(double *x1, int n1, double *x2, int n2,
                       double alpha, double *lower, double *upper) {
    double x_bar1 = mean(x1, n1);
    double x_bar2 = mean(x2, n2);
    double s1_sq = variance(x1, n1);
    double s2_sq = variance(x2, n2);
    double df = n1 + n2 - 2;
    double pooled = ((n1-1)*s1_sq + (n2-1)*s2_sq) / df;
    double se = sqrt(pooled * (1.0/n1 + 1.0/n2));
    double t = t_quantile(1.0 - alpha/2.0, df);
    double diff = x_bar1 - x_bar2;
    *lower = diff - t * se;
    *upper = diff + t * se;
}
```

---

## 3. Hypothesis Testing

### Framework

| Component | Meaning |
|---|---|
| H₀ | Null hypothesis (status quo, no effect) |
| H₁ (or Hₐ) | Alternative hypothesis (what we want to prove) |
| Test statistic | Function of data measuring evidence against H₀ |
| p-value | P(observing test statistic as extreme or more \| H₀ true) |
| α | Significance level (typically 0.05) |
| Decision | Reject H₀ if p-value < α |

```
if (p_value < alpha) {
    printf("Reject H0: statistically significant result.\n");
} else {
    printf("Fail to reject H0: insufficient evidence.\n");
}
```

### One-Sample t-test

Tests whether the population mean equals μ₀.
```
H₀: μ = μ₀
H₁: μ ≠ μ₀ (two-sided), μ > μ₀ (right-tailed), or μ < μ₀ (left-tailed)

t = (X̄ − μ₀) / (s / √n)   ∼   t_{n−1} under H₀
```

```c
double ht_ttest_one_sample(double *x, int n, double mu0, int tail) {
    double x_bar = mean(x, n);
    double s = sample_stddev(x, n);
    double t_stat = (x_bar - mu0) / (s / sqrt(n));
    double df = n - 1;
    return t_pvalue(t_stat, df, tail);
}
```

### Two-Sample t-test

```
H₀: μ₁ = μ₂
H₁: μ₁ ≠ μ₂ (or one-sided)

t = (X̄₁ − X̄₂) / (s_p · √(1/n₁ + 1/n₂))
```

```c
double ht_ttest_two_sample(double *x1, int n1, double *x2, int n2, int tail) {
    double x_bar1 = mean(x1, n1), x_bar2 = mean(x2, n2);
    double s1_sq = variance(x1, n1), s2_sq = variance(x2, n2);
    int df = n1 + n2 - 2;
    double pooled = ((n1-1)*s1_sq + (n2-1)*s2_sq) / df;
    double se = sqrt(pooled * (1.0/n1 + 1.0/n2));
    double t_stat = (x_bar1 - x_bar2) / se;
    return t_pvalue(t_stat, df, tail);
}
```

---

## 4. Type I vs Type II Error

| | H₀ True | H₀ False |
|---|---|---|
| **Fail to reject H₀** | Correct (1−α) | Type II error (β) |
| **Reject H₀** | Type I error (α) | Correct (1−β = power) |

- **Type I error (α)**: False positive — claiming an effect when none exists.
  Controlled by setting α (typically 0.05).
- **Type II error (β)**: False negative — missing a real effect.
  Reduced by increasing sample size or effect size.
- **Power = 1 − β**: Probability of correctly detecting a real effect.

```c
double ht_power_two_sample(double delta, double sigma, int n1, int n2,
                            double alpha) {
    double se = sigma * sqrt(1.0/n1 + 1.0/n2);
    double z_alpha = normal_quantile(1.0 - alpha / 2.0);
    double z_beta = delta / se - z_alpha;
    return normal_cdf(z_beta);
}

int ht_sample_size(double delta, double sigma, double alpha, double power) {
    double z_alpha = normal_quantile(1.0 - alpha / 2.0);
    double z_beta = normal_quantile(power);
    int n = (int)ceil(2.0 * pow(sigma * (z_alpha + z_beta) / delta, 2.0));
    return n;
}
```

---

## 5. Bayesian vs Frequentist Comparison

| Aspect | Frequentist | Bayesian |
|---|---|---|
| **Probability interpretation** | Long-run frequency | Degree of belief |
| **Parameters** | Fixed, unknown constants | Random variables with distributions |
| **Inference** | Point estimates + CIs + p-values | Posterior distribution + credible intervals |
| **Prior information** | Not incorporated in likelihood | Explicitly modeled as prior distribution |
| **Interval** | CI: random interval, fixed parameter | Credible interval: fixed interval, random parameter |
| **Easy to interpret** | p-values often misinterpreted | Posterior probabilities directly intuitive |
| **Computational cost** | Low (closed-form or optimization) | Higher (integration, MCMC) |
| **Used in ML** | Cross-validation, bootstrapping | Naive Bayes, Bayesian neural nets, Gaussian processes |

### Example: Coin Flip

**Frequentist**:
```
p̂ = 7/10 = 0.7
95% CI = 0.7 ± 1.96 · √(0.7·0.3/10) = [0.416, 0.984]
p-value for H₀: p=0.5: two-sided p = 0.344 (not significant)
```

**Bayesian** (Beta(1,1) prior, i.e. uniform):
```
Posterior: θ ~ Beta(1+7, 1+3) = Beta(8, 4)
Posterior mean: 8/12 ≈ 0.667
95% credible interval: [0.391, 0.891]
```

```c
// Frequentist
double p_hat = inf_mle_binomial(7, 10);
double ci_low, ci_high;
inf_ci_proportion(7, 10, 0.05, &ci_low, &ci_high);

// Bayesian
double prior_alpha = 1.0, prior_beta = 1.0;
double post_alpha, post_beta;
bayes_beta_binomial_update(prior_alpha, prior_beta, 7, 10,
                           &post_alpha, &post_beta);
double ci_low_b, ci_high_b;
bayes_credible_interval(post_alpha, post_beta, 0.95, &ci_low_b, &ci_high_b);
```

---

## 6. Law of Large Numbers (LLN)

As sample size n → ∞, the sample mean converges to the true population mean.

```
X̄_n = (1/n) Σ X_i  →  μ   (in probability, as n → ∞)
```

**Weak LLN**: For any ε > 0, P(|X̄_n − μ| > ε) → 0 as n → ∞.
**Strong LLN**: P(lim_{n→∞} X̄_n = μ) = 1.

```c
void rv_lln_simulate(double (*generator)(void), double true_mean,
                     int n, double *sample_mean) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += generator();
        // Track convergence: after each step, error = |sum/(i+1) - true_mean|
    }
    *sample_mean = sum / n;
}
```

**Demo output**:
```
n=10:   X̄=0.432, error=0.068  (from true μ=0.5)
n=100:  X̄=0.487, error=0.013
n=1000: X̄=0.502, error=0.002
n=10000:X̄=0.5001,error=0.0001
```

---

## 7. Central Limit Theorem (CLT)

Regardless of the population distribution (with finite variance), the
sampling distribution of the sample mean approaches a normal distribution
as n → ∞.

```
(X̄ − μ) / (σ / √n)  →  N(0, 1)   as n → ∞
```

Equivalently:
```
X̄  ∼̇  N(μ, σ²/n)   for large n
```

**Practical rule**: CLT applies for n ≥ 30 (for reasonably symmetric
populations) or n larger for skewed populations.

```c
// Demonstrate CLT: take many samples of size n, compute means,
// verify that the distribution of means is approximately normal
void inf_clt_demo(double (*generator)(void), int sample_size, int num_samples) {
    double *means = malloc(num_samples * sizeof(double));
    for (int s = 0; s < num_samples; s++) {
        double sum = 0.0;
        for (int i = 0; i < sample_size; i++) sum += generator();
        means[s] = sum / sample_size;
    }

    double mu = mean(means, num_samples);
    double sigma = sample_stddev(means, num_samples);

    // Shapiro-Wilk normality test on means
    double p_value = ht_shapiro_wilk(means, num_samples);
    printf("CLT verification: p-value for normality = %.4f\n", p_value);
    printf("  (p > 0.05 → means are normally distributed → CLT holds)\n");
}
```
