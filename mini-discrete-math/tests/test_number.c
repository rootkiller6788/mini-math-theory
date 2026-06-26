#include "number.h"
#include <stdio.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define CHECK_INT_EQ(actual, expected, msg) \
    do { \
        if ((actual) == (expected)) PASS(); \
        else { \
            printf("FAIL: %s (got %lld, expected %lld)\n", msg, \
                   (long long)(actual), (long long)(expected)); \
            tests_failed++; \
        } \
    } while(0)
#define CHECK_BOOL_EQ(actual, expected, msg) \
    do { \
        if ((actual) == (expected)) PASS(); \
        else { \
            printf("FAIL: %s (got %d, expected %d)\n", msg, actual, expected); \
            tests_failed++; \
        } \
    } while(0)

static void test_gcd_1071_462(void) {
    TEST("gcd(1071, 462) = 21");
    long long result = gcd(1071, 462);
    CHECK_INT_EQ(result, 21, "gcd(1071,462)");
}

static void test_gcd_252_105(void) {
    TEST("gcd(252, 105) = 21");
    long long result = gcd(252, 105);
    CHECK_INT_EQ(result, 21, "gcd(252,105)");
}

static void test_gcd_coprime(void) {
    TEST("gcd(17, 31) = 1 (coprime)");
    long long result = gcd(17, 31);
    CHECK_INT_EQ(result, 1, "gcd(17,31)");
}

static void test_gcd_with_zero(void) {
    TEST("gcd(0, 42) = 42");
    long long result = gcd(0, 42);
    CHECK_INT_EQ(result, 42, "gcd(0,42)");
}

static void test_gcd_same(void) {
    TEST("gcd(99, 99) = 99");
    long long result = gcd(99, 99);
    CHECK_INT_EQ(result, 99, "gcd(99,99)");
}

static void test_egcd_240_46(void) {
    TEST("egcd(240, 46) returns gcd=2, 240*x+46*y=2");
    long long x, y;
    long long g = egcd(240, 46, &x, &y);
    CHECK_INT_EQ(g, 2, "egcd gcd");
    CHECK_INT_EQ(240 * x + 46 * y, 2, "Bezout identity 240*x+46*y");
}

static void test_egcd_coprime(void) {
    TEST("egcd(17, 31) returns gcd=1 (coprime Bezout)");
    long long x, y;
    long long g = egcd(17, 31, &x, &y);
    CHECK_INT_EQ(g, 1, "egcd gcd for coprimes");
    CHECK_INT_EQ(17 * x + 31 * y, 1, "Bezout identity 17*x+31*y");
}

static void test_mod_inverse_7_mod_26(void) {
    TEST("7^(-1) mod 26 = 15");
    long long inv = mod_inverse(7, 26);
    CHECK_INT_EQ(inv, 15, "7^(-1) mod 26");
    CHECK_INT_EQ((7 * inv) % 26, 1, "verification 7*inv≡1 mod 26");
}

static void test_mod_inverse_no_inverse(void) {
    TEST("4^(-1) mod 26 does not exist (gcd≠1)");
    long long inv = mod_inverse(4, 26);
    CHECK_INT_EQ(inv, -1, "4^(-1) mod 26 should be -1");
}

static void test_mod_inverse_prime_mod(void) {
    TEST("3^(-1) mod 7 = 5 (since 3*5=15≡1 mod 7)");
    long long inv = mod_inverse(3, 7);
    CHECK_INT_EQ(inv, 5, "3^(-1) mod 7");
}

static void test_mod_pow_3_100_7(void) {
    TEST("3^100 mod 7 = 4");
    long long result = mod_pow(3, 100, 7);
    CHECK_INT_EQ(result, 4, "3^100 mod 7");
}

static void test_mod_pow_small(void) {
    TEST("2^10 mod 1000 = 24");
    long long result = mod_pow(2, 10, 1000);
    CHECK_INT_EQ(result, 24, "2^10 mod 1000");
}

static void test_mod_pow_identity(void) {
    TEST("5^0 mod 13 = 1");
    long long result = mod_pow(5, 0, 13);
    CHECK_INT_EQ(result, 1, "anything^0 mod m = 1");
}

static void test_mod_pow_large(void) {
    TEST("7^256 mod 1000 (fast exponentiation check)");
    long long result = mod_pow(7, 256, 1000);
    /* 7^256 mod 1000 = 601 (verified: 7^4=2401≡401, etc.) */
    CHECK_INT_EQ(result, 601, "7^256 mod 1000");
}

static void test_is_prime_small(void) {
    TEST("is_prime for small numbers");
    CHECK_BOOL_EQ(is_prime(2), true, "2 is prime");
    CHECK_BOOL_EQ(is_prime(3), true, "3 is prime");
    CHECK_BOOL_EQ(is_prime(4), false, "4 is not prime");
    CHECK_BOOL_EQ(is_prime(17), true, "17 is prime");
    CHECK_BOOL_EQ(is_prime(91), false, "91=7*13 is not prime");
    CHECK_BOOL_EQ(is_prime(97), true, "97 is prime");
    CHECK_BOOL_EQ(is_prime(1), false, "1 is not prime");
    CHECK_BOOL_EQ(is_prime(0), false, "0 is not prime");
}

static void test_euler_totient(void) {
    TEST("Euler totient φ(n)");
    CHECK_INT_EQ(euler_totient(6), 2, "φ(6)=2");
    CHECK_INT_EQ(euler_totient(15), 8, "φ(15)=8");
    CHECK_INT_EQ(euler_totient(7), 6, "φ(7)=6 (prime)");
    CHECK_INT_EQ(euler_totient(1), 1, "φ(1)=1");
    CHECK_INT_EQ(euler_totient(12), 4, "φ(12)=4");
}

static void test_rsa_roundtrip_61_53(void) {
    TEST("RSA encrypt/decrypt roundtrip (p=61, q=53)");
    long long p = 61, q = 53;
    long long n, e, d;
    rsa_generate_keys(&n, &e, &d, p, q);

    long long messages[] = {42, 100, 1234, 2048, 1, 999, 0, n - 1};
    int n_msgs = 8;
    int all_pass = 1;

    for (int i = 0; i < n_msgs; i++) {
        long long msg = messages[i];
        long long cipher = rsa_encrypt(msg, e, n);
        long long decrypted = rsa_decrypt(cipher, d, n);
        if (decrypted != msg) {
            printf("FAIL: msg=%lld, cipher=%lld, decrypted=%lld\n",
                   msg, cipher, decrypted);
            tests_failed++;
            all_pass = 0;
            break;
        }
    }
    if (all_pass) PASS();
}

static void test_rsa_roundtrip_small_primes(void) {
    TEST("RSA roundtrip with small primes (p=3, q=11)");
    long long p = 3, q = 11;
    long long n, e, d;
    rsa_generate_keys(&n, &e, &d, p, q);

    long long messages[] = {0, 1, 2, 5, 10, 15, 20, 25, 30, 32};
    int n_msgs = 10;
    int all_pass = 1;

    for (int i = 0; i < n_msgs; i++) {
        long long msg = messages[i];
        long long cipher = rsa_encrypt(msg, e, n);
        long long decrypted = rsa_decrypt(cipher, d, n);
        if (decrypted != msg) {
            printf("FAIL: msg=%lld, cipher=%lld, decrypted=%lld\n",
                   msg, cipher, decrypted);
            tests_failed++;
            all_pass = 0;
            break;
        }
    }
    if (all_pass) PASS();
}

static void test_rsa_key_generation(void) {
    TEST("RSA key validation: e*d ≡ 1 (mod φ(n))");
    long long p = 61, q = 53;
    long long n, e, d;
    rsa_generate_keys(&n, &e, &d, p, q);

    long long phi = (p - 1) * (q - 1);
    long long check = (e * d) % phi;

    CHECK_INT_EQ(check, 1, "e*d mod φ(n)");
    CHECK_INT_EQ(n, p * q, "n = p * q");
}

int main(void) {
    printf("====== Number Theory Module Tests ======\n\n");

    printf("--- GCD ---\n");
    test_gcd_1071_462();
    test_gcd_252_105();
    test_gcd_coprime();
    test_gcd_with_zero();
    test_gcd_same();

    printf("\n--- Extended Euclidean (Bezout) ---\n");
    test_egcd_240_46();
    test_egcd_coprime();

    printf("\n--- Modular Inverse ---\n");
    test_mod_inverse_7_mod_26();
    test_mod_inverse_no_inverse();
    test_mod_inverse_prime_mod();

    printf("\n--- Fast Modular Exponentiation ---\n");
    test_mod_pow_3_100_7();
    test_mod_pow_small();
    test_mod_pow_identity();
    test_mod_pow_large();

    printf("\n--- Prime Detection ---\n");
    test_is_prime_small();

    printf("\n--- Euler Totient ---\n");
    test_euler_totient();

    printf("\n--- RSA Cryptosystem ---\n");
    test_rsa_key_generation();
    test_rsa_roundtrip_61_53();
    test_rsa_roundtrip_small_primes();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
