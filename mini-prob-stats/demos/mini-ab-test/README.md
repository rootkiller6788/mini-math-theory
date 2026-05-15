# mini-ab-test: A/B Test Demo

Statistical significance testing for comparing two variants (control vs
treatment) to determine which performs better.

## Overview

A/B testing (split testing) compares two versions of a product, webpage, or
algorithm by randomly assigning users to groups and measuring a key metric.
This demo performs a complete A/B test analysis:

1. Randomly assign subjects to Group A (control) and Group B (treatment)
2. Compute summary statistics for each group
3. Perform a two-sample t-test for significance
4. Construct a confidence interval for the difference in means
5. Calculate required sample size for desired power

## Statistical Framework

### Data Setup

```
Group A: n_A subjects, metric values x_1, ..., x_{n_A}  (control)
Group B: n_B subjects, metric values y_1, ..., y_{n_B}  (treatment)
```

### Summary Statistics

```
X̄_A = (1/n_A) Σ x_i          (control group mean)
X̄_B = (1/n_B) Σ y_i          (treatment group mean)
s²_A = variance of group A
s²_B = variance of group B
Δ̂ = X̄_B − X̄_A                (observed treatment effect)
```

### Two-Sample t-Test

**Null hypothesis**: H₀: μ_A = μ_B (no treatment effect)
**Alternative**: H₁: μ_A ≠ μ_B (two-sided test)

```
Pooled variance: s_p² = ((n_A−1)s_A² + (n_B−1)s_B²) / (n_A + n_B − 2)
Standard error:  SE = s_p · √(1/n_A + 1/n_B)
Test statistic:  t = (X̄_B − X̄_A) / SE
Degrees of freedom: df = n_A + n_B − 2
p-value = 2 · P(T_{df} ≥ |t|)
```

If p-value < 0.05, reject H₀ — the difference is statistically significant.

### Welch's t-Test (Unequal Variances)

When variances differ substantially (Levene's test p < 0.05):

```
SE_welch = √(s_A²/n_A + s_B²/n_B)

df_welch = (s_A²/n_A + s_B²/n_B)² /
           [ (s_A²/n_A)²/(n_A−1) + (s_B²/n_B)²/(n_B−1) ]

t_welch = (X̄_B − X̄_A) / SE_welch
```

### Confidence Interval for Difference in Means

```
CI = Δ̂ ± t_{α/2, df} · SE

Interpretation: We are 95% confident the true difference μ_B − μ_A
lies in [lower, upper].
If 0 is outside the CI, the difference is statistically significant at α = 0.05.
```

### Effect Size (Cohen's d)

```
d = (X̄_B − X̄_A) / s_p

|d| < 0.2  : negligible effect
0.2 ≤ |d| < 0.5 : small effect
0.5 ≤ |d| < 0.8 : medium effect
|d| ≥ 0.8  : large effect
```

### Sample Size Calculation

To detect a minimum effect size δ with power 1−β at significance level α:

```
n_per_group = 2 · (z_{α/2} + z_β)² · σ² / δ²
```

where:
- z_{α/2}: critical value for significance (1.96 for α = 0.05)
- z_β: critical value for power (0.84 for 80% power, 1.28 for 90%)
- σ²: estimated variance of the metric
- δ: minimum detectable effect (difference in means)

## C Usage

```c
#include "hypothesis_test.h"
#include "inference.h"
#include "sampling.h"

// Simulate or load data
int n_A = 1000, n_B = 1000;
double *group_A = malloc(n_A * sizeof(double));
double *group_B = malloc(n_B * sizeof(double));
// ... populate with metric values ...

// Summary statistics
double mean_A = mean(group_A, n_A);
double mean_B = mean(group_B, n_B);
double var_A = variance(group_A, n_A);
double var_B = variance(group_B, n_B);
double delta_hat = mean_B - mean_A;

// Two-sample t-test
double p_value = ht_ttest_two_sample(group_A, n_A, group_B, n_B, TWO_TAILED);
printf("p-value: %.4f\n", p_value);
printf("Significant: %s\n", p_value < 0.05 ? "YES" : "NO");

// Confidence interval for difference
double ci_low, ci_high;
inf_ci_diff_means(group_A, n_A, group_B, n_B, 0.05, &ci_low, &ci_high);
printf("95%% CI for difference: [%.4f, %.4f]\n", ci_low, ci_high);

// Cohen's d
double pooled_sd = sqrt(((n_A-1)*var_A + (n_B-1)*var_B) / (n_A + n_B - 2));
double cohens_d = delta_hat / pooled_sd;
printf("Cohen's d: %.4f\n", cohens_d);

// Sample size for next experiment
double min_detectable_effect = 0.1;
double sigma = sqrt((var_A + var_B) / 2.0);
int required_n = ht_sample_size(min_detectable_effect, sigma, 0.05, 0.80);
printf("Required sample size per group (80%% power): %d\n", required_n);
```

## Expected Output

```
=== A/B Test: Control vs Treatment ===
Group A (Control) : n=1000, mean=0.5123, sd=0.2834
Group B (Treatment): n=1000, mean=0.5387, sd=0.2819
Observed lift: +5.15%

Welch's t-test:
  t = 2.089, df = 1997.4
  p-value = 0.0368
  → Significant at α = 0.05 ✓

95% CI for difference: [0.0016, 0.0512]
  → The true lift is between 0.31% and 10.0%

Cohen's d = 0.093 → negligible effect size

For 80% power to detect δ=0.02 (2% lift) at α=0.05:
  Required n per group: 3,148
```

## Notes

- **Randomization is critical**: Assign users randomly to avoid confounding.
  If assignment is not random, any observed difference may be due to
  pre-existing differences between groups, not the treatment.
- **Multiple testing**: If testing many metrics or running many A/B tests
  simultaneously, apply Bonferroni correction (α / m) or control the false
  discovery rate (FDR).
- **Peeking problem**: Don't check results repeatedly and stop early when
  significant — this inflates the type I error rate. Use sequential testing
  methods (e.g., sequential probability ratio test) for continuous monitoring.
- **Practical vs statistical significance**: With large n, even tiny,
  meaningless differences become statistically significant. Always report
  effect size (Cohen's d) and confidence intervals alongside p-values.
