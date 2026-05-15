# Course Alignment: MIT 18.05 + CS229

## Module ⬄ MIT 18.05 Mapping

| This Module | MIT 18.05 Unit | Topics |
|---|---|---|
| `probability.c` | Unit 1: Classical Probability | Sample spaces, events, axioms, combinatorics, conditional probability, Bayes' theorem, independence |
| `random_variable.c` | Unit 2: Random Variables | PMF, PDF, CDF, expectation, variance, covariance, correlation, moments, transformations |
| `distribution.c` | Units 2-3: Named Distributions | Binomial, geometric, Poisson, uniform, exponential, normal, gamma, beta, Student's t, chi-squared, F |
| `bayes.c` | Unit 4: Bayesian Statistics | Prior distributions, likelihood, posterior, conjugate priors (Beta-Binomial, Normal-Normal, Gamma-Poisson), credible intervals, Bayesian hypothesis testing, Bayes factors |
| `inference.c` | Unit 5: Frequentist Inference | Maximum likelihood estimation (MLE), method of moments, confidence intervals (mean, proportion, difference of means, variance), bootstrap confidence intervals |
| `hypothesis_test.c` | Unit 5: Hypothesis Testing | Null/alternative hypotheses, p-values, significance level (α), type I/II errors, power, Z-test, t-test (one-sample, two-sample, paired), chi-squared tests (independence, goodness-of-fit), ANOVA (one-way), multiple testing corrections |
| `regression.c` | Unit 6: Regression | Simple linear regression, least squares estimation, R-squared, residual analysis, logistic regression, odds ratios |
| `monte_carlo.c` | Unit 7: Monte Carlo Methods | Monte Carlo integration, rejection sampling, importance sampling, Markov Chain Monte Carlo (MCMC), Metropolis-Hastings algorithm, Gibbs sampling |
| `sampling.c` | Unit 8: Bootstrap & Sampling | Simple random sampling, stratified sampling, systematic sampling, bootstrap (percentile, BCa), jackknife |

## Module ⬄ CS229 Probability Review Mapping

| CS229 Topic | This Module | Key Functions |
|---|---|---|
| Probability axioms, conditional probability | `probability.c` | `prob_conditional`, `prob_bayes`, `prob_independent` |
| Random variables (discrete & continuous) | `random_variable.c` | `rv_expectation`, `rv_variance`, `rv_marginal` |
| Expectation, variance, covariance | `random_variable.c` | `rv_expectation`, `rv_variance`, `rv_covariance`, `rv_correlation` |
| Gaussian distribution & properties | `distribution.c` | `dist_gaussian_pdf`, `dist_gaussian_cdf`, `dist_multivariate_gaussian_pdf` |
| Maximum likelihood estimation | `inference.c` | `inf_mle_binomial`, `inf_mle_gaussian`, `inf_mle_poisson` |
| Naive Bayes classifier | `bayes.c` | `bayes_update`, `bayes_predict`, `bayes_naive_bayes_fit` |
| Law of Large Numbers | `random_variable.c` | `rv_law_of_large_numbers` |
| Central Limit Theorem | `inference.c` | `inf_clt_approximation` |
| Linear regression (normal equations) | `regression.c` | `reg_linear_fit`, `reg_linear_predict` |
| Logistic regression (gradient descent) | `regression.c` | `reg_logistic_fit`, `reg_logistic_predict` |

## Also Aligned With

- **Wasserman**: All of Statistics (concise reference text)
- **Casella & Berger**: Statistical Inference (graduate-level reference)
- **Gelman et al.**: Bayesian Data Analysis (Bayesian methods)
- **ISLR (James et al.)**: An Introduction to Statistical Learning (regression, resampling)

## Key Theorems Implemented

| Theorem | Implementation |
|---|---|
| Law of Large Numbers (LLN) | `random_variable.c`: `rv_lln_simulate()` — sample mean convergence |
| Central Limit Theorem (CLT) | `inference.c`: `inf_clt_approximation()` — sampling distribution → normal |
| Bayes' Theorem | `bayes.c`: `bayes_update()` — prior × likelihood ∝ posterior |
| Neyman-Pearson Lemma | `hypothesis_test.c`: `ht_likelihood_ratio_test()` |
| Gauss-Markov Theorem | `regression.c`: OLS is BLUE (Best Linear Unbiased Estimator) |
| Metropolis-Hastings | `monte_carlo.c`: `mc_metropolis_hastings()` |
