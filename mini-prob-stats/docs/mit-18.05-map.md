# MIT 18.05 Lecture Map

Lecture-by-lecture mapping from MIT 18.05 Introduction to Probability and
Statistics to implementation modules in mini-prob-stats.

---

## Readings 1–2: Probability Basics

**Topics**: Sample spaces, events, probability axioms, counting (permutations, combinations), conditional probability, Bayes' theorem, independence.

| Concept | Implementation |
|---|---|
| Sample spaces and events | `probability.c`: `prob_sample_space`, `prob_event_probability` |
| Addition rule: P(A ∪ B) = P(A) + P(B) − P(A ∩ B) | `probability.c`: `prob_union` |
| Multiplication rule: P(A ∩ B) = P(A) P(B \| A) | `probability.c`: `prob_intersection` |
| Bayes' theorem | `probability.c`: `prob_bayes` |
| Independence check: P(A ∩ B) = P(A) P(B) | `probability.c`: `prob_independent` |
| Counting: permutations nPr, combinations nCr | `probability.c`: `prob_permutations`, `prob_combinations` |

**Key Example**: Disease testing with Bayes' theorem.
```
P(D|+) = P(+|D) P(D) / P(+)
       = sensitivity × prevalence / P(+)
```

---

## Readings 3–5: Discrete Random Variables

**Topics**: Random variables, probability mass function (PMF), cumulative distribution function (CDF), expectation, variance, Bernoulli, binomial, geometric, Poisson.

| Concept | Implementation |
|---|---|
| PMF: P(X = k) | `random_variable.c`: `rv_pmf` |
| CDF: F(x) = P(X ≤ x) | `random_variable.c`: `rv_cdf` |
| Expectation: E[X] = Σ x f(x) | `random_variable.c`: `rv_expectation` |
| Variance: Var(X) = E[(X − μ)²] | `random_variable.c`: `rv_variance` |
| Linearity: E[aX + bY] = a E[X] + b E[Y] | `random_variable.c`: `rv_linearity` |
| Binomial: X ~ Bin(n, p) | `distribution.c`: `dist_binomial_pmf`, `dist_binomial_cdf` |
| Poisson: X ~ Poisson(λ) | `distribution.c`: `dist_poisson_pmf`, `dist_poisson_cdf` |
| Geometric: P(X = k) = (1−p)^(k−1) p | `distribution.c`: `dist_geometric_pmf` |

---

## Readings 6–8: Continuous Distributions

**Topics**: Probability density function (PDF), uniform, exponential, normal/Gaussian, gamma, beta, Student's t, chi-squared, F distributions.

| Concept | Implementation |
|---|---|
| PDF and CDF for continuous RVs | `distribution.c`: `dist_*_pdf`, `dist_*_cdf` |
| Uniform: X ~ U(a, b), f(x) = 1/(b−a) | `distribution.c`: `dist_uniform_pdf` |
| Exponential: X ~ Exp(λ), f(x) = λ e^(−λx) | `distribution.c`: `dist_exponential_pdf` |
| Normal: X ~ N(μ, σ²), f(x) = (1/(σ√(2π))) e^(−(x−μ)²/(2σ²)) | `distribution.c`: `dist_gaussian_pdf`, `dist_gaussian_cdf` |
| Standard normal Z ~ N(0,1) | `distribution.c`: `dist_standard_normal_pdf`, `dist_standard_normal_cdf` |
| Gamma: X ~ Gamma(α, β) | `distribution.c`: `dist_gamma_pdf` |
| Beta: X ~ Beta(α, β), conjugate prior for binomial | `distribution.c`: `dist_beta_pdf` |
| Student's t: for small-sample inference | `distribution.c`: `dist_t_pdf`, `dist_t_cdf` |
| Chi-squared: for variance tests | `distribution.c`: `dist_chi2_pdf`, `dist_chi2_cdf` |
| F distribution: for ANOVA | `distribution.c`: `dist_f_pdf`, `dist_f_cdf` |
| Transformations: Y = g(X) | `random_variable.c`: `rv_transform` |

---

## Readings 9–10: Bayesian Inference

**Topics**: Prior, likelihood, posterior, conjugate priors, Bayesian updating, credible intervals, Bayesian hypothesis testing.

| Concept | Implementation |
|---|---|
| Bayes: posterior ∝ prior × likelihood | `bayes.c`: `bayes_update` |
| Beta-Binomial conjugate | `bayes.c`: `bayes_beta_binomial_update` |
| Normal-Normal conjugate | `bayes.c`: `bayes_normal_normal_update` |
| Gamma-Poisson conjugate | `bayes.c`: `bayes_gamma_poisson_update` |
| Credible interval | `bayes.c`: `bayes_credible_interval` |
| Posterior predictive distribution | `bayes.c`: `bayes_predictive` |
| Bayes factor: BF₁₀ = P(D\|H₁)/P(D\|H₀) | `bayes.c`: `bayes_factor` |
| Prior sensitivity analysis | `bayes.c`: `bayes_sensitivity` |

**Example**: Beta-Binomial updating.
```
Prior: θ ~ Beta(α, β)
Data: k successes in n trials
Posterior: θ ~ Beta(α + k, β + n − k)
```

---

## Readings 11–12: Frequentist Inference

**Topics**: Maximum likelihood estimation (MLE), method of moments, confidence intervals, bootstrap.

| Concept | Implementation |
|---|---|
| MLE: maximize L(θ\|data) | `inference.c`: `inf_mle_binomial`, `inf_mle_gaussian`, `inf_mle_poisson` |
| MLE for binomial: p̂ = k/n | `inference.c`: `inf_mle_binomial` |
| MLE for normal: μ̂ = X̄, σ̂² = (1/n)Σ(Xᵢ−X̄)² | `inference.c`: `inf_mle_gaussian` |
| Fisher information | `inference.c`: `inf_fisher_information` |
| CI for mean (known σ): X̄ ± z_{α/2} · σ/√n | `inference.c`: `inf_ci_mean_known_sigma` |
| CI for mean (unknown σ): X̄ ± t_{α/2, n−1} · s/√n | `inference.c`: `inf_ci_mean` |
| CI for proportion: p̂ ± z_{α/2} √(p̂(1−p̂)/n) | `inference.c`: `inf_ci_proportion` |
| CI for difference of means | `inference.c`: `inf_ci_diff_means` |
| Bootstrap CI (percentile, BCa) | `inference.c`: `inf_bootstrap_ci` |

**CLT application**: For large n, the sampling distribution of X̄ is
approximately N(μ, σ²/n) regardless of the population distribution.

---

## Readings 13–14: Hypothesis Testing

**Topics**: Null and alternative hypotheses, p-values, significance level α, type I/II error, power, Z-test, t-test, chi-squared test, ANOVA.

| Concept | Implementation |
|---|---|
| H₀: null, H₁: alternative | `hypothesis_test.c`: all test functions |
| p-value: P(obs or more extreme \| H₀) | `hypothesis_test.c`: `ht_pvalue` |
| Reject H₀ if p-value < α (typically 0.05) | `hypothesis_test.c`: `ht_decide` |
| Type I error: α = P(reject H₀ \| H₀ true) | `hypothesis_test.c`: `ht_type1_error` |
| Type II error: β = P(fail to reject H₀ \| H₁ true) | `hypothesis_test.c`: `ht_type2_error` |
| Power: 1 − β | `hypothesis_test.c`: `ht_power`, `ht_sample_size` |
| Z-test (known σ) | `hypothesis_test.c`: `ht_ztest` |
| One-sample t-test: t = (X̄ − μ₀)/(s/√n) | `hypothesis_test.c`: `ht_ttest_one_sample` |
| Two-sample t-test (equal/unequal variance) | `hypothesis_test.c`: `ht_ttest_two_sample` |
| Paired t-test | `hypothesis_test.c`: `ht_ttest_paired` |
| Chi-squared test of independence | `hypothesis_test.c`: `ht_chi2_independence` |
| Chi-squared goodness-of-fit | `hypothesis_test.c`: `ht_chi2_goodness_of_fit` |
| One-way ANOVA: F = MS_between / MS_within | `hypothesis_test.c`: `ht_anova_oneway` |
| Multiple testing: Bonferroni, FDR | `hypothesis_test.c`: `ht_bonferroni_correction`, `ht_fdr` |

---

## Readings 15–16: Regression

**Topics**: Simple linear regression, least squares, R-squared, residuals, logistic regression.

| Concept | Implementation |
|---|---|
| Model: Y = β₀ + β₁ X + ε, ε ~ N(0, σ²) | `regression.c`: `reg_linear_fit` |
| OLS estimates: β̂ = (XᵀX)^(−1) XᵀY | `regression.c`: `reg_linear_fit` |
| R² = 1 − SS_res / SS_tot | `regression.c`: `reg_r_squared` |
| Residual standard error | `regression.c`: `reg_rse` |
| t-test for β = 0 | `regression.c`: `reg_coefficient_test` |
| Prediction intervals | `regression.c`: `reg_predict_interval` |
| Logistic: P(Y=1\|X) = 1/(1+e^(−(β₀+β₁X))) | `regression.c`: `reg_logistic_fit`, `reg_logistic_predict` |
| Odds ratio | `regression.c`: `reg_odds_ratio` |

---

## Readings 17–18: Bootstrap & Monte Carlo

**Topics**: Bootstrap resampling, Monte Carlo integration, MCMC.

| Concept | Implementation |
|---|---|
| Nonparametric bootstrap | `sampling.c`: `samp_bootstrap` |
| Bootstrap CI (percentile) | `inference.c`: `inf_bootstrap_ci` |
| Jackknife | `sampling.c`: `samp_jackknife` |
| Monte Carlo integration: ∫f(x)dx ≈ (1/N) Σ f(xᵢ)/g(xᵢ) | `monte_carlo.c`: `mc_integrate` |
| Rejection sampling | `monte_carlo.c`: `mc_rejection_sample` |
| Importance sampling | `monte_carlo.c`: `mc_importance_sample` |
| Metropolis-Hastings: α = min(1, r) | `monte_carlo.c`: `mc_metropolis_hastings` |
| Gibbs sampling | `monte_carlo.c`: `mc_gibbs` |
| Convergence diagnostics (trace plots, Gelman-Rubin) | `monte_carlo.c`: `mc_gelman_rubin` |
