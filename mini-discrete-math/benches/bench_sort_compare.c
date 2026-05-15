#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
double get_time_ms(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
#include <sys/time.h>
double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
#endif

static void build_test_formulas(int nvars, Formula* formulas[], int* nf) {
    *nf = 6;
    OpType roots[] = {OP_AND, OP_OR, OP_XOR, OP_IMPLIES, OP_IFF, OP_AND};
    for (int fi = 0; fi < *nf; fi++) {
        formulas[fi] = formula_create();
        Formula* f = formulas[fi];
        f->nvars = nvars;
        for (int v = 0; v < nvars; v++) {
            f->var_names[v] = 'A' + v;
            formula_add_var(f, v);
        }
        int a = 0, b = (nvars >= 2) ? 1 : 0;
        switch (roots[fi]) {
            case OP_AND:     formula_add_and(f, a, b); break;
            case OP_OR:      formula_add_or(f, a, b); break;
            case OP_XOR:     formula_add_xor(f, a, b); break;
            case OP_IMPLIES: formula_add_implies(f, a, b); break;
            case OP_IFF:     formula_add_iff(f, a, b); break;
            default: break;
        }
    }
}

static long long benchmark_tautology(Formula* f) {
    long long checks = 0;
    int rows = 1 << f->nvars;
    int root = f->ncount - 1;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS] = {0};
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        checks++;
        if (!formula_eval(f, root, assign))
            break;
    }
    return checks;
}

static long long benchmark_contradiction(Formula* f) {
    long long checks = 0;
    int rows = 1 << f->nvars;
    int root = f->ncount - 1;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS] = {0};
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        checks++;
        if (formula_eval(f, root, assign))
            break;
    }
    return checks;
}

static void free_formulas(Formula* formulas[], int nf) {
    for (int i = 0; i < nf; i++)
        free(formulas[i]);
}

int main(void) {
    printf("====== Tautology / Contradiction Verification Benchmark ======\n\n");
    printf("For each variable count (1 to %d), tests %d formulas per category.\n",
           MAX_VARS, 6);
    printf("Measures avg assignments checked and timing.\n\n");

    printf("%-6s %-10s %-18s %-18s %-14s %-14s\n",
           "Vars", "Max(2^n)", "AvgTautChecks", "AvgContraChecks",
           "TautTime(ms)", "ContraTime(ms)");
    printf("-------------------------------------------------------------------"
           "------------------\n");

    for (int nvars = 1; nvars <= MAX_VARS; nvars++) {
        int total = 1 << nvars;
        Formula* formulas[6];
        int nf;
        build_test_formulas(nvars, formulas, &nf);

        long long sum_taut = 0, sum_contra = 0;

        double t0 = get_time_ms();
        for (int i = 0; i < nf; i++)
            sum_taut += benchmark_tautology(formulas[i]);
        double taut_ms = get_time_ms() - t0;

        double t1 = get_time_ms();
        for (int i = 0; i < nf; i++)
            sum_contra += benchmark_contradiction(formulas[i]);
        double contra_ms = get_time_ms() - t1;

        printf("%-6d %-10d %-18.1f %-18.1f %-14.4f %-14.4f\n",
               nvars, total,
               (double)sum_taut / nf,
               (double)sum_contra / nf,
               taut_ms, contra_ms);

        free_formulas(formulas, nf);
    }

    printf("\nInterpretation:\n");
    printf("  - For tautologies (all rows must be true), all 2^n rows are checked.\n");
    printf("  - For contradictions, check stops early (at first true row).\n");
    printf("  - Time grows exponentially: O(2^n) formula evaluations per check.\n");

    return 0;
}
