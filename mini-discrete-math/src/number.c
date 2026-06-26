#include "number.h"
#include "combo.h"
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

/* Chinese Remainder Theorem (CRT)
 * Theorem: For pairwise coprime m[i], ∃ unique x mod M = ∏m[i]
 *         s.t. x ≡ a[i] (mod m[i]) for all i.
 * Uses Garner's algorithm: x = Σ a_i * M_i * inv(M_i mod m_i)
 * where M_i = M / m_i.
 * Reference: MIT 6.042J Number Theory, CLRS §31.5 */
long long crt_solve(long long a[], long long m[], int n) {
    long long M = 1;
    for (int i = 0; i < n; i++) M *= m[i];
    long long result = 0;
    for (int i = 0; i < n; i++) {
        long long Mi = M / m[i];
        long long inv = mod_inverse(Mi, m[i]);
        if (inv == -1) return -1; /* moduli not coprime */
        long long term = ((a[i] * Mi) % M * inv) % M;
        result = (result + term) % M;
    }
    return (result + M) % M;
}

/* Miller-Rabin primality test (probabilistic)
 * Write n-1 = 2^s * d. For random a, check: a^d ≡ 1 or a^(2^r*d) ≡ -1.
 * Theorem: If n is composite, at most n/4 bases are "liars" (Rabin).
 * k iterations give error probability ≤ (1/4)^k.
 * Reference: Rabin (1980), CLRS §31.8 */
static long long mmul(long long a, long long b, long long mod) {
    /* modular multiplication avoiding overflow for 64-bit */
    long long res = 0;
    a %= mod;
    while (b) {
        if (b & 1) res = (res + a) % mod;
        a = (a * 2) % mod;
        b >>= 1;
    }
    return res;
}

static long long mpow(long long base, long long exp, long long mod) {
    long long res = 1;
    base %= mod;
    while (exp) {
        if (exp & 1) res = mmul(res, base, mod);
        base = mmul(base, base, mod);
        exp >>= 1;
    }
    return res;
}

bool miller_rabin(long long n, int k) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    long long d = n - 1;
    int s = 0;
    while (d % 2 == 0) { d /= 2; s++; }

    /* deterministic bases for 64-bit: {2,3,5,7,11,13,17} suffice */
    long long bases[] = {2, 3, 5, 7, 11, 13, 17};
    int nbases = k < 7 ? k : 7;

    for (int i = 0; i < nbases; i++) {
        long long a = bases[i];
        if (a >= n) continue;
        long long x = mpow(a % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int r = 1; r < s; r++) {
            x = mmul(x, x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

/* Fermat's Little Theorem: If p is prime and gcd(a,p)=1, then a^(p-1) ≡ 1 (mod p)
 * This function checks the congruence. Note: Carmichael numbers pass this.
 * Reference: MIT 6.042J §7 */
bool fermat_little_check(long long a, long long p) {
    if (p < 2) return false;
    if (gcd(a, p) != 1) return false;
    return mod_pow(a, p - 1, p) == 1;
}

/* Primitive root modulo prime p
 * g is primitive root iff g^((p-1)/q) ≠ 1 mod p for all prime factors q of p-1.
 * Reference: MIT 6.042J, Gauss */
long long primitive_root(long long p) {
    if (p < 2) return -1;
    if (p == 2) return 1;

    /* factor p-1 */
    long long phi = p - 1;
    long long factors[64];
    int nf = 0;
    long long temp = phi;
    for (long long q = 2; q * q <= temp; q++) {
        if (temp % q == 0) {
            factors[nf++] = q;
            while (temp % q == 0) temp /= q;
        }
    }
    if (temp > 1) factors[nf++] = temp;

    /* test candidates */
    for (long long g = 2; g < p; g++) {
        bool ok = true;
        for (int i = 0; i < nf; i++) {
            if (mod_pow(g, phi / factors[i], p) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) return g;
    }
    return -1;
}

/* Legendre symbol (a/p): quadratic residue modulo odd prime p
 * Returns: 1 if a is QR mod p, -1 if not, 0 if p|a.
 * Uses Euler's criterion: (a/p) ≡ a^((p-1)/2) mod p
 * Reference: MIT 6.042J, Gauss's Quadratic Reciprocity */
int legendre_symbol(long long a, long long p) {
    if (a % p == 0) return 0;
    long long res = mod_pow(a, (p - 1) / 2, p);
    return (res == 1) ? 1 : -1;
}

/* Jacobi symbol (a/n): generalization of Legendre for odd n > 1
 * Doesn't determine quadratic residuosity but used in primality testing.
 * Reference: Bach & Shallit, Algorithmic Number Theory */
int jacobi_symbol(long long a, long long n) {
    if (n <= 0 || n % 2 == 0) return 0;
    if (a == 0) return 0;
    if (a == 1) return 1;

    /* remove factors of 2 from a */
    int result = 1;
    while (a % 2 == 0) {
        a /= 2;
        long long r = n % 8;
        if (r == 3 || r == 5) result = -result;
    }

    if (a >= n) a %= n;
    if (a == 0) return 0;
    if (a == 1) return result;

    /* quadratic reciprocity */
    if (a % 4 == 3 && n % 4 == 3) result = -result;
    return result * jacobi_symbol(n % a, a);
}

/* Tonelli-Shanks: modular square root x² ≡ a (mod p), p odd prime
 * Algorithm: find Q,S s.t. p-1 = Q*2^S, find non-residue z, iterate.
 * Reference: Tonelli (1891), Shanks (1973), Cohen §1.5 */
long long tonelli_shanks(long long a, long long p) {
    if (a % p == 0) return 0;
    if (legendre_symbol(a, p) != 1) return -1; /* no sqrt exists */

    /* p ≡ 3 (mod 4): sqrt ≡ a^((p+1)/4) */
    if (p % 4 == 3)
        return mod_pow(a, (p + 1) / 4, p);

    /* find Q, S: p-1 = Q * 2^S */
    long long Q = p - 1;
    int S = 0;
    while (Q % 2 == 0) { Q /= 2; S++; }

    /* find quadratic non-residue z */
    long long z = 2;
    while (legendre_symbol(z, p) != -1) z++;

    long long M = S;
    long long c = mod_pow(z, Q, p);
    long long t = mod_pow(a, Q, p);
    long long R = mod_pow(a, (Q + 1) / 2, p);

    while (t != 1) {
        /* find least i > 0 s.t. t^(2^i) ≡ 1 */
        int i = 1;
        long long t2 = (t * t) % p;
        while (t2 != 1) { t2 = (t2 * t2) % p; i++; }

        long long b = mod_pow(c, 1LL << (M - i - 1), p);
        M = i;
        c = (b * b) % p;
        t = (t * c) % p;
        R = (R * b) % p;
    }
    return R;
}

/* Pollard's rho factorization (Monte Carlo)
 * Uses f(x) = x^2 + c mod n, detects cycle with Floyd's cycle-finding.
 * Expected O(√p) where p is smallest prime factor.
 * Reference: Pollard (1975), CLRS §31.9 */
long long pollard_rho(long long n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    long long x = 2, y = 2, d = 1;
    long long c = 1;

    while (d == 1) {
        x = (mmul(x, x, n) + c) % n;
        y = (mmul(y, y, n) + c) % n;
        y = (mmul(y, y, n) + c) % n;
        long long diff = x - y;
        if (diff < 0) diff = -diff;
        d = gcd(diff, n);
        if (d == n) { c++; x = y = 2; d = 1; }
    }
    return d;
}

/* Carmichael function λ(n)
 * For n = ∏ p_i^e_i: λ(n) = lcm(λ(p_i^e_i))
 * λ(p^e) = φ(p^e) for odd p or p=2,e≤2; λ(2^e)=φ(2^e)/2 for e≥3.
 * Reference: Carmichael (1910), Erdős-Pomerance-Schmutz */
long long carmichael(long long n) {
    if (n == 1) return 1;
    long long result = 1;
    long long temp = n;

    for (long long p = 2; p * p <= temp; p++) {
        if (temp % p == 0) {
            int e = 0;
            long long pk = 1;
            while (temp % p == 0) { temp /= p; e++; pk *= p; }
            long long lam = (p == 2 && e >= 3) ?
                           euler_totient(pk) / 2 :
                           euler_totient(pk);
            result = result / gcd(result, lam) * lam; /* lcm */
        }
    }
    if (temp > 1) {
        long long lam = temp - 1;
        result = result / gcd(result, lam) * lam;
    }
    return result;
}

/* Lucas Theorem: C(n,k) mod p for prime p
 * Write n = n_m p^m + ... + n_0, k = k_m p^m + ... + k_0 in base p.
 * Then C(n,k) ≡ ∏ C(n_i, k_i) (mod p).
 * Reference: Lucas (1878), Fine (1947) */
long long lucas_theorem(long long n, long long k, long long p) {
    if (k < 0 || k > n) return 0;
    long long result = 1;
    while (n > 0 || k > 0) {
        long long ni = n % p;
        long long ki = k % p;
        if (ki > ni) return 0;
        result = (result * combination((int)ni, (int)ki)) % p;
        n /= p;
        k /= p;
    }
    return result;
}

/* Baby-step Giant-step discrete logarithm: find x s.t. g^x ≡ h (mod p)
 * Let m = ⌈√p⌉. Compute baby steps g^j, giant steps h*g^(-im).
 * O(√p) time, O(√p) space.
 * Reference: Shanks (1971), CLRS §31.7 */
long long bsgs_discrete_log(long long g, long long h, long long p) {
    long long m = (long long)ceil(sqrt((double)p));
    /* baby steps: g^j mod p for j in [0, m) */
    typedef struct { long long val; long long j; } Entry;
    Entry table[1024];
    int tsize = 0;
    long long cur = 1;
    for (long long j = 0; j < m && tsize < 1024; j++) {
        table[tsize].val = cur;
        table[tsize].j = j;
        tsize++;
        cur = (cur * g) % p;
    }

    /* sort by value (insertion sort) */
    for (int i = 1; i < tsize; i++) {
        Entry key = table[i];
        int j = i - 1;
        while (j >= 0 && table[j].val > key.val) {
            table[j + 1] = table[j];
            j--;
        }
        table[j + 1] = key;
    }

    /* g^(-m) mod p */
    long long g_inv_m = mod_pow(mod_inverse(g, p), m, p);
    long long gamma = h;

    for (long long i = 0; i < m; i++) {
        /* binary search in table */
        int lo = 0, hi = tsize - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (table[mid].val == gamma)
                return i * m + table[mid].j;
            if (table[mid].val < gamma)
                lo = mid + 1;
            else
                hi = mid - 1;
        }
        gamma = (gamma * g_inv_m) % p;
    }
    return -1; /* no solution */
}

/* Fermat factorization: express n = a² - b² = (a-b)(a+b)
 * For n = uv with u,v odd, close together: a = (u+v)/2, b = (u-v)/2.
 * Reference: Fermat, Bressoud §5 */
bool fermat_factor(long long n, long long* a, long long* b) {
    if (n <= 0) return false;
    if (n % 2 == 0) { *a = n / 2; *b = 1; return true; }

    long long x = (long long)ceil(sqrt((double)n));
    long long y2 = x * x - n;
    long long max_iter = 1000000;
    long long iter = 0;
    while (iter < max_iter) {
        long long y = (long long)sqrt((double)y2);
        if (y * y == y2) {
            *a = x + y;
            *b = x - y;
            return true;
        }
        x++;
        y2 = x * x - n;
        iter++;
    }
    return false;
}

/* Linear congruence solver: solve a*x ≡ b (mod m)
 * Solution exists iff gcd(a,m) | b. Returns number of solutions mod m.
 * Reference: MIT 6.042J §8 */
long long linear_congruence(long long a, long long b, long long m, long long* x) {
    long long g = gcd(a, m);
    if (b % g != 0) return -1; /* no solution */
    long long a_prime = a / g;
    long long b_prime = b / g;
    long long m_prime = m / g;
    long long inv = mod_inverse(a_prime, m_prime);
    *x = (inv * b_prime) % m_prime;
    if (*x < 0) *x += m_prime;
    return g; /* g distinct solutions: x, x+m', ..., x+(g-1)m' mod m */
}

/* Perfect power detection: check if n = a^b for b > 1
 * Tests exponents 2,3,... up to log2(n).
 * Reference: Bernstein (1998), Detecting perfect powers */
bool is_perfect_power(long long n, long long* base, long long* exponent) {
    if (n < 2) return false;
    /* max exponent e s.t. 2^e <= n */
    int max_exp = 0;
    long long tmp = n;
    while (tmp > 1) { tmp >>= 1; max_exp++; }
    if (max_exp < 2) return false;

    for (int b = max_exp; b >= 2; b--) {
        /* integer b-th root via binary search */
        long long lo = 1, hi = (long long)pow((double)n, 1.0 / b) + 2;
        while (lo <= hi) {
            long long mid = (lo + hi) / 2;
            long long pow_val = 1;
            int overflow = 0;
            for (int i = 0; i < b; i++) {
                if (pow_val > n / mid) { overflow = 1; break; }
                pow_val *= mid;
            }
            if (!overflow && pow_val == n) {
                *base = mid;
                *exponent = b;
                return true;
            }
            if (overflow || pow_val > n)
                hi = mid - 1;
            else
                lo = mid + 1;
        }
    }
    return false;
}

/* Modular square root of -1: find x s.t. x² ≡ -1 ≡ p-1 (mod p)
 * For p ≡ 1 mod 4, use Tonelli-Shanks. For p ≡ 3 mod 4: x = a^((p+1)/4).
 * Returns -1 if p ≡ 3 mod 4 or no solution.
 * Reference: Hardy & Wright, Theorem 82 */
long long sqrt_minus_one_mod_prime(long long p) {
    if (p == 2) return 1;
    if (p % 4 == 3) return -1; /* -1 is non-residue */
    return tonelli_shanks(p - 1, p);
}
