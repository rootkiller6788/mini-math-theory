# mini-risk-score: Risk Scoring Demo

Compute probability of an event, update risk with new evidence, and assess
calibration of probabilistic predictions.

## Overview

Risk scoring assigns a probability to an outcome (e.g., default, disease,
conversion) based on observed features. This demo covers:

1. Logistic regression for risk estimation
2. Bayes' theorem for updating risk with new information
3. Calibration plots: predicted probability vs actual frequency

## 1. Logistic Regression for Risk Scores

The logistic model estimates P(Y = 1 | X) using a linear combination of
features passed through the sigmoid function.

```
P(Y=1 | X=x) = 1 / (1 + e^(-z))
where z = β₀ + β₁x₁ + β₂x₂ + ... + βₚxₚ
```

**Odds and log-odds**:
```
Odds = P / (1−P)
Log-odds = log(odds) = β₀ + β₁x₁ + ... + βₚxₚ
```

Each coefficient βⱼ represents the change in log-odds per unit change in xⱼ,
holding other features constant.
```
e^(βⱼ) = odds ratio for one-unit increase in xⱼ
```

**Fitting via gradient descent**:
```
β ← β − α · ∇(−ℓ(β))

where the negative log-likelihood gradient is:
∂ℓ/∂β = Xᵀ(y − σ(Xβ))

and σ(z) = 1/(1 + e^(-z)) is the sigmoid function.
```

```c
typedef struct {
    double *coeffs;   // length = n_features + 1 (β₀ is intercept)
    int n_features;
} LogisticModel;

LogisticModel reg_logistic_fit(double *X, int *y, int n, int n_features,
                                double learning_rate, int max_iter) {
    LogisticModel model;
    model.n_features = n_features;
    int p = n_features + 1;
    model.coeffs = calloc(p, sizeof(double));

    for (int iter = 0; iter < max_iter; iter++) {
        double *grad = calloc(p, sizeof(double));
        for (int i = 0; i < n; i++) {
            double z = model.coeffs[0]; // intercept
            for (int j = 0; j < n_features; j++)
                z += model.coeffs[j+1] * X[i * n_features + j];
            double p_hat = 1.0 / (1.0 + exp(-z));
            double error = y[i] - p_hat;
            grad[0] += error;
            for (int j = 0; j < n_features; j++)
                grad[j+1] += error * X[i * n_features + j];
        }
        for (int j = 0; j < p; j++)
            model.coeffs[j] += learning_rate * grad[j] / n;
        free(grad);
    }
    return model;
}

double reg_logistic_predict(LogisticModel *model, double *x) {
    double z = model->coeffs[0];
    for (int j = 0; j < model->n_features; j++)
        z += model->coeffs[j+1] * x[j];
    return 1.0 / (1.0 + exp(-z));
}
```

### Example: Credit Risk

```
Features: income, debt_to_income, num_delinquencies
Target: defaulted (0/1)

Fitted model:
  β₀ = -3.2  (intercept)
  β₁ = -0.0001  (income: higher income → lower risk)
  β₂ =  2.1   (debt_to_income: higher ratio → higher risk)
  β₃ =  0.8   (num_delinquencies: more late payments → higher risk)

For a borrower with income=$60K, DTI=0.35, 1 delinquency:
  z = -3.2 - 0.0001·60000 + 2.1·0.35 + 0.8·1
    = -3.2 - 6.0 + 0.735 + 0.8 = -7.665
  P(default) = 1/(1+e^(7.665)) ≈ 0.00047 (0.047%)
```

---

## 2. Bayes' Theorem for Updating Risk

Risk can be updated as new evidence becomes available. Starting from a prior
risk estimate (e.g., population base rate), we incorporate new information
via likelihood ratios.

```
Prior risk:     P(D)         (e.g., population prevalence)
New evidence:   E            (e.g., positive test result)
Likelihood:     P(E|D)       (test sensitivity)
                P(E|¬D)      (1 − specificity)
Posterior risk: P(D|E) = P(E|D)·P(D) / [P(E|D)·P(D) + P(E|¬D)·(1−P(D))]
```

### Sequential Updating

When evidence arrives sequentially (E₁, E₂, ..., Eₖ), assuming conditional
independence:

```
P(D|E₁,...,Eₖ) ∝ P(D) · ∏ᵢ P(Eᵢ|D)
```

Or equivalently, update the odds:

```
Posterior odds = Prior odds × ∏ LR(Eᵢ)
where LR(Eᵢ) = P(Eᵢ|D) / P(Eᵢ|¬D)   (likelihood ratio)
```

```c
typedef struct {
    double prior;
    double *posteriors;
    int n_updates;
} BayesianRiskHistory;

BayesianRiskHistory bayes_update_risk_sequence(
    double prior, double *likelihood_ratios, int n_updates)
{
    BayesianRiskHistory hist;
    hist.prior = prior;
    hist.n_updates = n_updates;
    hist.posteriors = malloc(n_updates * sizeof(double));

    double prior_odds = prior / (1.0 - prior);
    double posterior_odds = prior_odds;

    for (int i = 0; i < n_updates; i++) {
        posterior_odds *= likelihood_ratios[i];
        hist.posteriors[i] = posterior_odds / (1.0 + posterior_odds);
    }
    return hist;
}
```

### Example: Medical Diagnosis

```
Disease prevalence P(D) = 0.01 (1%)
Test sensitivity P(+|D) = 0.95
Test specificity P(−|¬D) = 0.90 → P(+|¬D) = 0.10

Prior odds = 0.01 / 0.99 ≈ 0.0101
LR(+) = 0.95 / 0.10 = 9.5
Posterior odds = 0.0101 × 9.5 ≈ 0.096
Posterior probability = 0.096 / (1+0.096) ≈ 0.088 (8.8%)

Even with a positive test, probability only rises from 1% to ~9%.
A second positive test:
Posterior odds = 0.096 × 9.5 ≈ 0.912
Posterior probability = 0.912 / 1.912 ≈ 0.477 (47.7%)
```

---

## 3. Calibration: Predicted vs Actual

A well-calibrated model produces risk scores that match observed frequencies:
when the model predicts 30% risk, ~30% of those cases should actually have
the event.

### Calibration Plot

```
1. Sort predicted probabilities into bins (e.g., deciles)
2. For each bin, compute:
   - Mean predicted probability
   - Observed event rate (fraction of positives)
3. Plot predicted (x-axis) vs observed (y-axis)
4. Perfect calibration: points lie on the y = x line
```

```c
typedef struct {
    int n_bins;
    double *bin_edges;
    double *mean_predicted;   // per bin
    double *observed_rate;    // per bin
    int *bin_counts;
} CalibrationPlot;

CalibrationPlot calibration_plot(double *predicted, int *actual, int n,
                                  int n_bins) {
    CalibrationPlot calib;
    calib.n_bins = n_bins;
    calib.bin_edges = malloc((n_bins + 1) * sizeof(double));
    calib.mean_predicted = calloc(n_bins, sizeof(double));
    calib.observed_rate = calloc(n_bins, sizeof(double));
    calib.bin_counts = calloc(n_bins, sizeof(int));

    for (int b = 0; b <= n_bins; b++)
        calib.bin_edges[b] = (double)b / n_bins;

    for (int i = 0; i < n; i++) {
        int bin = (int)(predicted[i] * n_bins);
        if (bin >= n_bins) bin = n_bins - 1;
        calib.mean_predicted[bin] += predicted[i];
        calib.observed_rate[bin] += actual[i];
        calib.bin_counts[bin]++;
    }

    for (int b = 0; b < n_bins; b++) {
        if (calib.bin_counts[b] > 0) {
            calib.mean_predicted[b] /= calib.bin_counts[b];
            calib.observed_rate[b] /= calib.bin_counts[b];
        }
    }
    return calib;
}
```

### Brier Score

Measures the mean squared error of probabilistic predictions:
```
Brier = (1/n) Σ (p̂_i − y_i)²

Brier = 0    : perfect calibration
Brier = 0.25 : uninformative (always predict 0.5)
Lower is better.
```

### Expected Calibration Error (ECE)

```
ECE = Σ_{b=1}^{B} (|B_b|/n) · |observed_rate_b − mean_predicted_b|
```

where B_b is the set of predictions in bin b.

```c
double brier_score(double *predicted, int *actual, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += (predicted[i] - actual[i]) * (predicted[i] - actual[i]);
    return sum / n;
}

double expected_calibration_error(CalibrationPlot *calib, int total_n) {
    double ece = 0.0;
    for (int b = 0; b < calib->n_bins; b++) {
        double weight = (double)calib->bin_counts[b] / total_n;
        double diff = fabs(calib->observed_rate[b] - calib->mean_predicted[b]);
        ece += weight * diff;
    }
    return ece;
}
```

## Expected Output

```
=== Risk Score Demo ===

Logistic Regression Model:
  Intercept: -3.20
  income:    -0.0001  (OR=0.9999 per $1)
  dti:        2.10    (OR=8.17 per unit)
  delinquencies: 0.80 (OR=2.23 per late payment)

Sample Risk Scores:
  Borrower A: P(default)=0.05% (low risk)
  Borrower B: P(default)=2.31% (medium risk)
  Borrower C: P(default)=18.7% (high risk)

Bayesian Update Example:
  Prior: 1.0% → Test(+) → 8.8% → Test(+) → 47.7%

Calibration:
  Brier Score: 0.062
  ECE: 0.034
  Bin  Predicted  Observed  Count
  0    0.05       0.03      120   (slightly over-confident)
  1    0.15       0.16       95   (well calibrated)
  ...
  9    0.95       0.91       72   (over-confident at extremes)
```
