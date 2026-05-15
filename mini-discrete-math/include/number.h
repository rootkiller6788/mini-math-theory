#ifndef NUMBER_H
#define NUMBER_H

#include <stdbool.h>

long long gcd(long long a, long long b);
long long egcd(long long a, long long b, long long* x, long long* y);
long long mod_inverse(long long a, long long m);
long long mod_pow(long long base, long long exp, long long mod);
bool is_prime(long long n);
long long euler_totient(long long n);

void rsa_generate_keys(long long* n, long long* e, long long* d, long long p, long long q);
long long rsa_encrypt(long long msg, long long e, long long n);
long long rsa_decrypt(long long cipher, long long d, long long n);

void print_prime_factorization(long long n);

#endif
