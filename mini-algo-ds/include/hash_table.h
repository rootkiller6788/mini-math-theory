#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>

typedef struct {
    int key;
    int value;
    bool occupied;
    bool deleted;
} HashEntry;

typedef struct {
    HashEntry* table;
    int capacity;
    int size;
    double load_factor;
} HashTable;

HashTable* ht_create(int capacity);
void ht_free(HashTable* ht);
void ht_put(HashTable* ht, int key, int value);
int ht_get(HashTable* ht, int key, int default_val);
bool ht_contains(HashTable* ht, int key);
void ht_remove(HashTable* ht, int key);
int ht_size(HashTable* ht);
void ht_print(HashTable* ht);

int hash_basic(int key, int capacity);
int hash_multiplication(int key, int capacity);

#endif
