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

/* Chinese Remainder Theorem (CRT): find x ≡ a[i] mod m[i] */
long long crt_solve(long long a[], long long m[], int n);

/* Miller-Rabin primality test (probabilistic, k iterations) */
bool miller_rabin(long long n, int k);

/* Fermat's Little Theorem verification: a^(p-1) ≡ 1 (mod p) for prime p */
bool fermat_little_check(long long a, long long p);

/* Primitive root modulo prime p: smallest g s.t. ord(g) = p-1 */
long long primitive_root(long long p);

/* Legendre symbol (a/p) — quadratic residue */
int legendre_symbol(long long a, long long p);

/* Jacobi symbol (a/n) — generalization of Legendre */
int jacobi_symbol(long long a, long long n);

/* Tonelli-Shanks: sqrt modulo prime (solve x² ≡ a mod p) */
long long tonelli_shanks(long long a, long long p);

/* Pollard's rho factorization */
long long pollard_rho(long long n);

/* Carmichael function λ(n) — smallest m s.t. a^m ≡ 1 for all coprime a */
long long carmichael(long long n);

/* Lucas theorem: C(n,k) mod p for prime p */
long long lucas_theorem(long long n, long long k, long long p);

/* Baby-step Giant-step discrete logarithm: find x s.t. g^x ≡ h mod p */
long long bsgs_discrete_log(long long g, long long h, long long p);

/* Fermat factorization: express n as difference of squares */
bool fermat_factor(long long n, long long* a, long long* b);

/* Linear congruence solver: solve a*x ≡ b (mod m) */
long long linear_congruence(long long a, long long b, long long m, long long* x);

/* Perfect power detection: is n = a^b for some b > 1? */
bool is_perfect_power(long long n, long long* base, long long* exponent);

/* Modular square root of -1 modulo prime p ≡ 1 mod 4 */
long long sqrt_minus_one_mod_prime(long long p);

#endif
