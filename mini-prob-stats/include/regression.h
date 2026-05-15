#ifndef REGRESSION_H
#define REGRESSION_H

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

#endif
