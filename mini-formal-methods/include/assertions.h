#ifndef ASSERTIONS_H
#define ASSERTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "ASSERT FAIL: %s:%d — %s\n", __FILE__, __LINE__, msg); \
        abort(); \
    } \
} while(0)

void assert_eq_int(int a, int b, const char* msg);
void assert_eq_float(float a, float b, float eps, const char* msg);
void assert_neq_int(int a, int b, const char* msg);
void assert_lt_int(int a, int b, const char* msg);
void assert_le_int(int a, int b, const char* msg);
void assert_gt_int(int a, int b, const char* msg);
void assert_ge_int(int a, int b, const char* msg);
void assert_range_int(int x, int lo, int hi, const char* msg);
void assert_not_null(void* ptr, const char* msg);
void assert_str_eq(const char* a, const char* b, const char* msg);

int assert_test_count(void);
void assert_summary(void);

#endif
