/**
 * Skip List Implementation (Pugh, 1990)
 * L8: Advanced Probabilistic Data Structure
 *
 * Uses coin-flip level generation: P(level increase) = 0.5
 * Expected height: O(log n), expected space: O(n)
 *
 * Search: O(log n) expected, worst O(n)
 * Insert/Delete: O(log n) expected with update array
 *
 * MIT 6.046J Lecture 8 / CMU 15-451
 */

#include "skiplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static int rand_level(int max_level) {
    int level = 1;
    while ((rand() & 1) && level < max_level) level++;
    return level;
}

SkipList* sl_create(int max_level) {
    SkipList* sl = (SkipList*)malloc(sizeof(SkipList));
    sl->max_level = max_level > 0 ? max_level : SKIPLIST_MAX_LEVEL;
    sl->level = 1;
    sl->size = 0;
    sl->header = (SkipNode*)malloc(sizeof(SkipNode));
    sl->header->key = -1;
    sl->header->value = 0;
    sl->header->forward = (SkipNode**)calloc((size_t)sl->max_level, sizeof(SkipNode*));
    sl->header->level = sl->max_level;
    return sl;
}

void sl_free(SkipList* sl) {
    SkipNode* cur = sl->header->forward[0];
    while (cur) {
        SkipNode* next = cur->forward[0];
        free(cur->forward);
        free(cur);
        cur = next;
    }
    free(sl->header->forward);
    free(sl->header);
    free(sl);
}

void sl_insert(SkipList* sl, int key, int value) {
    SkipNode* update[SKIPLIST_MAX_LEVEL];
    SkipNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; i--) {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }
    cur = cur->forward[0];
    if (cur && cur->key == key) {
        cur->value = value;  /* update existing */
        return;
    }
    int new_level = rand_level(sl->max_level);
    if (new_level > sl->level) {
        for (int i = sl->level; i < new_level; i++)
            update[i] = sl->header;
        sl->level = new_level;
    }
    SkipNode* node = (SkipNode*)malloc(sizeof(SkipNode));
    node->key = key;
    node->value = value;
    node->level = new_level;
    node->forward = (SkipNode**)calloc((size_t)new_level, sizeof(SkipNode*));
    for (int i = 0; i < new_level; i++) {
        node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = node;
    }
    sl->size++;
}

bool sl_delete(SkipList* sl, int key) {
    SkipNode* update[SKIPLIST_MAX_LEVEL];
    SkipNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; i--) {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }
    cur = cur->forward[0];
    if (!cur || cur->key != key) return false;
    for (int i = 0; i < sl->level; i++) {
        if (update[i]->forward[i] != cur) break;
        update[i]->forward[i] = cur->forward[i];
    }
    free(cur->forward);
    free(cur);
    while (sl->level > 1 && !sl->header->forward[sl->level - 1])
        sl->level--;
    sl->size--;
    return true;
}

int sl_search(SkipList* sl, int key, int default_val) {
    SkipNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; i--)
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
    cur = cur->forward[0];
    if (cur && cur->key == key) return cur->value;
    return default_val;
}

bool sl_contains(SkipList* sl, int key) {
    return sl_search(sl, key, -1) != -1;
}

int sl_size(SkipList* sl) { return sl->size; }

int sl_min(SkipList* sl) {
    SkipNode* n = sl->header->forward[0];
    return n ? n->key : -1;
}

int sl_max(SkipList* sl) {
    SkipNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; i--)
        while (cur->forward[i])
            cur = cur->forward[i];
    return (cur != sl->header) ? cur->key : -1;
}

void sl_range_query(SkipList* sl, int lo, int hi, int* keys, int* values, int* count) {
    *count = 0;
    SkipNode* cur = sl->header->forward[0];
    while (cur && cur->key < lo) cur = cur->forward[0];
    while (cur && cur->key <= hi) {
        if (keys) keys[*count] = cur->key;
        if (values) values[*count] = cur->value;
        (*count)++;
        cur = cur->forward[0];
    }
}

void sl_print(SkipList* sl) {
    for (int i = sl->level - 1; i >= 0; i--) {
        SkipNode* cur = sl->header->forward[i];
        printf("Level %d: ", i);
        while (cur) {
            printf("%d -> ", cur->key);
            cur = cur->forward[i];
        }
        printf("NIL\n");
    }
}
