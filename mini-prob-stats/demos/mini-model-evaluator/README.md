# mini-model-evaluator: Model Evaluation Demo

Train/test splitting, cross-validation, statistical model comparison, and
bootstrap confidence intervals for model evaluation metrics.

## Overview

Proper model evaluation requires more than a single accuracy number. This
demo covers:

1. Train/test split with stratification
2. k-Fold cross-validation with variance estimates
3. Statistical comparison of two models (paired t-test, McNemar)
4. Bootstrap confidence intervals for any metric

## 1. Train/Test Split

Splitting data into training and held-out test sets is the most basic form
of evaluation. We need to be careful about:

- **Stratification**: Preserve class proportions in both sets
- **Shuffling**: Randomize before splitting to avoid ordering bias
- **Leakage**: Never use test data for any part of training (feature
  selection, normalization parameters, etc.)

```c
typedef struct {
    double *X_train, *X_test;
    int *y_train, *y_test;
    int n_train, n_test;
    int n_features;
} TrainTestSplit;

TrainTestSplit train_test_split(double *X, int *y, int n, int n_features,
                                 double test_ratio, int stratify) {
    TrainTestSplit split;
    split.n_test = (int)(n * test_ratio);
    split.n_train = n - split.n_test;
    split.n_features = n_features;

    int *indices = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i;

    if (stratify) {
        stratified_shuffle(indices, y, n);
    } else {
        shuffle(indices, n);
    }

    split.X_train = malloc(split.n_train * n_features * sizeof(double));
    split.y_train = malloc(split.n_train * sizeof(int));
    split.X_test = malloc(split.n_test * n_features * sizeof(double));
    split.y_test = malloc(split.n_test * sizeof(int));

    for (int i = 0; i < split.n_train; i++) {
        int idx = indices[i];
        split.y_train[i] = y[idx];
        memcpy(&split.X_train[i * n_features], &X[idx * n_features],
               n_features * sizeof(double));
    }
    for (int i = 0; i < split.n_test; i++) {
        int idx = indices[split.n_train + i];
        split.y_test[i] = y[idx];
        memcpy(&split.X_test[i * n_features], &X[idx * n_features],
               n_features * sizeof(double));
    }

    free(indices);
    return split;
}
```

After training on the training set, compute metrics on the test set:

```c
void evaluate_on_test(void *model, TrainTestSplit split,
                      double (*predict_proba)(void*, double*)) {
    int *y_pred = malloc(split.n_test * sizeof(int));
    double *y_score = malloc(split.n_test * sizeof(double));

    for (int i = 0; i < split.n_test; i++) {
        y_score[i] = predict_proba(model, &split.X_test[i * split.n_features]);
        y_pred[i] = y_score[i] >= 0.5 ? 1 : 0;
    }

    ConfusionMatrix cm = confusion_matrix(split.y_test, y_pred, split.n_test);
    printf("Accuracy:  %.4f\n", accuracy(cm));
    printf("Precision: %.4f\n", precision(cm));
    printf("Recall:    %.4f\n", recall(cm));
    printf("F1 Score:  %.4f\n", f1_score(cm));

    // Confidence interval for accuracy
    double ci_low, ci_high;
    accuracy_ci(cm, 0.05, &ci_low, &ci_high);
    printf("95%% CI:    [%.4f, %.4f]\n", ci_low, ci_high);

    free(y_pred);
    free(y_score);
}
```

## 2. k-Fold Cross-Validation

Cross-validation provides a more robust estimate of generalization
performance by averaging over k train/test splits.

```
Algorithm: k-Fold Cross-Validation
──────────────────────────────────
Input: Data D = {(x₁,y₁), ..., (xₙ,yₙ)}, k folds
Output: CV score estimate ± uncertainty

1. Shuffle D randomly
2. Partition D into k equal-sized folds F₁, ..., Fₖ
3. For fold i = 1 to k:
     Train on D \ F_i  (all data except fold i)
     Evaluate metric m_i on F_i
4. Report:
     CV mean: m̄ = (1/k) Σ m_i
     CV std:  s = √(1/(k−1) Σ (m_i − m̄)²)
     SE:      s / √k
     95% CI:  m̄ ± 1.96 · s / √k   (rough)
```

### Nested Cross-Validation

For unbiased model selection + evaluation when tuning hyperparameters:

```
Outer loop (evaluation): k_outer folds
  For each outer fold:
    Inner loop (model selection): k_inner folds on training data
      For each hyperparameter setting:
        k-inner-fold CV → mean score
      Select best hyperparameters
    Retrain with best hyperparameters on full training data
    Evaluate on outer test fold → score_i

Final: report mean ± std of k_outer scores
```

```c
typedef struct {
    int k;
    double *scores;
    double mean;
    double std;
    double se;
} CrossValidationResult;

CrossValidationResult k_fold_cv(double *X, int *y, int n, int n_features,
                                 int k,
                                 void *(*train_fn)(double*, int*, int, int),
                                 double (*score_fn)(void*, double*, int*, int)) {
    CrossValidationResult result;
    result.k = k;
    result.scores = malloc(k * sizeof(double));

    int *indices = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i;
    shuffle(indices, n);

    int fold_size = n / k;
    for (int fold = 0; fold < k; fold++) {
        int test_start = fold * fold_size;
        int test_end = (fold == k - 1) ? n : test_start + fold_size;
        int test_n = test_end - test_start;
        int train_n = n - test_n;

        double *X_train = malloc(train_n * n_features * sizeof(double));
        int *y_train = malloc(train_n * sizeof(int));
        double *X_test = malloc(test_n * n_features * sizeof(double));
        int *y_test = malloc(test_n * sizeof(int));

        int ti = 0, si = 0;
        for (int i = 0; i < n; i++) {
            if (i >= test_start && i < test_end) {
                y_test[si] = y[indices[i]];
                memcpy(&X_test[si * n_features],
                       &X[indices[i] * n_features],
                       n_features * sizeof(double));
                si++;
            } else {
                y_train[ti] = y[indices[i]];
                memcpy(&X_train[ti * n_features],
                       &X[indices[i] * n_features],
                       n_features * sizeof(double));
                ti++;
            }
        }

        void *model = train_fn(X_train, y_train, train_n, n_features);
        result.scores[fold] = score_fn(model, X_test, y_test, test_n);

        free(X_train); free(y_train);
        free(X_test); free(y_test);
    }

    result.mean = mean(result.scores, k);
    result.std = sample_stddev(result.scores, k);
    result.se = result.std / sqrt(k);
    free(indices);
    return result;
}
```

### Choosing k

| k | Pros | Cons |
|---|---|---|
| k = 2 | Fastest | High variance, pessimistic bias |
| k = 5 | Good balance | Moderate variance |
| k = 10 | Lower bias, common in ML | Slower |
| k = n (LOOCV) | Nearly unbiased | Very slow, high variance for AUC |

## 3. Statistical Comparison of Two Models

When Model B claims to beat Model A, we must verify the difference is
statistically significant.

### Paired t-Test on Cross-Validation Scores

When both models are evaluated on the same k folds:

```
d_i = score_B_i − score_A_i   for each fold i = 1..k

H₀: μ_d = 0  (no difference)
H₁: μ_d ≠ 0  (B is different from A)

t = d̄ / (s_d / √k)   ∼   t_{k−1} under H₀
```

```c
int paired_t_test_models(CrossValidationResult cv_a,
                          CrossValidationResult cv_b,
                          double alpha, double *p_value) {
    // Both must have the same k
    double *diffs = malloc(cv_a.k * sizeof(double));
    for (int i = 0; i < cv_a.k; i++)
        diffs[i] = cv_b.scores[i] - cv_a.scores[i];

    double d_bar = mean(diffs, cv_a.k);
    double s_d = sample_stddev(diffs, cv_a.k);
    double t_stat = d_bar / (s_d / sqrt(cv_a.k));
    double df = cv_a.k - 1;

    *p_value = 2.0 * t_cdf(-fabs(t_stat), df);
    free(diffs);
    return *p_value < alpha;
}
```

### McNemar's Test (For Classification)

Tests whether two classifiers differ in their error patterns on the same
test set.

```
            Model B correct   Model B wrong
Model A correct       a             b
Model A wrong         c             d

χ² = (|b−c|−1)² / (b+c)   ∼   χ²₁ under H₀
```

This test is especially sensitive when one model is better on a specific
subset of examples that the other gets wrong.

### 5×2 Cross-Validation Paired t-Test (Dietterich)

More robust than simple k-fold paired t-test. Runs 5 iterations of 2-fold
CV, giving 10 differences. The test statistic uses an adjusted variance
estimate to avoid the elevated Type I error of the naive paired t-test.

```c
double five_by_two_cv_t_test(double *X, int *y, int n, int n_features,
                             void *(*train_a)(double*, int*, int, int),
                             double (*test_a)(void*, double*, int*, int),
                             void *(*train_b)(double*, int*, int, int),
                             double (*test_b)(void*, double*, int*, int)) {
    double diffs[5][2];
    double s2 = 0.0;

    for (int iter = 0; iter < 5; iter++) {
        // Random 50/50 split
        shuffle_data(X, y, n);
        int n1 = n / 2, n2 = n - n1;

        // Train on first half, test on second
        void *ma1 = train_a(X, y, n1, n_features);
        void *mb1 = train_b(X, y, n1, n_features);
        diffs[iter][0] = test_a(ma1, X+n1*n_features, y+n1, n2)
                       - test_b(mb1, X+n1*n_features, y+n1, n2);

        // Train on second half, test on first
        void *ma2 = train_a(X+n1*n_features, y+n1, n2, n_features);
        void *mb2 = train_b(X+n1*n_features, y+n1, n2, n_features);
        diffs[iter][1] = test_a(ma2, X, y, n1)
                       - test_b(mb2, X, y, n1);

        // Pooled variance for this iteration
        double s2_i = diffs[iter][0]*diffs[iter][0]
                    + diffs[iter][1]*diffs[iter][1];
        s2 += s2_i;
    }

    double t_stat = diffs[0][0] / sqrt(s2 / 5.0);
    return t_pvalue(t_stat, 5, TWO_TAILED);
}
```

## 4. Bootstrap Confidence Intervals for Metrics

For any metric (accuracy, precision, recall, F1, AUC, etc.), we can obtain
a confidence interval via bootstrap without assuming any distribution.

### Bootstrap Procedure for Test Set Metrics

```
Given: n test predictions (y_true_i, y_score_i) for i = 1..n

For b = 1 to B (e.g., B = 2000):
  1. Sample n indices with replacement from {1, ..., n}
  2. Compute metric M on the bootstrap sample → M_b

Sort M_1, ..., M_B
95% Percentile CI: [M_{(B·0.025)}, M_{(B·0.975)}]
```

```c
typedef struct {
    double *y_score;
    int *y_true;
    int n;
} EvaluationData;

double* bootstrap_metric_ci(double (*metric_fn)(EvaluationData*),
                             EvaluationData *data, int B,
                             double alpha, double *lower, double *upper) {
    double *replicates = malloc(B * sizeof(double));
    int *indices = malloc(data->n * sizeof(int));

    for (int b = 0; b < B; b++) {
        for (int i = 0; i < data->n; i++)
            indices[i] = rand() % data->n;

        double *ys_sample = malloc(data->n * sizeof(double));
        int *yt_sample = malloc(data->n * sizeof(int));
        for (int i = 0; i < data->n; i++) {
            ys_sample[i] = data->y_score[indices[i]];
            yt_sample[i] = data->y_true[indices[i]];
        }

        EvaluationData boot_data = {ys_sample, yt_sample, data->n};
        replicates[b] = metric_fn(&boot_data);
        free(ys_sample); free(yt_sample);
    }

    qsort(replicates, B, sizeof(double), compare_double);
    *lower = replicates[(int)(B * alpha / 2.0)];
    *upper = replicates[(int)(B * (1.0 - alpha / 2.0))];
    free(indices);
    return replicates;
}
```

### Bootstrap CI for AUC

```c
double auc_eval(EvaluationData *data) {
    // Compute AUC from scores and true labels
    int n = data->n;
    int n_pos = 0;
    for (int i = 0; i < n; i++) n_pos += data->y_true[i];
    int n_neg = n - n_pos;

    // Rank scores, compute Mann-Whitney U statistic
    // AUC = U / (n_pos * n_neg)
    // ...
}
```

## Expected Output

```
=== Model Evaluation ===

Dataset: 1000 samples, 20 features, 2 classes
Class balance: 65% positive, 35% negative
Train/test split: 80/20 (stratified)

Model A (Logistic Regression):
  Train accuracy: 0.873
  Test accuracy:  0.845
  Test precision: 0.821
  Test recall:    0.912
  Test F1:        0.864
  Test AUC:       0.892
  95% bootstrap CI for Test AUC: [0.871, 0.913]

5-Fold Cross-Validation:
  Fold 1: 0.851   Fold 2: 0.839   Fold 3: 0.862
  Fold 4: 0.844   Fold 5: 0.848
  Mean CV accuracy: 0.849 ± 0.009
  SE: 0.004

Model B (Random Forest):
  Test accuracy:  0.872
  Test F1:        0.885
  Test AUC:       0.918
  95% bootstrap CI for Test AUC: [0.898, 0.936]

Model Comparison: B vs A
  Δ Accuracy:  +0.027  (B better)
  Δ F1:        +0.021
  Δ AUC:       +0.026

  5×2 CV paired t-test:
    t-statistic = 3.41, p-value = 0.019
    → Model B significantly better than A at α = 0.05 ✓

  McNemar test:
    b (A right / B wrong) = 18
    c (A wrong / B right) = 38
    χ² = 6.45, p-value = 0.011
    → Consistent difference in error patterns ✓

  Bootstrap test for AUC difference:
    Observed ΔAUC = 0.026
    95% CI for ΔAUC: [0.008, 0.044]
    p-value = 0.006
    → Significant AUC improvement ✓
```

## Notes

- **Data leakage**: Never fit preprocessing (scaling, imputation, encoding)
  on the full dataset before splitting. Fit on training data only, then
  transform both train and test.
- **Class imbalance**: For imbalanced datasets (e.g., 1% positive class),
  accuracy is misleading. Use precision, recall, F1, AUC-ROC, or
  precision-recall AUC instead.
- **Temporal data**: For time-series or sequential data, use time-based
  splits (train on past, test on future) rather than random splits.
- **Group leakage**: If data has groups (e.g., multiple samples per user),
  split at the group level to prevent the same user from appearing in
  both train and test.
- **Statistical vs practical significance**: With large test sets, even
  tiny (practically meaningless) improvements become statistically
  significant. Always report effect size along with p-values.
