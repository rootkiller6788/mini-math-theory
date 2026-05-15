#include "error_correction.h"
#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void) {
    printf("====== Error Correction Demo ======\n\n");
    srand((unsigned)time(NULL));

    printf("--- 1. Hamming(7,4) Encoding ---\n");
    bool data[4] = {1, 0, 1, 1};
    bool encoded[7];
    hamming74_encode(data, encoded);
    printf("Data:    ");
    for (int i = 0; i < 4; i++) printf("%d", data[i]);
    printf("\nEncoded: ");
    for (int i = 0; i < 7; i++) printf("%d", encoded[i]);
    printf("\n  p1=d1⊕d2⊕d4, p2=d1⊕d3⊕d4, p3=d2⊕d3⊕d4\n");
    printf("  p1=%d p2=%d p3=%d\n\n", encoded[0], encoded[1], encoded[3]);

    printf("--- 2. Hamming(7,4) Single Error Correction ---\n");
    for (int err_pos = 0; err_pos < 7; err_pos++) {
        bool received[7];
        memcpy(received, encoded, 7 * sizeof(bool));
        received[err_pos] = !received[err_pos];

        bool decoded[4];
        int result = hamming74_decode(received, decoded);
        printf("Error at bit %d: syndrome → position %d, ",
               err_pos, result);
        bool correct = (memcmp(decoded, data, 4) == 0);
        printf("%s\n", correct ? "CORRECTED ✓" : "FAIL");
    }
    printf("\n");

    printf("--- 3. Hamming(8,4) SEC-DED ---\n");
    bool enc84[8];
    hamming84_encode(data, enc84);
    printf("Hamming(8,4) encoded: ");
    for (int i = 0; i < 8; i++) printf("%d", enc84[i]);

    bool recv84[8];
    memcpy(recv84, enc84, 8);
    // Flip one bit
    recv84[3] = !recv84[3];
    bool dec84[4];
    int res84 = hamming84_decode(recv84, dec84);
    printf("\n1-bit error at pos 3: corrected? %s (%s)\n\n",
           res84 >= 0 ? "YES" : "NO",
           memcmp(dec84, data, 4) == 0 ? "data OK" : "data corrupted");

    printf("--- 4. Error Rate Simulation ---\n");
    int nblocks = 10000;
    int uncorrected = 0, corrected = 0, undetected = 0;
    for (int b = 0; b < nblocks; b++) {
        bool d[4];
        for (int i = 0; i < 4; i++) d[i] = rand() % 2;

        bool enc[7];
        hamming74_encode(d, enc);

        bool noisy[7];
        add_bit_errors(enc, 7, 0.05, noisy);

        bool dec[4];
        int result = hamming74_decode(noisy, dec);

        if (result == 0) {
            if (memcmp(d, dec, 4) == 0) corrected++; // no error needed
            else undetected++;
        } else if (result > 0) {
            corrected++;
        } else {
            uncorrected++;
        }
    }
    printf("%d blocks transmitted @ p_err=0.05:\n", nblocks);
    printf("  Corrected/clean: %d (%.1f%%)\n", corrected, 100.0 * corrected / nblocks);
    printf("  Uncorrectable:   %d (%.1f%%)\n", uncorrected, 100.0 * uncorrected / nblocks);
    printf("  Undetected:      %d (%.1f%%)\n\n", undetected, 100.0 * undetected / nblocks);

    printf("--- 5. Repetition Code R(3,1) ---\n");
    bool rep_enc[3];
    repetition_encode(1, 3, rep_enc);
    printf("Bit=1 encoded as: %d%d%d\n", rep_enc[0], rep_enc[1], rep_enc[2]);

    bool noisy_rep[3] = {1, 0, 1};
    bool decoded_rep = repetition_decode(noisy_rep, 3);
    printf("Received 1,0,1 → decoded: %d (majority vote)\n", decoded_rep);

    return 0;
}
