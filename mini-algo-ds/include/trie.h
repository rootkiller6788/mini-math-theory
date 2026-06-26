/**
 * Trie (Prefix Tree) - L5: String Data Structure
 * 
 * O(L) insert/search for strings of length L.
 * Core operations: insert, search, prefix search, delete.
 * 
 * Application: autocomplete (L7), IP routing (L7), spell check
 *
 * MIT 6.006 Lecture 20: Tries
 * CLRS Problem 12-2: Radix Trees
 */

#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

#define TRIE_ALPHABET 26
#define TRIE_MAX_WORDS 1024

typedef struct TrieNode {
    struct TrieNode* children[TRIE_ALPHABET];
    bool is_end;
    int count;        /* number of words ending at this node */
} TrieNode;

typedef struct {
    TrieNode* root;
    int word_count;
} Trie;

Trie* trie_create(void);
void trie_free(Trie* t);
void trie_insert(Trie* t, const char* word);
bool trie_search(Trie* t, const char* word);
bool trie_starts_with(Trie* t, const char* prefix);
bool trie_delete(Trie* t, const char* word);
int  trie_count(Trie* t);  /* total words stored */
int  trie_count_prefix(Trie* t, const char* prefix);
void trie_autocomplete(Trie* t, const char* prefix, char words[][256], int* count);
char* trie_longest_common_prefix(Trie* t, char* buf, int bufsize);
void trie_all_words(Trie* t, char words[][256], int* count);
int  trie_node_count(Trie* t);

#endif
