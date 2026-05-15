#include "hash_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int hash_basic(int key, int capacity) {
    return ((key % capacity) + capacity) % capacity;
}

int hash_multiplication(int key, int capacity) {
    double A = 0.6180339887;
    double frac = key * A - (int)(key * A);
    return (int)(capacity * frac);
}

static bool is_prime(int n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

static int next_prime(int n) {
    while (!is_prime(n)) n++;
    return n;
}

HashTable* ht_create(int capacity) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    ht->capacity = next_prime(capacity);
    ht->table = (HashEntry*)calloc(ht->capacity, sizeof(HashEntry));
    ht->size = 0;
    ht->load_factor = 0.0;
    return ht;
}

void ht_free(HashTable* ht) {
    free(ht->table);
    free(ht);
}

static void ht_resize(HashTable* ht) {
    int old_capacity = ht->capacity;
    HashEntry* old_table = ht->table;

    ht->capacity = next_prime(old_capacity * 2);
    ht->table = (HashEntry*)calloc(ht->capacity, sizeof(HashEntry));
    ht->size = 0;
    ht->load_factor = 0.0;

    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i].occupied && !old_table[i].deleted) {
            ht_put(ht, old_table[i].key, old_table[i].value);
        }
    }
    free(old_table);
}

void ht_put(HashTable* ht, int key, int value) {
    if ((double)(ht->size + 1) / ht->capacity > 0.75) {
        ht_resize(ht);
    }

    int idx = hash_multiplication(key, ht->capacity);
    int first_deleted = -1;

    while (ht->table[idx].occupied) {
        if (ht->table[idx].deleted) {
            if (first_deleted == -1) first_deleted = idx;
        } else if (ht->table[idx].key == key) {
            ht->table[idx].value = value;
            return;
        }
        idx = (idx + 1) % ht->capacity;
    }

    if (first_deleted != -1) {
        idx = first_deleted;
        ht->size++;
    } else {
        ht->size++;
    }

    ht->table[idx].key = key;
    ht->table[idx].value = value;
    ht->table[idx].occupied = true;
    ht->table[idx].deleted = false;
    ht->load_factor = (double)ht->size / ht->capacity;
}

int ht_get(HashTable* ht, int key, int default_val) {
    int idx = hash_multiplication(key, ht->capacity);

    while (ht->table[idx].occupied) {
        if (!ht->table[idx].deleted && ht->table[idx].key == key) {
            return ht->table[idx].value;
        }
        idx = (idx + 1) % ht->capacity;
    }
    return default_val;
}

bool ht_contains(HashTable* ht, int key) {
    int idx = hash_multiplication(key, ht->capacity);

    while (ht->table[idx].occupied) {
        if (!ht->table[idx].deleted && ht->table[idx].key == key) {
            return true;
        }
        idx = (idx + 1) % ht->capacity;
    }
    return false;
}

void ht_remove(HashTable* ht, int key) {
    int idx = hash_multiplication(key, ht->capacity);

    while (ht->table[idx].occupied) {
        if (!ht->table[idx].deleted && ht->table[idx].key == key) {
            ht->table[idx].deleted = true;
            ht->size--;
            ht->load_factor = (double)ht->size / ht->capacity;
            return;
        }
        idx = (idx + 1) % ht->capacity;
    }
}

int ht_size(HashTable* ht) {
    return ht->size;
}

void ht_print(HashTable* ht) {
    for (int i = 0; i < ht->capacity; i++) {
        if (ht->table[i].occupied && !ht->table[i].deleted) {
            printf("[%d] %d -> %d\n", i, ht->table[i].key, ht->table[i].value);
        }
    }
}
