# mini-prob-stats

MIT 18.05 + CS229 probability & statistics in C.

A self-contained, zero-dependency C library covering probability theory,
statistical inference, hypothesis testing, regression, and Monte Carlo methods,
with emphasis on practical applications in machine learning and data science.

## Modules

| Module | Header | Description |
|---|---|---|
| **Probability** | `probability.h` | Classical probability, conditional probability, Bayes' theorem, combinatorics |
| **Random Variable** | `random_variable.h` | Discrete & continuous random variables, expectation, variance, covariance |
| **Distribution** | `distribution.h` | Binomial, Poisson, Gaussian, exponential, gamma, beta, Student's t, chi-squared, F |
| **Bayes** | `bayes.h` | Bayesian inference, prior/posterior, conjugate priors, credible intervals |
| **Inference** | `inference.h` | MLE, method of moments, confidence intervals, bootstrapping |
| **Hypothesis Test** | `hypothesis_test.h` | Z-test, t-test, chi-squared test, ANOVA, p-values, type I/II error |
| **Regression** | `regression.h` | Linear regression, logistic regression, R-squared, residuals |
| **Monte Carlo** | `monte_carlo.h` | Rejection sampling, importance sampling, MCMC, Metropolis-Hastings |
| **Sampling** | `sampling.h` | Simple random sampling, stratified sampling, bootstrap, jackknife |

## Quick Reference

| Module | Key Functions |
|---|---|
| `probability.c` | `prob_union`, `prob_intersection`, `prob_conditional`, `prob_bayes`, `prob_permutations`, `prob_combinations` |
| `random_variable.c` | `rv_expectation`, `rv_variance`, `rv_covariance`, `rv_correlation`, `rv_moment`, `rv_skewness`, `rv_kurtosis` |
| `distribution.c` | `dist_binomial_pmf`, `dist_binomial_cdf`, `dist_poisson_pmf`, `dist_gaussian_pdf`, `dist_gaussian_cdf`, `dist_exponential_pdf`, `dist_gamma_pdf`, `dist_beta_pdf`, `dist_t_pdf`, `dist_chi2_pdf`, `dist_f_pdf` |
| `bayes.c` | `bayes_update`, `bayes_posterior`, `bayes_predictive`, `bayes_credible_interval`, `bayes_factor` |
| `inference.c` | `inf_mle_binomial`, `inf_mle_gaussian`, `inf_ci_mean`, `inf_ci_proportion`, `inf_ci_diff_means`, `inf_bootstrap_ci` |
| `hypothesis_test.c` | `ht_ztest`, `ht_ttest_one_sample`, `ht_ttest_two_sample`, `ht_ttest_paired`, `ht_chi2_independence`, `ht_chi2_goodness_of_fit`, `ht_anova_oneway`, `ht_pvalue` |
| `regression.c` | `reg_linear_fit`, `reg_linear_predict`, `reg_r_squared`, `reg_logistic_fit`, `reg_logistic_predict`, `reg_residuals` |
| `monte_carlo.c` | `mc_integrate`, `mc_rejection_sample`, `mc_importance_sample`, `mc_metropolis_hastings`, `mc_gibbs` |
| `sampling.c` | `samp_simple_random`, `samp_stratified`, `samp_bootstrap`, `samp_jackknife`, `samp_systematic` |

## Directory Structure

```
mini-prob-stats/
├── README.md, Makefile
├── docs/
│   ├── course-alignment.md        MIT 18.05 + CS229 module mapping
│   ├── mit-18.05-map.md           MIT 18.05 lecture-by-lecture mapping
│   ├── statistical-inference.md   MLE, CI, hypothesis testing, CLT, LLN
│   └── ml-evaluation.md           Confusion matrix, ROC, cross-validation
├── include/         9 header files
├── src/             9 implementations
├── examples/        5 demo programs
├── demos/           5 applied demos
├── tests/ benches/
└── build/
```

## Building

### Prerequisites

- C compiler (GCC, Clang, or MSVC)
- GNU Make or compatible

### Build All

```
make all
```

Compiles all source files and builds 5 example programs in `build/`.

### Run Tests

```
make test
```

Builds and runs all example programs.

### Clean

```
make clean
```

Removes the `build/` directory and all compiled artifacts.

## Key Statistical Results (all implemented)

| Result | Formula | Implementation |
|---|---|---|
| Bayes' Theorem | P(A\|B) = P(B\|A) P(A) / P(B) | `bayes.c` |
| Law of Large Numbers | X̄_n → μ as n → ∞ | `random_variable.c` |
| Central Limit Theorem | (X̄ - μ) / (σ/√n) → N(0,1) | `inference.c` |
| MLE for binomial | p̂ = k/n | `inference.c` |
| Confidence Interval (mean) | X̄ ± z_{α/2} · σ/√n | `inference.c` |
| t-test statistic | t = (X̄ - μ₀) / (s/√n) | `hypothesis_test.c` |
| R-squared | R² = 1 - SS_res / SS_tot | `regression.c` |
| Metropolis-Hastings | α = min(1, π(x')q(x\|x') / π(x)q(x'\|x)) | `monte_carlo.c` |
