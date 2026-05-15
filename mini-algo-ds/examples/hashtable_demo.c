#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"

int main(void) {
    printf("=== Hash Table Demo ===\n\n");

    printf("--- 1. Basic Insert & Get ---\n");
    HashTable* ht = ht_create(8);
    ht_put(ht, 1, 100);
    ht_put(ht, 2, 200);
    ht_put(ht, 9, 900);
    ht_put(ht, 16, 1600);

    printf("get(1) = %d (expect 100)\n", ht_get(ht, 1, -1));
    printf("get(2) = %d (expect 200)\n", ht_get(ht, 2, -1));
    printf("get(9) = %d (expect 900)\n", ht_get(ht, 9, -1));
    printf("get(16) = %d (expect 1600)\n", ht_get(ht, 16, -1));
    printf("get(99) = %d (expect -1, missing)\n\n", ht_get(ht, 99, -1));

    printf("--- 2. Remove ---\n");
    printf("contains(2)? %s\n", ht_contains(ht, 2) ? "yes" : "no");
    ht_remove(ht, 2);
    printf("After remove 2: contains(2)? %s\n", ht_contains(ht, 2) ? "yes" : "no");
    printf("get(2) = %d (expect -1)\n\n", ht_get(ht, 2, -1));

    printf("--- 3. Size & Print ---\n");
    printf("size = %d\n", ht_size(ht));
    ht_print(ht);
    printf("\n");

    printf("--- 4. Auto-resize Test ---\n");
    for (int i = 0; i < 20; i++)
        ht_put(ht, i * 10, i * 100);
    printf("After inserting 20 items: size = %d\n", ht_size(ht));
    bool ok = true;
    for (int i = 0; i < 20; i++)
        if (ht_get(ht, i * 10, -1) != i * 100) ok = false;
    printf("All 20 items retrievable? %s\n", ok ? "YES" : "NO");

    ht_free(ht);
    return 0;
}
