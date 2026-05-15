# mini-recommender-stats: Recommender System Statistical Analysis

Statistical analysis of recommender systems: beyond accuracy — measuring
coverage, diversity, novelty, and performing rigorous A/B tests.

## Overview

Recommender systems are typically evaluated on accuracy (RMSE, precision@k,
recall@k). This demo goes further:

1. **Coverage**: What fraction of items/catalog is ever recommended?
2. **Diversity**: How different are recommendations across users?
3. **Novelty**: How surprising/unexpected are the recommendations?
4. **A/B testing**: Is a new recommender actually better?
5. **Bootstrap confidence intervals**: Quantifying uncertainty in metrics

## 1. Coverage

**Item coverage**: Fraction of the catalog that appears in at least one
user's top-k recommendations.

```
Coverage(@k) = |unique items in top-k across all users| / |total catalog|

High coverage → system utilizes more of the catalog, not just popular items.
Low coverage → potential filter bubble / popularity bias.
```

```c
double item_coverage(int **recommendations, int n_users, int k,
                     int n_items) {
    int *covered = calloc(n_items, sizeof(int));
    for (int u = 0; u < n_users; u++) {
        for (int i = 0; i < k; i++) {
            covered[recommendations[u][i]] = 1;
        }
    }
    int total = 0;
    for (int i = 0; i < n_items; i++) total += covered[i];
    free(covered);
    return (double)total / n_items;
}
```

**User coverage**: Fraction of users who receive at least one "useful"
recommendation (e.g., above a relevance threshold).

## 2. Diversity

**Intra-list diversity**: Within a user's recommendation list, how
dissimilar are the items? Measured via average pairwise distance.

```
For user u with top-k list L_u = {i₁, i₂, ..., iₖ}:

Diversity_u = (1 / (k(k−1))) · Σ_{a≠b ∈ L_u} (1 − similarity(i_a, i_b))
```

Overall diversity is the mean across all users.

```c
double intra_list_diversity(int *recs, int k,
                            double (*distance)(int, int)) {
    double sum = 0.0;
    int pairs = 0;
    for (int a = 0; a < k; a++) {
        for (int b = a + 1; b < k; b++) {
            sum += 1.0 - distance(recs[a], recs[b]);
            pairs++;
        }
    }
    return sum / pairs;
}

double average_diversity(int **recommendations, int n_users, int k,
                          double (*distance)(int, int)) {
    double total = 0.0;
    for (int u = 0; u < n_users; u++)
        total += intra_list_diversity(recommendations[u], k, distance);
    return total / n_users;
}
```

**Inter-list diversity**: How different are recommendation lists across
different users? Higher means more personalization.

```
For users u, v, with lists L_u, L_v:

InterDiversity = (2 / (n(n−1))) · Σ_{u<v} (1 − Jaccard(L_u, L_v))

where Jaccard(A, B) = |A ∩ B| / |A ∪ B|
```

## 3. Novelty

Novelty measures how unexpected recommendations are. Two common approaches:

**Popularity-based novelty**:
```
Novelty_u = (1/k) Σ_{i ∈ L_u} −log₂(popularity(i) / total_interactions)

High novelty → recommending less popular/long-tail items
Low novelty → mostly recommending blockbusters
```

**Self-information novelty**:
```
SelfInfo(i) = −log₂ P(i) = −log₂(#interactions_with_i / total_interactions)

Average novelty = (1/(n·k)) Σ_u Σ_{i ∈ L_u} SelfInfo(i)
```

```c
double self_information_novelty(int **recommendations, int n_users, int k,
                                 int *item_interaction_counts,
                                 int total_interactions) {
    double total = 0.0;
    for (int u = 0; u < n_users; u++) {
        for (int i = 0; i < k; i++) {
            int item = recommendations[u][i];
            double p = (double)item_interaction_counts[item] / total_interactions;
            if (p > 0) total += -log2(p);
        }
    }
    return total / (n_users * k);
}
```

## 4. A/B Testing Recommender Changes

When deploying a new recommendation algorithm, use an A/B test to verify
improvement on business metrics.

### Metrics to Test

| Metric | Type | Test |
|---|---|---|
| CTR (click-through rate) | Proportion | Two-sample Z-test for proportions |
| Average session duration | Continuous | Two-sample t-test |
| Conversion rate | Proportion | Two-sample Z-test for proportions |
| Revenue per user | Continuous | Two-sample t-test (or Mann-Whitney if skewed) |

### Two-Sample Z-Test for Proportions

For CTR, conversion rate, etc.:
```
p̂_A = clicks_A / impressions_A
p̂_B = clicks_B / impressions_B
p̂_pool = (clicks_A + clicks_B) / (impressions_A + impressions_B)

SE = √(p̂_pool · (1−p̂_pool) · (1/n_A + 1/n_B))
Z = (p̂_B − p̂_A) / SE
p-value = 2 · Φ(−|Z|)
```

### Power Analysis

Determine required traffic before launching:
```
Required impressions per group =
    (z_{α/2} + z_β)² · 2 · p̂_pool(1−p̂_pool) / δ²

where δ = minimum detectable effect (e.g., 1% absolute CTR lift)
```

```c
typedef struct {
    int impressions_a, clicks_a;
    int impressions_b, clicks_b;
} ABTestData;

double ab_test_proportions(ABTestData data, double *ci_low, double *ci_high) {
    double p_a = (double)data.clicks_a / data.impressions_a;
    double p_b = (double)data.clicks_b / data.impressions_b;
    double p_pool = (double)(data.clicks_a + data.clicks_b)
                  / (data.impressions_a + data.impressions_b);
    double se = sqrt(p_pool * (1-p_pool)
                     * (1.0/data.impressions_a + 1.0/data.impressions_b));
    double z = (p_b - p_a) / se;
    double p_value = 2.0 * normal_cdf(-fabs(z));

    // 95% CI for difference
    double se_diff = sqrt(p_a*(1-p_a)/data.impressions_a
                         + p_b*(1-p_b)/data.impressions_b);
    double diff = p_b - p_a;
    *ci_low = diff - 1.96 * se_diff;
    *ci_high = diff + 1.96 * se_diff;

    return p_value;
}
```

## 5. Bootstrap Confidence Intervals for Metrics

Many recommender metrics (coverage, diversity, novelty, NDCG@k) don't have
simple variance formulas. Bootstrap provides distribution-free CIs.

### Bootstrap Procedure

```
Given: Metric function M(users, items, interactions)
B = 1000 bootstrap replicates

For b = 1 to B:
  1. Sample n_users users with replacement from the original set
  2. Compute metric M_b on the bootstrap sample

Sort M_1, ..., M_B
95% CI: [M_{(B*0.025)}, M_{(B*0.975)}]
```

```c
double* bootstrap_metric_ci(int n_users, int B,
                            double (*metric_fn)(int*, int, void*),
                            void *data, int *user_indices,
                            double *lower, double *upper,
                            double alpha) {
    double *replicates = malloc(B * sizeof(double));
    int *sample = malloc(n_users * sizeof(int));

    for (int b = 0; b < B; b++) {
        for (int i = 0; i < n_users; i++)
            sample[i] = user_indices[rand() % n_users];
        replicates[b] = metric_fn(sample, n_users, data);
    }

    qsort(replicates, B, sizeof(double), compare_double);
    *lower = replicates[(int)(B * alpha / 2.0)];
    *upper = replicates[(int)(B * (1.0 - alpha / 2.0))];

    free(sample);
    return replicates;
}
```

### Comparing Two Recommenders via Bootstrap

To test if Recommender B is significantly better than A on metric M:

```
1. Compute observed difference: d_obs = M(B) − M(A)
2. Bootstrap B paired differences:
   For b = 1..B:
     Sample users with replacement
     d_b = M_b(B) − M_b(A)
3. Compute p-value:
   p = (1/B) · Σ_b I(d_b ≤ 0)   (one-sided: B > A)
   p = (1/B) · Σ_b I(|d_b| ≥ |d_obs|)   (two-sided)
```

```c
double bootstrap_paired_test(double *rec_a_metrics, double *rec_b_metrics,
                              int n_users, int B) {
    double d_obs = 0.0;
    for (int u = 0; u < n_users; u++)
        d_obs += rec_b_metrics[u] - rec_a_metrics[u];
    d_obs /= n_users;

    int count_extreme = 0;
    for (int b = 0; b < B; b++) {
        double d_boot = 0.0;
        for (int i = 0; i < n_users; i++) {
            int idx = rand() % n_users;
            d_boot += rec_b_metrics[idx] - rec_a_metrics[idx];
        }
        d_boot /= n_users;
        if (fabs(d_boot) >= fabs(d_obs)) count_extreme++;
    }
    return (double)count_extreme / B;
}
```

## Expected Output

```
=== Recommender System Statistical Analysis ===

Metric                    Value     95% Bootstrap CI
─────────────────────────────────────────────────────
Item Coverage (@10)       0.347     [0.312, 0.381]
Intra-list Diversity      0.612     [0.598, 0.627]
Novelty (self-information)3.847    [3.791, 3.904]
Precision@10              0.231     [0.217, 0.246]
Recall@10                 0.189     [0.174, 0.203]
NDCG@10                   0.354     [0.338, 0.371]

A/B Test: New RecSys vs Current
  Metric: CTR
  Control:  3.21% (321/10000)
  Treatment: 3.58% (358/10000)
  Lift: +11.5%
  95% CI: [+0.08%, +0.66%]
  p-value: 0.142
  → NOT significant at α=0.05

Bootstrap comparison (NDCG@10):
  Observed difference: +0.018
  95% CI: [0.003, 0.034]
  p-value: 0.027
  → Significant improvement ✓

Recommendations:
  - Novelty can be improved (currently biased toward popular items)
  - Increase A/B test traffic: need ~50K users/group for 80% power at 0.5% lift
```

## Notes

- **Online vs offline metrics**: Offline metrics (precision, recall, NDCG)
  don't always correlate with online business metrics (CTR, conversion,
  retention). Always validate offline findings with an online A/B test.
- **Novelty-diversity tradeoffs**: Improving novelty often reduces accuracy.
  Consider multi-objective optimization approaches.
- **Long-tail items**: Coverage metrics reveal whether the system suffers
  from popularity bias. Cold-start items are especially vulnerable.
- **Position bias**: Users are more likely to click higher-ranked items
  regardless of relevance. Use inverse propensity weighting (IPW) to
  debias offline evaluation.
