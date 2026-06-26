/**
 * Trie Implementation (Prefix Tree)
 * L5: String Algorithm - O(L) insert/search/delete for length L
 *
 * Each node has 26 children pointers (a-z).
 * Supports autocomplete (L7 application), prefix matching.
 *
 * MIT 6.006 Lecture 20 / CLRS Problem 12-2
 */

#include "trie.h"
#include <stdlib.h>
#include <string.h>

static TrieNode* node_create(void) {
    TrieNode* n = (TrieNode*)calloc(1, sizeof(TrieNode));
    return n;
}

Trie* trie_create(void) {
    Trie* t = (Trie*)malloc(sizeof(Trie));
    t->root = node_create();
    t->word_count = 0;
    return t;
}

static void free_nodes(TrieNode* node) {
    for (int i = 0; i < TRIE_ALPHABET; i++) {
        if (node->children[i]) free_nodes(node->children[i]);
    }
    free(node);
}

void trie_free(Trie* t) {
    if (t) { free_nodes(t->root); free(t); }
}

void trie_insert(Trie* t, const char* word) {
    TrieNode* cur = t->root;
    for (const char* p = word; *p; p++) {
        int idx = *p - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) continue;
        if (!cur->children[idx]) cur->children[idx] = node_create();
        cur = cur->children[idx];
    }
    if (!cur->is_end) { t->word_count++; }
    cur->is_end = true;
    cur->count++;
}

bool trie_search(Trie* t, const char* word) {
    TrieNode* cur = t->root;
    for (const char* p = word; *p; p++) {
        int idx = *p - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) return false;
        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return cur->is_end;
}

bool trie_starts_with(Trie* t, const char* prefix) {
    TrieNode* cur = t->root;
    for (const char* p = prefix; *p; p++) {
        int idx = *p - 'a';
        if (idx < 0 || idx >= TRIE_ALPHABET) return false;
        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return true;
}

static bool delete_rec(TrieNode* node, const char* word, int depth) {
    if (!node) return false;
    if (word[depth] == '\0') {
        if (!node->is_end) return false;
        node->is_end = false;
        node->count--;
        for (int i = 0; i < TRIE_ALPHABET; i++)
            if (node->children[i]) return false;
        return true;
    }
    int idx = word[depth] - 'a';
    if (delete_rec(node->children[idx], word, depth + 1)) {
        free(node->children[idx]);
        node->children[idx] = NULL;
        /* Check if this node is now a leaf (no children, not end-of-word) */
        if (node->is_end) return false;
        for (int i = 0; i < TRIE_ALPHABET; i++)
            if (node->children[i]) return false;
        return node->count == 0;
    }
    return false;
}

bool trie_delete(Trie* t, const char* word) {
    if (!trie_search(t, word)) return false;
    delete_rec(t->root, word, 0);
    t->word_count--;
    return true;
}

int trie_count(Trie* t) { return t->word_count; }

int trie_count_prefix(Trie* t, const char* prefix) {
    TrieNode* cur = t->root;
    for (const char* p = prefix; *p; p++) {
        int idx = *p - 'a';
        if (!cur->children[idx]) return 0;
        cur = cur->children[idx];
    }
    /* Count all words in subtree */
    int total = cur->is_end ? cur->count : 0;
    for (int i = 0; i < TRIE_ALPHABET; i++) {
        if (cur->children[i]) {
            /* Simplified: just count direct children */
            if (cur->children[i]->is_end)
                total += cur->children[i]->count;
        }
    }
    return total;
}

/* DFS to collect all words with given prefix for autocomplete */
static void collect_words(TrieNode* node, char* buf, int depth,
                          char words[][256], int* count, int max_count) {
    if (*count >= max_count) return;
    if (node->is_end && *count < max_count) {
        buf[depth] = '\0';
        strcpy(words[*count], buf);
        (*count)++;
    }
    for (int i = 0; i < TRIE_ALPHABET; i++) {
        if (node->children[i] && *count < max_count) {
            buf[depth] = (char)('a' + i);
            collect_words(node->children[i], buf, depth + 1,
                          words, count, max_count);
        }
    }
}

void trie_autocomplete(Trie* t, const char* prefix,
                       char words[][256], int* count) {
    *count = 0;
    TrieNode* cur = t->root;
    char buf[256];
    int len = 0;
    for (const char* p = prefix; *p; p++, len++) {
        int idx = *p - 'a';
        if (!cur->children[idx]) return;
        buf[len] = *p;
        cur = cur->children[idx];
    }
    collect_words(cur, buf, len, words, count, TRIE_MAX_WORDS);
}

char* trie_longest_common_prefix(Trie* t, char* buf, int bufsize) {
    TrieNode* cur = t->root;
    int depth = 0;
    while (depth < bufsize - 1) {
        int child_count = 0, child_idx = -1;
        for (int i = 0; i < TRIE_ALPHABET; i++) {
            if (cur->children[i]) { child_count++; child_idx = i; }
        }
        if (child_count != 1 || cur->is_end) break;
        buf[depth++] = (char)('a' + child_idx);
        cur = cur->children[child_idx];
    }
    buf[depth] = '\0';
    return buf;
}

void trie_all_words(Trie* t, char words[][256], int* count) {
    *count = 0;
    char buf[256];
    collect_words(t->root, buf, 0, words, count, TRIE_MAX_WORDS);
}

static int count_nodes(TrieNode* node) {
    int c = 1;
    for (int i = 0; i < TRIE_ALPHABET; i++)
        if (node->children[i]) c += count_nodes(node->children[i]);
    return c;
}

int trie_node_count(Trie* t) { return count_nodes(t->root); }
