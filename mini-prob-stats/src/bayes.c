#include "bayes.h"
#include "distribution.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

BetaBinomial* bayes_beta_binomial_create(double alpha, double beta) {
    BetaBinomial* bb = (BetaBinomial*)malloc(sizeof(BetaBinomial));
    if (!bb) return NULL;
    bb->alpha = alpha;
    bb->beta = beta;
    bb->successes = 0;
    bb->trials = 0;
    return bb;
}

void bayes_update(BetaBinomial* bb, int successes, int trials) {
    if (!bb) return;
    bb->successes += successes;
    bb->trials += trials;
}

double bayes_posterior_mean(BetaBinomial* bb) {
    if (!bb) return 0.0;
    double a = bb->alpha + bb->successes;
    double b = bb->beta + bb->trials - bb->successes;
    return a / (a + b);
}

double bayes_posterior_variance(BetaBinomial* bb) {
    if (!bb) return 0.0;
    double a = bb->alpha + bb->successes;
    double b = bb->beta + bb->trials - bb->successes;
    return (a * b) / ((a + b) * (a + b) * (a + b + 1.0));
}

double bayes_predictive(BetaBinomial* bb) {
    return bayes_posterior_mean(bb);
}

void bayes_credible_interval(BetaBinomial* bb, double prob, double* lo, double* hi) {
    if (!bb) return;
    double m = bayes_posterior_mean(bb);
    double s = sqrt(bayes_posterior_variance(bb));
    double tail = (1.0 - prob) / 2.0;
    double z = 0.0;

    if (tail >= 0.025) {
        z = 1.96;   /* ~95% */
        if (tail >= 0.05) z = 1.645;
    }
    if (tail < 0.025 && tail >= 0.005) z = 2.576;
    if (tail < 0.005) z = 3.0;

    *lo = m - z * s;
    *hi = m + z * s;
    if (*lo < 0.0) *lo = 0.0;
    if (*hi > 1.0) *hi = 1.0;
}

void bayes_free(BetaBinomial* bb) {
    free(bb);
}

NaiveBayes* naive_bayes_create(void) {
    NaiveBayes* nb = (NaiveBayes*)malloc(sizeof(NaiveBayes));
    if (!nb) return NULL;
    nb->prior[0] = 0.5;
    nb->prior[1] = 0.5;
    nb->mean[0] = 0.0;
    nb->mean[1] = 0.0;
    nb->variance[0] = 1.0;
    nb->variance[1] = 1.0;
    return nb;
}

void naive_bayes_fit(NaiveBayes* nb, double* data, int* labels, int n) {
    if (!nb || !data || !labels) return;
    int cnt[2] = {0, 0};
    double sum[2] = {0.0, 0.0};

    for (int i = 0; i < n; i++) {
        int c = labels[i];
        cnt[c]++;
        sum[c] += data[i];
    }

    nb->prior[0] = (double)cnt[0] / n;
    nb->prior[1] = (double)cnt[1] / n;

    for (int c = 0; c < 2; c++) {
        if (cnt[c] > 0) {
            nb->mean[c] = sum[c] / cnt[c];
        }
    }

    double sumsq[2] = {0.0, 0.0};
    for (int i = 0; i < n; i++) {
        int c = labels[i];
        double d = data[i] - nb->mean[c];
        sumsq[c] += d * d;
    }

    for (int c = 0; c < 2; c++) {
        if (cnt[c] > 1) {
            nb->variance[c] = sumsq[c] / (cnt[c] - 1);
        }
        if (nb->variance[c] < 1e-10) nb->variance[c] = 1e-10;
    }
}

int naive_bayes_predict(NaiveBayes* nb, double x) {
    if (!nb) return 0;
    double best = -1e300;
    int best_class = 0;
    for (int c = 0; c < 2; c++) {
        double var = nb->variance[c];
        double d = x - nb->mean[c];
        double log_pdf = -0.5 * log(2.0 * PI * var) - (d * d) / (2.0 * var);
        double score = log(nb->prior[c]) + log_pdf;
        if (score > best) {
            best = score;
            best_class = c;
        }
    }
    return best_class;
}

void naive_bayes_free(NaiveBayes* nb) {
    free(nb);
}
