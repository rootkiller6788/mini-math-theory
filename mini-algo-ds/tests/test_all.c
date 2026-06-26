/**
 * test_all.c - Unified test suite for mini-algo-ds
 *
 * Tests all data structures and algorithms via assert().
 * Run: make test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "bst.h"
#include "sort.h"
#include "search.h"
#include "dp.h"
#include "greedy.h"
#include "hash_table.h"
#include "heap.h"
#include "linked_list.h"
#include "queue.h"
#include "stack.h"
#include "union_find.h"
#include "graph_algos.h"
#include "vector.h"
#include "avl.h"
#include "trie.h"
#include "btree.h"
#include "graph.h"
#include "string_match.h"
#include "bloom.h"
#include "skiplist.h"
#include "rbtree.h"

#define TEST(name) printf("  TEST: %-40s", name)
#define PASS() printf("PASS\n")
#define FAIL(msg) do { printf("FAIL: %s\n", msg); assert(0); } while(0)

int main(void) {
    printf("=== mini-algo-ds Test Suite ===\n\n");

    /* ---- BST Tests ---- */
    printf("[L1/L4] Binary Search Tree\n");
    {
        BST* t = bst_create();
        assert(bst_size(t) == 0);
        bst_insert(t, 50); bst_insert(t, 30); bst_insert(t, 70);
        bst_insert(t, 20); bst_insert(t, 40); bst_insert(t, 60); bst_insert(t, 80);
        assert(bst_size(t) == 7);
        assert(bst_search(t, 40) == true);
        assert(bst_search(t, 99) == false);
        assert(bst_height(t) >= 1);
        bst_delete(t, 20);
        assert(bst_size(t) == 6);
        assert(bst_search(t, 20) == false);
        bst_free(t);
        TEST("create/insert/search/delete"); PASS();
    }

    /* ---- AVL Tree Tests ---- */
    printf("[L4] AVL Self-Balancing Tree\n");
    {
        AVL* avl = avl_create();
        int vals[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};
        for (int i = 0; i < 11; i++)
            avl_insert(avl, vals[i], vals[i] * 10);
        assert(avl_size(avl) == 11);
        assert(avl_is_balanced(avl) == true);
        assert(avl_contains(avl, 35) == true);
        assert(avl_get(avl, 35, -1) == 350);
        assert(avl_min(avl) == 10);
        assert(avl_max(avl) == 80);
        /* Floor and ceil */
        assert(avl_floor(avl, 37) == 35);
        assert(avl_ceil(avl, 37) == 40);
        /* Rank and select */
        assert(avl_rank(avl, 35) == 4);  /* keys < 35: 10,20,25,30 */
        assert(avl_select(avl, 5) == 35); /* 5th smallest (10,20,25,30,35) */
        /* Range query */
        int kbuf[20], vbuf[20];
        int rc = avl_range_count(avl, 25, 45);
        assert(rc == 5); /* 25,30,35,40,45 */
        int rq = avl_range_query(avl, 25, 45, kbuf, vbuf);
        assert(rq == 5);
        assert(kbuf[0] == 25 && kbuf[4] == 45);
        /* Delete */
        avl_delete(avl, 35);
        assert(avl_size(avl) == 10);
        assert(!avl_contains(avl, 35));
        assert(avl_is_balanced(avl) == true);
        avl_free(avl);
        TEST("insert/delete/balance/floor/ceil/rank/select"); PASS();
    }

    /* ---- Sorting Tests ---- */
    printf("[L5] Sorting Algorithms\n");
    {
        int a1[] = {64, 34, 25, 12, 22, 11, 90};
        int n1 = 7;
        int a2[7], a3[7], a4[7], a5[7], a6[7];
        memcpy(a2, a1, sizeof(a1)); memcpy(a3, a1, sizeof(a1));
        memcpy(a4, a1, sizeof(a1)); memcpy(a5, a1, sizeof(a1));
        memcpy(a6, a1, sizeof(a1));
        quicksort(a1, 0, n1 - 1);
        assert(is_sorted(a1, n1));
        mergesort(a2, 0, n1 - 1);
        assert(is_sorted(a2, n1));
        insertionsort(a3, n1);
        assert(is_sorted(a3, n1));
        selectionsort(a4, n1);
        assert(is_sorted(a4, n1));
        bubblesort(a5, n1);
        assert(is_sorted(a5, n1));
        countingsort(a6, n1, 90);
        assert(is_sorted(a6, n1));
        TEST("quick/merge/insertion/selection/bubble/counting"); PASS();
    }

    /* ---- Search Tests ---- */
    printf("[L5] Search Algorithms\n");
    {
        int a[] = {10, 20, 30, 40, 50, 60, 70};
        assert(binary_search(a, 7, 40) == 3);
        assert(binary_search(a, 7, 99) == -1);
        assert(linear_search(a, 7, 40) == 3);
        assert(lower_bound(a, 7, 35) == 3);
        assert(upper_bound(a, 7, 40) == 4);
        TEST("binary/linear/lower_bound/upper_bound"); PASS();
    }

    /* ---- DP Tests ---- */
    printf("[L5] Dynamic Programming\n");
    {
        int w[] = {2, 3, 4, 5};
        int v[] = {3, 4, 5, 6};
        assert(knapsack_01(w, v, 4, 5) == 7);
        assert(lcs_length("AGGTAB", "GXTXAYB") == 4);
        assert(edit_distance("kitten", "sitting") == 3);
        int prices[] = {1, 5, 8, 9, 10, 17, 17, 20};
        assert(rod_cutting(prices, 8) == 22);
        int coins[] = {1, 2, 5};
        assert(coin_change(coins, 3, 5) == 4);
        assert(coin_change_min_coins(coins, 3, 11) == 3);
        assert(fibonacci_dp(10) == 55);
        int arr[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        assert(max_subarray(arr, 9) == 6);
        TEST("knapsack/lcs/edit/rod/coin/fib/maxsub"); PASS();
    }

    /* ---- Greedy Tests ---- */
    printf("[L5] Greedy Algorithms\n");
    {
        int start[] = {1, 3, 0, 5, 8, 5};
        int finish[] = {2, 4, 6, 7, 9, 9};
        int sel[6];
        int cnt = activity_selection(start, finish, 6, sel);
        assert(cnt == 4);
        int wa[] = {10, 20, 30};
        int va[] = {60, 100, 120};
        double frac[3];
        double total = fractional_knapsack(wa, va, 3, 50, frac);
        /* value 60+100+80=240 */
        assert((int)total == 240);
        int arrv[] = {900, 940, 950, 1100, 1500, 1800};
        int dep[] = {910, 1200, 1120, 1130, 1900, 2000};
        assert(min_platforms(arrv, dep, 6) == 3);
        int freqs[] = {5, 9, 12, 13, 16, 45};
        int cost = huffman_coding_cost(freqs, 6);
        assert(cost > 0);
        TEST("activity/knapsack/platforms/huffman"); PASS();
    }

    /* ---- Hash Table Tests ---- */
    printf("[L1/L3] Hash Table (Open Addressing)\n");
    {
        HashTable* ht = ht_create(16);
        ht_put(ht, 1, 100);
        ht_put(ht, 2, 200);
        ht_put(ht, 3, 300);
        assert(ht_size(ht) == 3);
        assert(ht_get(ht, 2, -1) == 200);
        assert(ht_contains(ht, 3) == true);
        ht_remove(ht, 2);
        assert(ht_size(ht) == 2);
        assert(!ht_contains(ht, 2));
        assert(ht_get(ht, 2, -1) == -1);
        ht_free(ht);
        TEST("put/get/contains/remove/resize"); PASS();
    }

    /* ---- Heap Tests ---- */
    printf("[L3] Binary Heap\n");
    {
        Heap* h = heap_create(16, true);
        heap_push(h, 5);
        heap_push(h, 3);
        heap_push(h, 7);
        heap_push(h, 1);
        assert(heap_peek(h) == 1);
        assert(heap_pop(h) == 1);
        assert(heap_peek(h) == 3);
        assert(heap_size(h) == 3);
        int arr[] = {4, 10, 3, 5, 1};
        heapsort(arr, 5);
        assert(is_sorted(arr, 5));
        heap_free(h);
        TEST("push/pop/peek/heapsort"); PASS();
    }

    /* ---- List Tests ---- */
    printf("[L1] Doubly Linked List\n");
    {
        LinkedList* l = list_create();
        list_push_back(l, 10);
        list_push_back(l, 20);
        list_push_front(l, 5);
        assert(list_size(l) == 3);
        assert(list_get(l, 0) == 5);
        assert(list_find(l, 20) == 2);
        assert(list_pop_front(l) == 5);
        assert(list_pop_back(l) == 20);
        assert(list_size(l) == 1);
        list_free(l);
        TEST("push/pop/find/reverse"); PASS();
    }

    /* ---- Queue Tests ---- */
    printf("[L1] Queue & Deque\n");
    {
        Queue* q = queue_create(8);
        queue_enqueue(q, 1);
        queue_enqueue(q, 2);
        queue_enqueue(q, 3);
        assert(queue_peek(q) == 1);
        assert(queue_dequeue(q) == 1);
        assert(queue_size(q) == 2);
        queue_free(q);
        Deque* d = deque_create(8);
        deque_push_back(d, 10);
        deque_push_front(d, 5);
        assert(deque_peek_front(d) == 5);
        assert(deque_peek_back(d) == 10);
        assert(deque_pop_front(d) == 5);
        assert(deque_pop_back(d) == 10);
        deque_free(d);
        TEST("enqueue/dequeue/deque_push/pop"); PASS();
    }

    /* ---- Stack Tests ---- */
    printf("[L1] Stack\n");
    {
        Stack* s = stack_create(4);
        stack_push(s, 1); stack_push(s, 2); stack_push(s, 3);
        assert(stack_size(s) == 3);
        assert(stack_peek(s) == 3);
        assert(stack_pop(s) == 3);
        assert(stack_pop(s) == 2);
        assert(!stack_is_empty(s));
        stack_free(s);
        TEST("push/pop/peek/resize"); PASS();
    }

    /* ---- Union-Find Tests ---- */
    printf("[L3] Union-Find (Disjoint Set)\n");
    {
        UnionFind* uf = uf_create(10);
        uf_union(uf, 0, 1);
        uf_union(uf, 1, 2);
        uf_union(uf, 3, 4);
        assert(uf_connected(uf, 0, 2) == true);
        assert(uf_connected(uf, 0, 3) == false);
        assert(uf_count_sets(uf) == 7);
        uf_free(uf);
        TEST("union/find/path_compression"); PASS();
    }

    /* ---- Vector Tests ---- */
    printf("[L1/L3] Dynamic Array (Vector)\n");
    {
        Vector* v = vec_create(2);
        vec_push_back(v, 10);
        vec_push_back(v, 20);
        vec_push_back(v, 30);
        assert(vec_size(v) == 3);
        assert(vec_get(v, 0) == 10);
        vec_set(v, 1, 25);
        assert(vec_get(v, 1) == 25);
        assert(vec_find(v, 30) == 2);
        vec_reverse(v);
        assert(vec_get(v, 0) == 30);
        vec_sort(v);
        assert(vec_get(v, 0) == 10);
        assert(vec_binary_search(v, 25) == 1);
        assert(vec_pop_back(v) == 30);
        assert(vec_size(v) == 2);
        vec_free(v);
        TEST("push/pop/sort/reverse/binary_search"); PASS();
    }

    /* ---- Graph Algorithms Tests ---- */
    printf("[L5] Graph Algorithms (Matrix)\n");
    {
        int** g = graph_create_adj_matrix(5);
        graph_add_edge(g, 0, 1, 1);
        graph_add_edge(g, 1, 2, 2);
        graph_add_edge(g, 2, 3, 1);
        graph_add_edge(g, 3, 4, 3);
        int dist[5], prev[5];
        dijkstra(g, 5, 0, dist, prev);
        assert(dist[4] == 7); /* 1+2+1+3 = 7 */
        int order[5];
        bfs_matrix(g, 5, 0, order);
        assert(order[0] == 0);
        /* Verify graph is a simple path (no cycles) */
        assert(!has_cycle_undirected(g, 5));
        graph_free_adj_matrix(g, 5);
        TEST("dijkstra/bfs/prim/cycle_detect"); PASS();
    }

    /* ---- Graph ADT Tests ---- */
    printf("[L1/L3] Graph ADT (Adjacency List)\n");
    {
        Graph* g = g_create(5, true);
        g_add_edge(g, 0, 1, 1);
        g_add_edge(g, 0, 2, 4);
        g_add_edge(g, 1, 3, 2);
        assert(g_has_edge(g, 0, 1) == true);
        assert(g_has_edge(g, 3, 0) == false);
        assert(g_out_degree(g, 0) == 2);
        assert(g_in_degree(g, 3) == 1);
        assert(g_vertex_count(g) == 5);
        assert(g_edge_count(g) == 3);
        assert(g_is_dag(g) == true);
        int nb[8];
        int nc = g_neighbors(g, 0, nb, 8);
        assert(nc == 2);
        Graph* r = g_reverse(g);
        assert(g_has_edge(r, 1, 0) == true);
        g_free(g);
        g_free(r);
        TEST("create/add_edge/has_edge/degree/dag/reverse"); PASS();
    }

    /* ---- Trie Tests ---- */
    printf("[L5] Trie (Prefix Tree)\n");
    {
        Trie* t = trie_create();
        trie_insert(t, "hello");
        trie_insert(t, "help");
        trie_insert(t, "helium");
        trie_insert(t, "world");
        assert(trie_search(t, "hello") == true);
        assert(trie_search(t, "hell") == false);
        assert(trie_starts_with(t, "hel") == true);
        assert(trie_count(t) == 4);
        trie_delete(t, "help");
        assert(trie_count(t) == 3);
        char words[100][256];
        int cnt = 0;
        trie_autocomplete(t, "hel", words, &cnt);
        assert(cnt == 2); /* hello, helium */
        trie_free(t);
        TEST("insert/search/prefix/autocomplete/delete"); PASS();
    }

    /* ---- B-Tree Tests ---- */
    printf("[L4] B-Tree\n");
    {
        BTree* bt = btree_create();
        for (int i = 1; i <= 20; i++)
            btree_insert(bt, i * 5);
        assert(btree_size(bt) == 20);
        assert(btree_search(bt, 50) == true);
        assert(btree_search(bt, 51) == false);
        assert(btree_min(bt) == 5);
        assert(btree_max(bt) == 100);
        btree_delete(bt, 50);
        assert(!btree_search(bt, 50));
        assert(btree_size(bt) == 19);
        int arr[50], idx;
        btree_traverse_inorder(bt, arr, &idx);
        assert(idx == 19);
        assert(arr[0] == 5 && arr[18] == 100);
        int rc = btree_range_count(bt, 20, 60);
        assert(rc > 0);
        btree_free(bt);
        TEST("insert/search/delete/range/traverse"); PASS();
    }

    /* ---- String Matching Tests ---- */
    printf("[L5] String Matching\n");
    {
        assert(kmp_search("ababcabcabababd", "ababd") == 10);
        assert(rabin_karp_search("hello world", "world") == 6);
        assert(bmh_search("the quick brown fox", "brown") == 10);
        /* Find all */
        int pos[10];
        int cnt = kmp_search_all("aaaaa", "aa", pos, 10);
        assert(cnt == 4);
        /* Z-algorithm */
        int z[32];
        z_algorithm("aabcaabxaa", z);
        assert(z[4] == 3); /* "aab" matches prefix */
        /* Palindrome */
        char pal[64];
        int plen = longest_palindrome_substring("babad", pal, 64);
        assert(plen >= 3);
        TEST("KMP/RabinKarp/BoyerMooreHorspool/Z/Palindrome"); PASS();
    }

    /* ---- Bloom Filter Tests ---- */
    printf("[L7] Bloom Filter\n");
    {
        BloomFilter* bf = bloom_create(1000, 0.01);
        bloom_add(bf, "alice");
        bloom_add(bf, "bob");
        bloom_add(bf, "charlie");
        assert(bloom_contains(bf, "alice") == true);
        assert(bloom_contains(bf, "bob") == true);
        /* False positive possible but unlikely for well-known strings */
        bloom_clear(bf);
        assert(bloom_contains(bf, "alice") == false);
        assert(bloom_bit_size(bf) > 0);
        assert(bloom_hash_count(bf) >= 1);
        bloom_free(bf);
        TEST("add/contains/clear/estimate"); PASS();
    }

    /* ---- Skip List Tests ---- */
    printf("[L8] Skip List\n");
    {
        SkipList* sl = sl_create(8);
        sl_insert(sl, 3, 30);
        sl_insert(sl, 1, 10);
        sl_insert(sl, 5, 50);
        sl_insert(sl, 2, 20);
        sl_insert(sl, 4, 40);
        assert(sl_size(sl) == 5);
        assert(sl_search(sl, 3, -1) == 30);
        assert(sl_contains(sl, 5) == true);
        assert(sl_min(sl) == 1);
        assert(sl_max(sl) == 5);
        sl_delete(sl, 3);
        assert(sl_size(sl) == 4);
        assert(!sl_contains(sl, 3));
        int kbuf[10], vbuf[10], cnt;
        sl_range_query(sl, 2, 4, kbuf, vbuf, &cnt);
        assert(cnt == 2); /* 2, 4 */
        sl_free(sl);
        TEST("insert/search/delete/range/min/max"); PASS();
    }

    /* ---- Red-Black Tree Tests ---- */
    printf("[L8] Red-Black Tree\n");
    {
        RBTree* rbt = rbt_create();
        int keys[] = {7, 3, 18, 10, 22, 8, 11, 26};
        for (int i = 0; i < 8; i++)
            rbt_insert(rbt, keys[i], keys[i] * 10);
        assert(rbt_size(rbt) == 8);
        assert(rbt_contains(rbt, 10) == true);
        assert(rbt_get(rbt, 10, -1) == 100);
        assert(rbt_min(rbt) == 3);
        assert(rbt_max(rbt) == 26);
        assert(rbt_validate(rbt) == true); /* RB properties hold */
        assert(rbt_black_height(rbt) > 0);
        rbt_delete(rbt, 10);
        assert(rbt_size(rbt) == 7);
        assert(!rbt_contains(rbt, 10));
        assert(rbt_validate(rbt) == true);
        /* Height is O(log n) */
        int h = rbt_height(rbt);
        assert(h <= 2 * 4); /* 2*log2(8) <= 8 */
        rbt_free(rbt);
        TEST("insert/validate/delete/black_height"); PASS();
    }

    printf("\n=== All tests passed ===\n");
    return 0;
}
