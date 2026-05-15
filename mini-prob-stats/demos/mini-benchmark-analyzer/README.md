# mini-benchmark-analyzer: Benchmark Analysis Demo

Statistical analysis of benchmark results: compute descriptive statistics,
confidence intervals, and determine if observed improvements are real.

## Overview

Benchmarking in software engineering and ML requires rigorous statistical
treatment. A single run is unreliable — we need multiple measurements,
summary statistics, confidence intervals, and hypothesis tests.

This demo covers:

1. Descriptive statistics (mean, median, variance, percentiles)
2. Confidence intervals for benchmark results
3. Statistical significance tests for comparing benchmarks
4. Effect size and practical significance
5. Handling outliers in benchmark data

## 1. Descriptive Statistics

```c
typedef struct {
    int n;              // number of measurements
    double mean;        // arithmetic mean
    double median;      // 50th percentile
    double std_dev;     // sample standard deviation
    double variance;    // sample variance
    double min, max;    // range
    double q1, q3;      // 25th, 75th percentiles
    double iqr;         // interquartile range (Q3 − Q1)
    double p5, p95;     // 5th and 95th percentiles
    double skewness;    // asymmetry measure
    double kurtosis;    // tail heaviness
} BenchmarkStats;

BenchmarkStats compute_stats(double *measurements, int n) {
    BenchmarkStats s;
    s.n = n;

    // Sort for median and percentiles
    double *sorted = malloc(n * sizeof(double));
    memcpy(sorted, measurements, n * sizeof(double));
    qsort(sorted, n, sizeof(double), compare_double);

    s.mean = mean(measurements, n);
    s.variance = variance(measurements, n);
    s.std_dev = sqrt(s.variance);
    s.min = sorted[0];
    s.max = sorted[n - 1];

    s.median = percentile_from_sorted(sorted, n, 0.50);
    s.q1 = percentile_from_sorted(sorted, n, 0.25);
    s.q3 = percentile_from_sorted(sorted, n, 0.75);
    s.iqr = s.q3 - s.q1;
    s.p5 = percentile_from_sorted(sorted, n, 0.05);
    s.p95 = percentile_from_sorted(sorted, n, 0.95);
    s.skewness = rv_skewness(measurements, n);
    s.kurtosis = rv_kurtosis(measurements, n);

    free(sorted);
    return s;
}
```

### Warm-Up and Steady-State

Benchmarks often need warm-up to reach steady state (JIT compilation, cache
warming, etc.). Discard the first few measurements:

```c
double *trim_warmup(double *measurements, int n, int warmup_runs) {
    int keep = n - warmup_runs;
    double *steady = malloc(keep * sizeof(double));
    memcpy(steady, measurements + warmup_runs, keep * sizeof(double));
    return steady;
}
```

### Outlier Detection (IQR Method)

```
Lower fence = Q1 − 1.5 · IQR
Upper fence = Q3 + 1.5 · IQR
Points outside fences are flagged as outliers.
```

```c
int *detect_outliers(double *measurements, int n, int *n_outliers) {
    BenchmarkStats s = compute_stats(measurements, n);
    double lower = s.q1 - 1.5 * s.iqr;
    double upper = s.q3 + 1.5 * s.iqr;

    int *outlier_mask = calloc(n, sizeof(int));
    *n_outliers = 0;
    for (int i = 0; i < n; i++) {
        if (measurements[i] < lower || measurements[i] > upper) {
            outlier_mask[i] = 1;
            (*n_outliers)++;
        }
    }
    return outlier_mask;
}
```

## 2. Confidence Intervals for Benchmark Results

### CI for Mean (t-distribution)

For small samples or unknown population variance:

```
CI = X̄ ± t_{α/2, n−1} · s / √n
```

```c
void benchmark_mean_ci(double *runs, int n, double alpha,
                       double *lower, double *upper) {
    double x_bar = mean(runs, n);
    double s = sample_stddev(runs, n);
    double t = t_quantile(1.0 - alpha / 2.0, n - 1);
    double margin = t * s / sqrt(n);
    *lower = x_bar - margin;
    *upper = x_bar + margin;
}
```

### CI for Median (Binomial Method)

The median CI uses order statistics. For n runs, the (1−α) CI for the
median is [X_{(k+1)}, X_{(n−k)}] where k is the largest integer such that
P(Bin(n, 0.5) ≤ k) ≤ α/2.

For large n, approximate:
```
rank_low  ≈ n/2 − z_{α/2} · √n / 2
rank_high ≈ n/2 + z_{α/2} · √n / 2
```

```c
void benchmark_median_ci(double *runs, int n, double alpha,
                          double *lower, double *upper) {
    double *sorted = malloc(n * sizeof(double));
    memcpy(sorted, runs, n * sizeof(double));
    qsort(sorted, n, sizeof(double), compare_double);

    double z = normal_quantile(1.0 - alpha / 2.0);
    int rank_low = (int)ceil(n / 2.0 - z * sqrt(n) / 2.0);
    int rank_high = (int)floor(n / 2.0 + z * sqrt(n) / 2.0);

    if (rank_low < 0) rank_low = 0;
    if (rank_high >= n) rank_high = n - 1;

    *lower = sorted[rank_low];
    *upper = sorted[rank_high];
    free(sorted);
}
```

### Bootstrap CI

Non-parametric, no distributional assumptions:

```c
void benchmark_bootstrap_ci(double *runs, int n, int B,
                             double *lower, double *upper, double alpha) {
    double *means = malloc(B * sizeof(double));
    for (int b = 0; b < B; b++) {
        double sum = 0.0;
        for (int i = 0; i < n; i++)
            sum += runs[rand() % n];
        means[b] = sum / n;
    }
    qsort(means, B, sizeof(double), compare_double);
    *lower = means[(int)(B * alpha / 2.0)];
    *upper = means[(int)(B * (1.0 - alpha / 2.0))];
    free(means);
}
```

## 3. Statistical Significance of Improvements

### Comparing Two Benchmarks: Independent Runs

When Benchmark A and B are run on the same hardware but are independent
measurements:

**Welch's t-test** (doesn't assume equal variance):
```
t = (X̄_B − X̄_A) / √(s_A²/n_A + s_B²/n_B)

df = (s_A²/n_A + s_B²/n_B)²
   / [ (s_A²/n_A)²/(n_A−1) + (s_B²/n_B)²/(n_B−1) ]

95% CI for difference: (X̄_B − X̄_A) ± t_{α/2,df} · √(s_A²/n_A + s_B²/n_B)
```

```c
int is_improvement_significant(double *runs_a, int n_a,
                               double *runs_b, int n_b,
                               double alpha, double *p_value) {
    double mean_a = mean(runs_a, n_a);
    double mean_b = mean(runs_b, n_b);
    double var_a = variance(runs_a, n_a);
    double var_b = variance(runs_b, n_b);

    double se = sqrt(var_a/n_a + var_b/n_b);
    double t_stat = (mean_b - mean_a) / se;

    double num = (var_a/n_a + var_b/n_b) * (var_a/n_a + var_b/n_b);
    double den = (var_a/n_a)*(var_a/n_a)/(n_a-1)
               + (var_b/n_b)*(var_b/n_b)/(n_b-1);
    double df = num / den;

    *p_value = 2.0 * t_cdf(-fabs(t_stat), df);
    return *p_value < alpha;
}
```

### Minimum Detectable Effect

Given n runs and observed variance, what improvement can we reliably detect?

```
δ_min = (t_{α/2,df} + t_β,df) · √(s²_A/n_A + s²_B/n_B)
```

### Practical Significance

Statistical significance ≠ practical significance. Assess effect size:

```
Cohen's d = (X̄_B − X̄_A) / s_pooled

Relative improvement = (X̄_B − X̄_A) / X̄_A × 100%

Speedup = X̄_A / X̄_B   (for runtime benchmarks)
```

```c
typedef struct {
    double mean_diff;
    double mean_diff_pct;       // relative change (%)
    double cohens_d;
    double speedup;             // >1 means B is faster
    double p_value;
    double ci95_low, ci95_high;
    int significant_95;         // 1 if p < 0.05
} BenchmarkComparison;

BenchmarkComparison compare_benchmarks(double *runs_a, int n_a,
                                       double *runs_b, int n_b) {
    BenchmarkComparison comp;
    double mean_a = mean(runs_a, n_a);
    double mean_b = mean(runs_b, n_b);

    comp.mean_diff = mean_b - mean_a;
    comp.mean_diff_pct = 100.0 * comp.mean_diff / fabs(mean_a);
    comp.speedup = mean_a / mean_b;

    // Welch's t-test
    double var_a = variance(runs_a, n_a);
    double var_b = variance(runs_b, n_b);
    double se = sqrt(var_a/n_a + var_b/n_b);
    double t_stat = comp.mean_diff / se;
    double num = (var_a/n_a + var_b/n_b) * (var_a/n_a + var_b/n_b);
    double den = (var_a/n_a)*(var_a/n_a)/(n_a-1)
               + (var_b/n_b)*(var_b/n_b)/(n_b-1);
    double df = num / den;
    comp.p_value = 2.0 * t_cdf(-fabs(t_stat), df);
    comp.significant_95 = comp.p_value < 0.05;

    // Cohen's d
    double pooled_sd = sqrt(((n_a-1)*var_a + (n_b-1)*var_b) / (n_a + n_b - 2));
    comp.cohens_d = comp.mean_diff / pooled_sd;

    // CI
    double t_crit = t_quantile(0.975, df);
    comp.ci95_low = comp.mean_diff - t_crit * se;
    comp.ci95_high = comp.mean_diff + t_crit * se;

    return comp;
}
```

## 4. Multi-Benchmark Table

When comparing multiple implementations/configurations, present a summary
table with uncertainties:

```c
void print_benchmark_table(char **names, double **runs, int *n_runs,
                            int n_benches) {
    printf("%-20s %8s %8s %8s %8s %8s\n",
           "Benchmark", "Mean", "Median", "StdDev", "CI95-", "CI95+");
    printf("──────────────────────────────────────────────────────────\n");

    for (int i = 0; i < n_benches; i++) {
        double mean_val = mean(runs[i], n_runs[i]);
        double median_val;
        double *sorted = malloc(n_runs[i] * sizeof(double));
        memcpy(sorted, runs[i], n_runs[i] * sizeof(double));
        qsort(sorted, n_runs[i], sizeof(double), compare_double);
        median_val = percentile_from_sorted(sorted, n_runs[i], 0.50);
        free(sorted);

        double sd = sample_stddev(runs[i], n_runs[i]);
        double ci_low, ci_high;
        benchmark_mean_ci(runs[i], n_runs[i], 0.05, &ci_low, &ci_high);

        printf("%-20s %8.3f %8.3f %8.3f %8.3f %8.3f\n",
               names[i], mean_val, median_val, sd, ci_low, ci_high);
    }
}
```

## Expected Output

```
=== Benchmark Analysis ===

Warm-up: 5 runs discarded

Algorithm A (n=30):
  Mean:    142.3 ms    Median: 141.8 ms
  Std Dev:   3.7 ms    Variance: 13.7 ms²
  Min:     134.1 ms    Max: 152.9 ms
  Q1:      139.5 ms    Q3: 144.2 ms
  IQR:       4.7 ms
  95% CI: [140.9, 143.7] ms

Algorithm B (n=30):
  Mean:    135.6 ms    Median: 135.2 ms
  Std Dev:   4.1 ms    Variance: 16.8 ms²
  95% CI: [134.1, 137.1] ms

Comparison: B vs A
  Mean difference: -6.7 ms
  Relative change: -4.7% (B is faster)
  Speedup: 1.049×
  Cohen's d: 1.71 (large effect)
  95% CI for difference: [-8.7, -4.7] ms
  p-value: 0.00003

  → Statistically significant improvement ✓
  → Practically significant: 4.7% speedup ✓

Outlier check:
  A: 1 outlier (152.9 ms) — retained in analysis
  B: no outliers

Multi-Benchmark Summary:
  Algorithm    Mean (ms)  Speedup vs A  p-value
  ─────────────────────────────────────────────
  A (baseline) 142.3      1.000×        —
  B            135.6      1.049×        0.00003 ***
  C            141.8      1.003×        0.612
  D            128.9      1.104×        0.00001 ***
```

## Notes

- **Minimum runs**: At least 10 runs for reasonable mean estimation, 30+
  for reliable CI and normality assumption. More runs needed for tail
  latency (p99, p99.9).
- **Warm-up**: Discard first 3–10 runs to exclude JIT compilation and cache
  warming effects. Check for steady-state visually or via change-point
  detection.
- **Environment consistency**: Control background processes, CPU frequency
  scaling, thermal throttling. Randomize benchmark order to avoid systematic
  bias from environmental drift.
- **Reporting**: Always report the number of runs, mean, standard deviation,
  and confidence intervals. Never report a single run.
- **Multiple comparisons**: If comparing k benchmarks against a baseline,
  apply Bonferroni correction: compare p-values against α/k.
