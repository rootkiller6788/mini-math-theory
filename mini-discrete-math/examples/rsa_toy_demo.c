#include "number.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("====== 02 数论 ======\n\n");

    // 1. GCD
    printf("--- 最大公约数 ---\n");
    printf("gcd(1071, 462) = %lld\n", gcd(1071, 462));
    printf("gcd(252, 105)  = %lld\n\n", gcd(252, 105));

    // 2. Extended GCD (Bezout)
    printf("--- 扩展欧几里得 ---\n");
    long long x, y;
    long long g = egcd(240, 46, &x, &y);
    printf("240*%lld + 46*%lld = %lld (gcd=%lld)\n\n", x, y, 240*x+46*y, g);

    // 3. 模逆元
    printf("--- 模逆元 ---\n");
    long long inv = mod_inverse(7, 26);
    printf("7^(-1) mod 26 = %lld  (验证: 7*%lld mod 26 = %lld)\n\n",
           inv, inv, (7 * inv) % 26);

    // 4. 快速幂
    printf("--- 快速幂取模 ---\n");
    printf("3^100 mod 7 = %lld\n", mod_pow(3, 100, 7));
    printf("2^10 mod 1000 = %lld\n\n", mod_pow(2, 10, 1000));

    // 5. 素数检测
    printf("--- 素数检测 ---\n");
    printf("17 是素数? %s\n", is_prime(17) ? "是" : "否");
    printf("91 是素数? %s\n", is_prime(91) ? "是" : "否");
    printf("97 是素数? %s\n\n", is_prime(97) ? "是" : "否");

    // 6. 欧拉函数
    printf("--- 欧拉函数 φ(n) ---\n");
    printf("φ(6) = %lld\n", euler_totient(6));
    printf("φ(15) = %lld\n", euler_totient(15));
    printf("φ(7) = %lld\n\n", euler_totient(7));

    // 7. RSA 加解密
    printf("--- RSA 演示 ---\n");
    long long p = 61, q = 53;
    long long n, e, d;
    rsa_generate_keys(&n, &e, &d, p, q);
    printf("p=%lld, q=%lld\n", p, q);
    printf("n=%lld, e=%lld, d=%lld\n", n, e, d);

    long long msg = 1234;
    printf("明文: %lld\n", msg);
    long long cipher = rsa_encrypt(msg, e, n);
    printf("密文: %lld\n", cipher);
    long long decrypted = rsa_decrypt(cipher, d, n);
    printf("解密: %lld  %s\n\n", decrypted, msg == decrypted ? "✓" : "✗");

    // 8. 质因数分解
    printf("--- 质因数分解 ---\n");
    print_prime_factorization(84);
    print_prime_factorization(1024);
    print_prime_factorization(997);

    return 0;
}
