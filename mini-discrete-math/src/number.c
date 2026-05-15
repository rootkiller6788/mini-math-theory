#include "number.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long gcd(long long a, long long b) {
    while (b) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

long long egcd(long long a, long long b, long long* x, long long* y) {
    if (b == 0) {
        *x = 1; *y = 0;
        return a;
    }
    long long x1, y1;
    long long g = egcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

long long mod_inverse(long long a, long long m) {
    long long x, y;
    long long g = egcd(a, m, &x, &y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1)
            result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

bool is_prime(long long n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;
    for (long long i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

long long euler_totient(long long n) {
    long long result = n;
    for (long long p = 2; p * p <= n; p++) {
        if (n % p == 0) {
            while (n % p == 0)
                n /= p;
            result -= result / p;
        }
    }
    if (n > 1)
        result -= result / n;
    return result;
}

void rsa_generate_keys(long long* n, long long* e, long long* d, long long p, long long q) {
    *n = p * q;
    long long phi = (p - 1) * (q - 1);
    *e = 65537;
    if (gcd(*e, phi) != 1) {
        for (*e = 3; gcd(*e, phi) != 1; (*e) += 2)
            ;
    }
    *d = mod_inverse(*e, phi);
}

long long rsa_encrypt(long long msg, long long e, long long n) {
    return mod_pow(msg, e, n);
}

long long rsa_decrypt(long long cipher, long long d, long long n) {
    return mod_pow(cipher, d, n);
}

void print_prime_factorization(long long n) {
    printf("%lld = ", n);
    long long temp = n;
    int first = 1;
    for (long long p = 2; p * p <= temp; p++) {
        int cnt = 0;
        while (temp % p == 0) {
            temp /= p;
            cnt++;
        }
        if (cnt > 0) {
            if (!first) printf(" * ");
            printf("%lld^%d", p, cnt);
            first = 0;
        }
    }
    if (temp > 1) {
        if (!first) printf(" * ");
        printf("%lld^1", temp);
    }
    if (first) printf("1");
    printf("\n");
}
