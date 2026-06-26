#ifndef REGRESSION_H
#define REGRESSION_H

#include <stdbool.h>

/* L1: Simple Linear Regression */
typedef struct {
    double beta0, beta1;
    double r_squared;
    double se_beta1;
    double t_stat;
    double p_value;
} LinearRegression;

LinearRegression* linreg_fit(double* x, double* y, int n);
double linreg_predict(LinearRegression* lr, double x);
void linreg_print(LinearRegression* lr);
void linreg_free(LinearRegression* lr);
double* linreg_residuals(double* x, double* y, int n, double beta0, double beta1);

/* L5: Multiple Linear Regression */
typedef struct {
    double* beta;       /* length = p+1 (beta[0] = intercept) */
    int p;              /* number of predictors */
    double r_squared;
} MultipleRegression;

MultipleRegression* multireg_fit(double* X, double* y, int n, int p);
double multireg_predict(MultipleRegression* mr, double* x);
void multireg_free(MultipleRegression* mr);

/* L5: Logistic Regression (binary) */
typedef struct {
    double* beta;       /* length = p+1 */
    int p;
    bool converged;
} LogisticRegression;

LogisticRegression* logreg_fit(double* X, int* y, int n, int p,
                                double lr, int max_iter);
double logreg_predict_prob(LogisticRegression* lr_model, double* x);
int logreg_predict(LogisticRegression* lr_model, double* x);
void logreg_free(LogisticRegression* lr_model);

/* L5: Polynomial Regression */
typedef struct {
    double* beta;
    int degree;
    double r_squared;
} PolynomialRegression;

PolynomialRegression* polyreg_fit(double* x, double* y, int n, int degree);
double polyreg_predict(PolynomialRegression* pr, double x);
void polyreg_free(PolynomialRegression* pr);

/* L8: Ridge Regression (L2) */
typedef struct {
    double* beta;
    int p;
    double lambda;
    double r_squared;
} RidgeRegression;

RidgeRegression* ridge_fit(double* X, double* y, int n, int p, double lambda);
double ridge_predict(RidgeRegression* rr, double* x);
void ridge_free(RidgeRegression* rr);

#endif
