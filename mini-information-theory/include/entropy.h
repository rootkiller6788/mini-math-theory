#ifndef ENTROPY_H
#define ENTROPY_H

double entropy_shannon(const double* probs, int n);
double entropy_binary(double p);
double entropy_joint(const double** joint_probs, int nx, int ny);
double entropy_conditional(const double** joint_probs, int nx, int ny);
double entropy_of_text(const char* text);
double entropy_rate(const char* text, int block_size);

typedef struct {
    double prob;
    char symbol;
} SymbolProb;

void entropy_empirical_distribution(const char* text, SymbolProb* dist, int* n);
double entropy_from_counts(const int* counts, int n);

#endif
