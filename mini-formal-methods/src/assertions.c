#include "assertions.h"

static int g_pass = 0;
static int g_fail = 0;

void assert_eq_int(int a, int b, const char* msg) {
    if (a == b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_eq: %d != %d — %s\n", a, b, msg);
}

void assert_eq_float(float a, float b, float eps, const char* msg) {
    if (fabsf(a - b) <= eps) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_eq_float: %f != %f (eps=%f) — %s\n", a, b, eps, msg);
}

void assert_neq_int(int a, int b, const char* msg) {
    if (a != b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_neq: %d == %d — %s\n", a, b, msg);
}

void assert_lt_int(int a, int b, const char* msg) {
    if (a < b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_lt: %d >= %d — %s\n", a, b, msg);
}

void assert_le_int(int a, int b, const char* msg) {
    if (a <= b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_le: %d > %d — %s\n", a, b, msg);
}

void assert_gt_int(int a, int b, const char* msg) {
    if (a > b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_gt: %d <= %d — %s\n", a, b, msg);
}

void assert_ge_int(int a, int b, const char* msg) {
    if (a >= b) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_ge: %d < %d — %s\n", a, b, msg);
}

void assert_range_int(int x, int lo, int hi, const char* msg) {
    if (x >= lo && x <= hi) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_range: %d not in [%d,%d] — %s\n", x, lo, hi, msg);
}

void assert_not_null(void* ptr, const char* msg) {
    if (ptr != NULL) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_not_null: ptr is NULL — %s\n", msg);
}

void assert_str_eq(const char* a, const char* b, const char* msg) {
    if (strcmp(a, b) == 0) { g_pass++; return; }
    g_fail++;
    fprintf(stderr, "FAIL assert_str_eq: \"%s\" != \"%s\" — %s\n", a, b, msg);
}

int assert_test_count(void) { return g_pass + g_fail; }
void assert_summary(void) {
    printf("Assertions: %d passed, %d failed, %d total\n", g_pass, g_fail, g_pass + g_fail);
}
