# ML Evaluation: Probability & Statistics Foundations

How probability and statistics underpin machine learning model evaluation.

---

## 1. Confusion Matrix

For binary classification, the confusion matrix compares predicted labels
to actual labels.

| | Actual Positive | Actual Negative |
|---|---|---|
| **Predicted Positive** | TP (True Positive) | FP (False Positive) |
| **Predicted Negative** | FN (False Negative) | TN (True Negative) |

```c
typedef struct {
    int tp, fp, tn, fn;
} ConfusionMatrix;

ConfusionMatrix confusion_matrix(int *y_true, int *y_pred, int n) {
    ConfusionMatrix cm = {0};
    for (int i = 0; i < n; i++) {
        if (y_true[i] == 1 && y_pred[i] == 1) cm.tp++;
        else if (y_true[i] == 0 && y_pred[i] == 1) cm.fp++;
        else if (y_true[i] == 0 && y_pred[i] == 0) cm.tn++;
        else if (y_true[i] == 1 && y_pred[i] == 0) cm.fn++;
    }
    return cm;
}
```

---

## 2. Accuracy, Precision, Recall, F1

| Metric | Formula | Interpretation |
|---|---|---|
| **Accuracy** | (TP + TN) / (TP + TN + FP + FN) | Fraction correct overall |
| **Precision** | TP / (TP + FP) | Of predicted positives, how many are real |
| **Recall** (Sensitivity) | TP / (TP + FN) | Of actual positives, how many we caught |
| **Specificity** | TN / (TN + FP) | Of actual negatives, how many we caught |
| **F1 Score** | 2 · Precision · Recall / (Precision + Recall) | Harmonic mean of P and R |
| **NPV** | TN / (TN + FN) | Negative predictive value |
| **FPR** | FP / (FP + TN) = 1 − Specificity | False positive rate |

```c
double accuracy(ConfusionMatrix cm) {
    return (double)(cm.tp + cm.tn) / (cm.tp + cm.tn + cm.fp + cm.fn);
}

double precision(ConfusionMatrix cm) {
    int denom = cm.tp + cm.fp;
    return denom == 0 ? 0.0 : (double)cm.tp / denom;
}

double recall(ConfusionMatrix cm) {
    int denom = cm.tp + cm.fn;
    return denom == 0 ? 0.0 : (double)cm.tp / denom;
}

double f1_score(ConfusionMatrix cm) {
    double p = precision(cm);
    double r = recall(cm);
    double denom = p + r;
    return denom == 0 ? 0.0 : 2.0 * p * r / denom;
}
```

### Confidence Intervals for Metrics

All metrics are proportions — we can construct confidence intervals:
```
CI for accuracy = acc ± z_{α/2} · √(acc · (1−acc) / n)
```

```c
void accuracy_ci(ConfusionMatrix cm, double alpha,
                 double *lower, double *upper) {
    int n = cm.tp + cm.tn + cm.fp + cm.fn;
    double acc = accuracy(cm);
    double se = sqrt(acc * (1.0 - acc) / n);
    double z = normal_quantile(1.0 - alpha / 2.0);
    *lower = acc - z * se;
    *upper = acc + z * se;
}
```

---

## 3. ROC Curve and AUC

The **ROC curve** plots TPR (recall) vs FPR at various classification
thresholds. The **AUC (Area Under the Curve)** measures the probability that
a random positive is ranked higher than a random negative.

```
AUC = ∫₀¹ TPR(FPR⁻¹(t)) dt

AUC = 1.0 : perfect classifier
AUC = 0.5 : random classifier (no discrimination)
AUC < 0.5 : worse than random (use negative predictions)
```

```c
typedef struct {
    double *fpr, *tpr, *thresholds;
    int n_points;
} ROCCurve;

ROCCurve roc_curve(double *y_score, int *y_true, int n) {
    // Sort by descending score, compute cumulative TP, FP at each threshold
    ROCCurve curve;
    // ... sorting and accumulating logic ...
    return curve;
}

double auc(ROCCurve curve) {
    // Trapezoidal integration of the ROC curve
    double area = 0.0;
    for (int i = 1; i < curve.n_points; i++) {
        area += (curve.fpr[i] - curve.fpr[i-1])
              * (curve.tpr[i] + curve.tpr[i-1]) / 2.0;
    }
    return area;
}

// AUC confidence interval via DeLong's method or bootstrap
double auc_bootstrap_ci(double *y_score, int *y_true, int n,
                        double alpha, double *lower, double *upper) {
    int B = 1000;
    double *aucs = malloc(B * sizeof(double));
    for (int b = 0; b < B; b++) {
        // Draw bootstrap sample with replacement
        // Compute AUC on bootstrap sample
        aucs[b] = /* ... */;
    }
    qsort(aucs, B, sizeof(double), compare_double);
    *lower = aucs[(int)(B * alpha / 2.0)];
    *upper = aucs[(int)(B * (1.0 - alpha / 2.0))];
}
```

---

## 4. Cross-Validation

Cross-validation estimates how well a model generalizes to unseen data.

### k-Fold Cross-Validation

```
1. Split data into k equal-sized folds
2. For each fold i = 1..k:
   - Train on all data except fold i
   - Evaluate on fold i → get metric m_i
3. Report mean and std of m_1, ..., m_k

CV estimate: m_cv = (1/k) Σ m_i
SE of CV:    SE = s / √k   (s = sample std of m_i)
```

```c
typedef struct {
    double mean;
    double std;
    double *fold_scores;
    int k;
} CVResult;

CVResult cross_validate(double *X, int *y, int n, int n_features, int k,
                        void *(*train)(double*, int*, int, int),
                        double (*evaluate)(void*, double*, int*, int)) {
    CVResult result;
    result.k = k;
    result.fold_scores = malloc(k * sizeof(double));

    int *indices = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i;
    shuffle(indices, n);

    int fold_size = n / k;
    for (int fold = 0; fold < k; fold++) {
        int test_start = fold * fold_size;
        int test_end = (fold == k - 1) ? n : (fold + 1) * fold_size;
        int test_n = test_end - test_start;
        int train_n = n - test_n;

        // Build train/test splits
        // ... allocate and populate X_train, y_train, X_test, y_test ...

        void *model = train(X_train, y_train, train_n, n_features);
        result.fold_scores[fold] = evaluate(model, X_test, y_test, test_n);
    }

    result.mean = mean(result.fold_scores, k);
    result.std = sample_stddev(result.fold_scores, k);
    return result;
}
```

### Stratified k-Fold

Ensures each fold preserves the class proportions from the full dataset.
Especially important for imbalanced datasets.

```c
int *stratified_fold_indices(int *y, int n, int k, int fold) {
    // Separate indices by class label
    // For each class, split its indices into k folds
    // Return indices for the requested fold
}
```

---

## 5. Bootstrap for Model Evaluation

The bootstrap draws B samples of size n with replacement from the original
data to estimate the sampling distribution of any statistic.

### Bootstrap Confidence Interval for Accuracy

```
1. Original data D of size n
2. For b = 1 to B = 1000:
   - Draw bootstrap sample D*_b (size n, with replacement from D)
   - Train on D*_b, evaluate on out-of-bag (OOB) or test set → acc*_b
3. Percentile CI: [acc*_{(B·α/2)}, acc*_{(B·(1−α/2))}]
```

Out-of-bag (OOB) error: For each observation, average the predictions
from trees/bootstrap samples where that observation was NOT in the
training set. Provides an unbiased estimate of generalization error.

```c
double bootstrap_oob_error(double *X, int *y, int n, int n_features, int B,
                           void *(*train)(double*, int*, int, int),
                           double (*predict)(void*, double*)) {
    // Track OOB predictions for each observation
    double *oob_sum = calloc(n, sizeof(double));
    int *oob_count = calloc(n, sizeof(int));

    for (int b = 0; b < B; b++) {
        int *in_bag = calloc(n, sizeof(int));
        for (int i = 0; i < n; i++) {
            int idx = rand() % n;
            in_bag[idx] = 1;
        }
        // Train on in-bag, evaluate OOB predictions
        // ...
    }

    // Compute OOB error
    int errors = 0, total = 0;
    for (int i = 0; i < n; i++) {
        if (oob_count[i] > 0) {
            double pred = oob_sum[i] / oob_count[i] > 0.5 ? 1 : 0;
            if (pred != y[i]) errors++;
            total++;
        }
    }
    return (double)errors / total;
}
```

---

## 6. Statistical Significance in Model Comparison

When comparing two models, we need to determine whether observed differences
are real or due to chance.

### McNemar's Test (Paired Categorical Outcomes)

Tests whether two classifiers differ in how they classify the same instances.

| Model A \ Model B | Correct | Incorrect |
|---|---|---|
| **Correct** | a = both correct | b = A correct, B wrong |
| **Incorrect** | c = A wrong, B correct | d = both wrong |

```
χ² = (|b − c| − 1)² / (b + c)   ∼   χ²₁ under H₀ (no difference)
```

```c
double mcnemar_test(int *y_true, int *y_pred_a, int *y_pred_b, int n) {
    int b = 0, c = 0; // discordant pairs
    for (int i = 0; i < n; i++) {
        int a_correct = (y_pred_a[i] == y_true[i]);
        int b_correct = (y_pred_b[i] == y_true[i]);
        if (a_correct && !b_correct) b++;
        if (!a_correct && b_correct) c++;
    }
    double chi2 = (b + c >= 20)
                  ? (b - c) * (b - c) / (double)(b + c)
                  : (fabs(b - c) - 1.0) * (fabs(b - c) - 1.0) / (b + c);
    return chi2_pvalue(chi2, 1);
}
```

### 5×2 Cross-Validation Paired t-Test

Dietterich's 5×2 cv paired t-test is widely used for comparing classifiers:

```
1. Perform 5 iterations of 2-fold cross-validation
2. For each iteration i and fold j:
   Compute difference d_{i,j} = metric_A_{i,j} − metric_B_{i,j}
3. Compute test statistic using the 10 differences
4. Under H₀ (equal performance): t ∼ t₅ approximately
```

```c
double five_two_cv_t_test(double *X, int *y, int n, int n_features,
                          void *(*train_a)(double*, int*, int, int),
                          double (*test_a)(void*, double*, int*, int),
                          void *(*train_b)(double*, int*, int, int),
                          double (*test_b)(void*, double*, int*, int)) {
    int n1 = n / 2;
    double diffs[5][2];
    double s_sq_sum = 0.0;

    for (int i = 0; i < 5; i++) {
        shuffle_data(X, y, n);
        // Train A and B on first half, test on second half → diff[0]
        // Train A and B on second half, test on first half → diff[1]

        double p1_a = test_a(model_a1, X + n1 * n_features, y + n1, n - n1);
        double p1_b = test_b(model_b1, X + n1 * n_features, y + n1, n - n1);
        diffs[i][0] = p1_a - p1_b;

        double p2_a = test_a(model_a2, X, y, n1);
        double p2_b = test_b(model_b2, X, y, n1);
        diffs[i][1] = p2_a - p2_b;

        double s_i_sq = diffs[i][0] * diffs[i][0] + diffs[i][1] * diffs[i][1];
        s_sq_sum += s_i_sq;
    }

    double t_stat = diffs[0][0] / sqrt(s_sq_sum / 5.0);
    return t_pvalue(t_stat, 5, TWO_TAILED);
}
```

### Bootstrap Test for Model Comparison

```
1. Compute observed difference d_obs on full data
2. For b = 1 to B:
   - Bootstrap sample from paired predictions
   - Compute difference d*_b
3. p-value = (1/B) Σ I(|d*_b| ≥ |d_obs|)
4. Reject H₀: no difference if p < 0.05
```
