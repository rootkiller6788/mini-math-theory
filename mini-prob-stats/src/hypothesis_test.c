/* hypothesis_test.c — Frequentist hypothesis testing framework.
 *
 * L4: Type I / Type II error, p-values, significance levels
 * L5: t-test (one-sample, two-sample, paired, Welch)
 * L5: Z-test (proportion, difference of proportions)
 * L5: Chi-squared (independence, goodness-of-fit)
 * L5: ANOVA (one-way)
 * L7: Kolmogorov-Smirnov test, Mann-Whitney U
 *
 * Theorem sources:
 *   Casella & Berger §8 (Hypothesis Tests)
 *   MIT 18.05 §9 (Tests of Significance)
 *   Neyman & Pearson (1933)
 *   Welch (1947): unequal variance t-test
 */

#include "hypothesis_test.h"
#include "inference.h"
#include "distribution.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Helper: Standard normal CDF via erf
 * -------------------------------------------------------------------------- */
static double normal_cdf_approx(double x) {
    return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

static double compute_p_value_normal(double statistic) {
    double absz = fabs(statistic);
    return 2.0 * (1.0 - normal_cdf_approx(absz));
}

/* --------------------------------------------------------------------------
 * L5: One-sample t-test
 * H₀: μ = μ₀  vs  H₁: μ ≠ μ₀
 * t = (X̄ - μ₀) / (s/√n)  ~  t_{n-1}
 * -------------------------------------------------------------------------- */
TTestResult ttest_one_sample(double* data, int n, double mu0) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n <= 1) return res;

    double xbar = mean(data, n);
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = data[i] - xbar;
        ss += d * d;
    }
    double s = sqrt(ss / (n - 1));
    double se = s / sqrt((double)n);

    res.t_statistic = (xbar - mu0) / se;
    res.df = n - 1;
    res.p_value = compute_p_value_normal(res.t_statistic);
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Two-sample t-test (equal variance assumed)
 * H₀: μ₁ = μ₂  vs  H₁: μ₁ ≠ μ₂
 * t = (X̄₁ - X̄₂) / (s_p √(1/n₁ + 1/n₂))
 * where s²_p = ((n₁-1)s₁² + (n₂-1)s₂²) / (n₁+n₂-2)
 * -------------------------------------------------------------------------- */
TTestResult ttest_two_sample(double* d1, int n1, double* d2, int n2) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n1 <= 1 || n2 <= 1) return res;

    double xbar1 = mean(d1, n1);
    double xbar2 = mean(d2, n2);
    double ss1 = 0.0, ss2 = 0.0;
    for (int i = 0; i < n1; i++) { double d = d1[i] - xbar1; ss1 += d * d; }
    for (int i = 0; i < n2; i++) { double d = d2[i] - xbar2; ss2 += d * d; }

    double sp2 = (ss1 + ss2) / (n1 + n2 - 2);
    double se = sqrt(sp2 * (1.0 / n1 + 1.0 / n2));

    res.t_statistic = (xbar1 - xbar2) / se;
    res.df = n1 + n2 - 2;
    res.p_value = compute_p_value_normal(res.t_statistic);
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Paired t-test
 * Computes differences dᵢ = xᵢ - yᵢ, then one-sample t-test on d.
 * H₀: μ_d = 0  vs  H₁: μ_d ≠ 0
 * -------------------------------------------------------------------------- */
TTestResult ttest_paired(double* x, double* y, int n) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n <= 1 || !x || !y) return res;

    double* d = (double*)malloc(n * sizeof(double));
    if (!d) return res;

    for (int i = 0; i < n; i++) d[i] = x[i] - y[i];
    res = ttest_one_sample(d, n, 0.0);
    free(d);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Welch's t-test (unequal variances, no assumption of equal σ²)
 * Welch (1947), Satterthwaite (1946)
 * t = (X̄₁ - X̄₂) / √(s₁²/n₁ + s₂²/n₂)
 * df ≈ (s₁²/n₁ + s₂²/n₂)² / ((s₁²/n₁)²/(n₁-1) + (s₂²/n₂)²/(n₂-1))
 * -------------------------------------------------------------------------- */
TTestResult ttest_welch(double* d1, int n1, double* d2, int n2) {
    TTestResult res = {0.0, 1.0, 0.0, false};
    if (n1 <= 1 || n2 <= 1) return res;

    double xbar1 = mean(d1, n1);
    double xbar2 = mean(d2, n2);
    double v1 = variance(d1, n1) / n1;
    double v2 = variance(d2, n2) / n2;

    double se = sqrt(v1 + v2);
    if (se == 0.0) {
        res.t_statistic = 0.0;
        res.p_value = 1.0;
        return res;
    }

    res.t_statistic = (xbar1 - xbar2) / se;

    /* Welch-Satterthwaite degrees of freedom */
    double num = (v1 + v2) * (v1 + v2);
    double den = (v1 * v1) / (n1 - 1) + (v2 * v2) / (n2 - 1);
    res.df = (den > 0.0) ? (num / den) : 1.0;

    res.p_value = compute_p_value_normal(res.t_statistic);
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * Helper: Wilson-Hilferty chi-squared p-value approximation
 * (χ²/df)^{1/3} → N(1 - 2/(9df), 2/(9df))
 * -------------------------------------------------------------------------- */
static double wilson_hilferty_pvalue(double chi2, int df) {
    if (df <= 0) return 1.0;
    if (chi2 <= 0.0) return 1.0;
    double r = chi2 / df;
    double c1 = 2.0 / (9.0 * df);
    double z = (pow(r, 1.0 / 3.0) - (1.0 - c1)) / sqrt(c1);
    return 1.0 - normal_cdf_approx(z);
}

/* --------------------------------------------------------------------------
 * L5: Chi-squared test of independence (contingency table)
 * H₀: variables are independent
 * χ² = Σ_{ij} (O_{ij} - E_{ij})² / E_{ij}
 * E_{ij} = row_sumᵢ · col_sumⱼ / grand_total
 * -------------------------------------------------------------------------- */
Chi2Result chi2_test_independence(int** table, int rows, int cols) {
    Chi2Result res = {0.0, 1.0, 0, false};
    if (rows <= 0 || cols <= 0 || !table) return res;

    int* row_sum = (int*)calloc(rows, sizeof(int));
    int* col_sum = (int*)calloc(cols, sizeof(int));
    int grand = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            row_sum[i] += table[i][j];
            col_sum[j] += table[i][j];
            grand += table[i][j];
        }
    }

    if (grand == 0) {
        free(row_sum); free(col_sum);
        return res;
    }

    double chi2 = 0.0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double expected = (double)row_sum[i] * col_sum[j] / grand;
            if (expected > 0.0) {
                double diff = table[i][j] - expected;
                chi2 += (diff * diff) / expected;
            }
        }
    }

    res.df = (rows - 1) * (cols - 1);
    res.chi2_statistic = chi2;
    if (res.df > 0) {
        res.p_value = wilson_hilferty_pvalue(chi2, res.df);
    } else {
        res.p_value = 1.0;
    }
    res.reject_h0 = (res.p_value < 0.05);

    free(row_sum);
    free(col_sum);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Chi-squared goodness-of-fit test
 * H₀: data follows the expected distribution
 * χ² = Σ (Oᵢ - Eᵢ)² / Eᵢ
 *
 * observed: count in each bin
 * expected: count expected under H₀ in each bin
 * -------------------------------------------------------------------------- */
Chi2Result chi2_goodness_of_fit(int* observed, double* expected, int n_bins,
                                 int n_params_estimated) {
    Chi2Result res = {0.0, 1.0, 0, false};
    if (n_bins <= 0 || !observed || !expected) return res;

    double chi2 = 0.0;
    for (int i = 0; i < n_bins; i++) {
        if (expected[i] > 0.0) {
            double diff = observed[i] - expected[i];
            chi2 += (diff * diff) / expected[i];
        }
    }

    res.df = n_bins - 1 - n_params_estimated;
    res.chi2_statistic = chi2;
    if (res.df > 0) {
        res.p_value = wilson_hilferty_pvalue(chi2, res.df);
    } else {
        res.p_value = 1.0;
    }
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: Z-test for proportion
 * H₀: p = p₀  vs  H₁: p ≠ p₀
 * z = (p̂ - p₀) / √(p₀(1-p₀)/n)
 * -------------------------------------------------------------------------- */
ZTestResult ztest_proportion(int k, int n, double p0) {
    ZTestResult res = {0.0, 1.0, false};
    if (n <= 0) return res;

    double p_hat = (double)k / n;
    double se = sqrt(p0 * (1.0 - p0) / n);
    if (se == 0.0) {
        if (fabs(p_hat - p0) < 1e-10) {
            res.z_statistic = 0.0;
            res.p_value = 1.0;
        } else {
            res.z_statistic = (p_hat > p0) ? 1e10 : -1e10;
            res.p_value = 0.0;
        }
        res.reject_h0 = (res.p_value < 0.05);
        return res;
    }

    res.z_statistic = (p_hat - p0) / se;
    res.p_value = 2.0 * (1.0 - normal_cdf_approx(fabs(res.z_statistic)));
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L7: Z-test for difference of two proportions
 * H₀: p₁ = p₂  vs  H₁: p₁ ≠ p₂
 * Uses pooled proportion: p̂ = (k₁+k₂)/(n₁+n₂)
 * -------------------------------------------------------------------------- */
ZTestResult ztest_two_proportions(int k1, int n1, int k2, int n2) {
    ZTestResult res = {0.0, 1.0, false};
    if (n1 <= 0 || n2 <= 0) return res;

    double p1 = (double)k1 / n1;
    double p2 = (double)k2 / n2;
    double p_pool = (double)(k1 + k2) / (n1 + n2);

    double se = sqrt(p_pool * (1.0 - p_pool) * (1.0 / n1 + 1.0 / n2));
    if (se == 0.0) {
        res.z_statistic = 0.0;
        res.p_value = 1.0;
        res.reject_h0 = false;
        return res;
    }

    res.z_statistic = (p1 - p2) / se;
    res.p_value = 2.0 * (1.0 - normal_cdf_approx(fabs(res.z_statistic)));
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L5: One-way ANOVA
 * H₀: all group means are equal
 * F = MS_between / MS_within  ~  F_{k-1, N-k}
 *
 * Returns F-statistic; p-value from Wilson-Hilferty approximation.
 * Complexity: O(N) where N = total observations across k groups
 * -------------------------------------------------------------------------- */
AnovaResult anova_oneway(double** groups, int* sizes, int k) {
    AnovaResult res = {0.0, 1.0, 0, 0, false};
    if (k < 2 || !groups || !sizes) return res;

    /* Grand mean and total N */
    int N = 0;
    double grand_sum = 0.0;
    for (int g = 0; g < k; g++) {
        for (int i = 0; i < sizes[g]; i++) {
            grand_sum += groups[g][i];
            N++;
        }
    }
    if (N <= k) return res;
    double grand_mean = grand_sum / N;

    /* Between-group SS */
    double ss_between = 0.0;
    for (int g = 0; g < k; g++) {
        double group_mean = mean(groups[g], sizes[g]);
        double diff = group_mean - grand_mean;
        ss_between += sizes[g] * diff * diff;
    }

    /* Within-group SS */
    double ss_within = 0.0;
    for (int g = 0; g < k; g++) {
        double gm = mean(groups[g], sizes[g]);
        for (int i = 0; i < sizes[g]; i++) {
            double diff = groups[g][i] - gm;
            ss_within += diff * diff;
        }
    }

    res.df_between = k - 1;
    res.df_within = N - k;

    double ms_between = ss_between / res.df_between;
    double ms_within = ss_within / res.df_within;

    if (ms_within <= 0.0) {
        res.f_statistic = 1e10;
        res.p_value = 0.0;
        res.reject_h0 = true;
        return res;
    }

    res.f_statistic = ms_between / ms_within;

    /* F → χ² → normal p-value via Wilson-Hilferty */
    if (res.f_statistic > 0.0 && res.df_between > 0) {
        double chi2_equiv = res.f_statistic * res.df_between;
        res.p_value = wilson_hilferty_pvalue(chi2_equiv, res.df_between);
    } else {
        res.p_value = 1.0;
    }
    res.reject_h0 = (res.p_value < 0.05);
    return res;
}

/* --------------------------------------------------------------------------
 * L7: Kolmogorov-Smirnov two-sample test
 * Tests whether two samples come from the same continuous distribution.
 * D = max |F₁(x) - F₂(x)| (supremum of ECDF difference)
 * H₀: samples are from same distribution
 *
 * Complexity: O(n log n) for sorting, O(n) for statistic
 * -------------------------------------------------------------------------- */
double ks_test_two_sample(double* d1, int n1, double* d2, int n2, double* p_value) {
    if (n1 <= 0 || n2 <= 0 || !d1 || !d2) {
        if (p_value) *p_value = 1.0;
        return 0.0;
    }

    /* Create sorted copies */
    double* s1 = (double*)malloc(n1 * sizeof(double));
    double* s2 = (double*)malloc(n2 * sizeof(double));
    if (!s1 || !s2) { free(s1); free(s2); if (p_value) *p_value = 1.0; return 0.0; }
    memcpy(s1, d1, n1 * sizeof(double));
    memcpy(s2, d2, n2 * sizeof(double));

    /* Sort using simple bubble sort for portability (n ≤ 1000 typical) */
    for (int i = 0; i < n1 - 1; i++)
        for (int j = i + 1; j < n1; j++)
            if (s1[i] > s1[j]) { double t = s1[i]; s1[i] = s1[j]; s1[j] = t; }
    for (int i = 0; i < n2 - 1; i++)
        for (int j = i + 1; j < n2; j++)
            if (s2[i] > s2[j]) { double t = s2[i]; s2[i] = s2[j]; s2[j] = t; }

    /* Two-pointer walk for ECDF difference */
    double d_max = 0.0;
    int i = 0, j = 0;
    while (i < n1 || j < n2) {
        double val;
        if (i >= n1) val = s2[j];
        else if (j >= n2) val = s1[i];
        else val = (s1[i] < s2[j]) ? s1[i] : s2[j];

        while (i < n1 && s1[i] <= val) i++;
        while (j < n2 && s2[j] <= val) j++;

        double f1 = (double)i / n1;
        double f2 = (double)j / n2;
        double diff = fabs(f1 - f2);
        if (diff > d_max) d_max = diff;
    }

    free(s1); free(s2);

    /* Kolmogorov asymptotic p-value:
     *   p ≈ 2 Σ_{k=1}^∞ (-1)^{k-1} exp(-2k² λ²)
     * where λ = (√(n₁n₂/(n₁+n₂))) · D
     */
    double n_eff = sqrt((double)(n1 * n2) / (n1 + n2));
    double lambda = (n_eff + 0.12 + 0.11 / n_eff) * d_max;
    double p = 0.0;
    for (int k = 1; k <= 10; k++) {
        double term = exp(-2.0 * k * k * lambda * lambda);
        if (k % 2 == 0) p -= term;
        else p += term;
    }
    p *= 2.0;
    if (p > 1.0) p = 1.0;
    if (p < 0.0) p = 0.0;

    if (p_value) *p_value = p;
    return d_max;
}

/* --------------------------------------------------------------------------
 * L7: Mann-Whitney U test (Wilcoxon rank-sum)
 * Non-parametric test for difference between two independent samples.
 * H₀: distributions are equal
 *
 * Complexity: O((n₁+n₂) log(n₁+n₂)) for ranking
 * -------------------------------------------------------------------------- */
double mann_whitney_u(double* d1, int n1, double* d2, int n2, double* p_value) {
    if (n1 <= 0 || n2 <= 0) {
        if (p_value) *p_value = 1.0;
        return 0.0;
    }

    int total = n1 + n2;
    typedef struct { double val; int group; } Ranked;
    Ranked* items = (Ranked*)malloc(total * sizeof(Ranked));
    if (!items) { if (p_value) *p_value = 1.0; return 0.0; }

    for (int i = 0; i < n1; i++) { items[i].val = d1[i]; items[i].group = 0; }
    for (int i = 0; i < n2; i++) { items[n1 + i].val = d2[i]; items[n1 + i].group = 1; }

    /* Sort by value */
    for (int i = 0; i < total; i++) {
        for (int j = i + 1; j < total; j++) {
            if (items[j].val < items[i].val) {
                Ranked tmp = items[i];
                items[i] = items[j];
                items[j] = tmp;
            }
        }
    }

    /* Assign ranks (with average for ties) */
    double* ranks = (double*)malloc(total * sizeof(double));
    if (!ranks) { free(items); if (p_value) *p_value = 1.0; return 0.0; }

    int pos = 0;
    while (pos < total) {
        int end = pos + 1;
        while (end < total && items[end].val == items[pos].val) end++;
        double avg_rank = (pos + end + 1) / 2.0;
        for (int k = pos; k < end; k++) ranks[k] = avg_rank;
        pos = end;
    }

    double R1 = 0.0;
    for (int i = 0; i < total; i++) {
        if (items[i].group == 0) R1 += ranks[i];
    }

    double U1 = R1 - n1 * (n1 + 1.0) / 2.0;
    double U2 = n1 * n2 - U1;
    double U = (U1 < U2) ? U1 : U2;

    /* Normal approximation for z-score */
    double mu = n1 * n2 / 2.0;
    double sigma = sqrt(n1 * n2 * (n1 + n2 + 1.0) / 12.0);
    double z = (U - mu) / sigma;
    double p = 2.0 * (1.0 - normal_cdf_approx(fabs(z)));
    if (p > 1.0) p = 1.0;
    if (p < 0.0) p = 0.0;

    if (p_value) *p_value = p;

    free(items); free(ranks);
    return U;
}

/* --------------------------------------------------------------------------
 * L4: Type I / Type II error analysis
 *
 * Type I error (α): Reject H₀ when true  → p-value < α
 * Type II error (β): Fail to reject H₀ when false
 * Power = 1 - β
 *
 * For Z-test: Power(μ₁) = 1 - Φ(z_{α/2} - (μ₁-μ₀)/(σ/√n))
 *                         + Φ(-z_{α/2} - (μ₁-μ₀)/(σ/√n))
 * -------------------------------------------------------------------------- */
double test_power_gaussian(int n, double mu0, double mu1, double sigma, double alpha) {
    double z_alpha = gaussian_quantile(1.0 - alpha / 2.0);
    double delta = (mu1 - mu0) / (sigma / sqrt((double)n));
    if (delta > 0.0) {
        return 1.0 - normal_cdf_approx(z_alpha - delta);
    } else {
        return normal_cdf_approx(-z_alpha - delta);
    }
}

double sample_size_for_power(double mu0, double mu1, double sigma,
                              double alpha, double desired_power) {
    double z_alpha = gaussian_quantile(1.0 - alpha / 2.0);
    double z_beta = gaussian_quantile(desired_power);
    double delta = mu1 - mu0;
    if (fabs(delta) < 1e-10) return 0.0;
    double n = (z_alpha + z_beta) * (z_alpha + z_beta) * sigma * sigma
             / (delta * delta);
    return ceil(n);
}
