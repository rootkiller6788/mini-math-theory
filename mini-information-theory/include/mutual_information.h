#ifndef MUTUAL_INFORMATION_H
#define MUTUAL_INFORMATION_H

double mutual_information(const double** pxy, const double* px, const double* py,
                          int nx, int ny);
double normalized_mutual_information(const double** pxy, const double* px,
                                     const double* py, int nx, int ny);
double conditional_mutual_information(const double** pxyz, int nx, int ny, int nz);
void mutual_info_from_counts(const int** counts, int nx, int ny, double* mi);

#endif
