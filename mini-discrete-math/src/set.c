#include "set.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int cmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

void set_init(Set* s) {
    s->size = 0;
}

void set_add(Set* s, int x) {
    for (int i = 0; i < s->size; i++)
        if (s->elements[i] == x) return;
    s->elements[s->size++] = x;
    qsort(s->elements, s->size, sizeof(int), cmp);
}

void set_remove(Set* s, int x) {
    for (int i = 0; i < s->size; i++) {
        if (s->elements[i] == x) {
            for (int j = i; j < s->size - 1; j++)
                s->elements[j] = s->elements[j + 1];
            s->size--;
            return;
        }
    }
}

bool set_contains(Set* s, int x) {
    for (int i = 0; i < s->size; i++)
        if (s->elements[i] == x) return true;
    return false;
}

int set_size(Set* s) {
    return s->size;
}

void set_print(Set* s) {
    printf("{");
    for (int i = 0; i < s->size; i++) {
        printf("%d", s->elements[i]);
        if (i < s->size - 1) printf(", ");
    }
    printf("}");
}

Set set_union(Set* a, Set* b) {
    Set result;
    set_init(&result);
    int i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->elements[i] < b->elements[j])
            result.elements[result.size++] = a->elements[i++];
        else if (a->elements[i] > b->elements[j])
            result.elements[result.size++] = b->elements[j++];
        else {
            result.elements[result.size++] = a->elements[i++];
            j++;
        }
    }
    while (i < a->size) result.elements[result.size++] = a->elements[i++];
    while (j < b->size) result.elements[result.size++] = b->elements[j++];
    return result;
}

Set set_intersection(Set* a, Set* b) {
    Set result;
    set_init(&result);
    int i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->elements[i] < b->elements[j]) i++;
        else if (a->elements[i] > b->elements[j]) j++;
        else {
            result.elements[result.size++] = a->elements[i];
            i++; j++;
        }
    }
    return result;
}

Set set_difference(Set* a, Set* b) {
    Set result;
    set_init(&result);
    for (int i = 0; i < a->size; i++)
        if (!set_contains(b, a->elements[i]))
            result.elements[result.size++] = a->elements[i];
    return result;
}

bool set_subset(Set* a, Set* b) {
    for (int i = 0; i < a->size; i++)
        if (!set_contains(b, a->elements[i])) return false;
    return true;
}

bool set_equal(Set* a, Set* b) {
    return a->size == b->size && set_subset(a, b);
}

PowerSet power_set_generate(Set* s) {
    PowerSet ps;
    ps.count = 0;
    int total = 1 << s->size;
    for (int mask = 0; mask < total; mask++) {
        set_init(&ps.sets[mask]);
        for (int i = 0; i < s->size; i++)
            if (mask & (1 << i))
                ps.sets[mask].elements[ps.sets[mask].size++] = s->elements[i];
    }
    ps.count = total;
    return ps;
}

CartesianProduct cartesian_product(Set* a, Set* b) {
    CartesianProduct cp;
    cp.size = 0;
    for (int i = 0; i < a->size; i++)
        for (int j = 0; j < b->size; j++) {
            cp.pairs[cp.size].first = a->elements[i];
            cp.pairs[cp.size].second = b->elements[j];
            cp.size++;
        }
    return cp;
}

/* Symmetric difference: A Δ B = (A \ B) ∪ (B \ A) = (A ∪ B) \ (A ∩ B)
 * Reference: MIT 6.042J §8, Boolean algebra */
Set symmetric_difference(Set* a, Set* b) {
    Set diff1 = set_difference(a, b);
    Set diff2 = set_difference(b, a);
    return set_union(&diff1, &diff2);
}

/* De Morgan's Laws: Given universe U:
 * ¬(A ∪ B) = ¬A ∩ ¬B, i.e., complement of union = intersection of complements.
 * ¬(A ∩ B) = ¬A ∪ ¬B, i.e., complement of intersection = union of complements.
 * Reference: De Morgan (1847), MIT 6.042J §8.3 */
bool verify_de_morgan(Set* a, Set* b, Set* universe) {
    /* ¬X = universe \ X */
    Set not_a = set_difference(universe, a);
    Set not_b = set_difference(universe, b);

    /* Law 1: ¬(A ∪ B) = ¬A ∩ ¬B */
    Set un_ab = set_union(a, b);
    Set lhs1 = set_difference(universe, &un_ab);
    Set rhs1 = set_intersection(&not_a, &not_b);
    if (!set_equal(&lhs1, &rhs1)) return false;

    /* Law 2: ¬(A ∩ B) = ¬A ∪ ¬B */
    Set int_ab = set_intersection(a, b);
    Set lhs2 = set_difference(universe, &int_ab);
    Set rhs2 = set_union(&not_a, &not_b);
    if (!set_equal(&lhs2, &rhs2)) return false;

    return true;
}

/* Multiset operations
 * Reference: Knuth ACP Vol.2, Blizard (1989), "Multiset Theory" */
void multiset_init(Multiset* ms) {
    ms->distinct = 0;
}

void multiset_add(Multiset* ms, int x) {
    for (int i = 0; i < ms->distinct; i++) {
        if (ms->elements[i] == x) {
            ms->counts[i]++;
            return;
        }
    }
    if (ms->distinct < MAX_SET_SIZE) {
        ms->elements[ms->distinct] = x;
        ms->counts[ms->distinct] = 1;
        ms->distinct++;
    }
}

void multiset_remove(Multiset* ms, int x) {
    for (int i = 0; i < ms->distinct; i++) {
        if (ms->elements[i] == x && ms->counts[i] > 0) {
            ms->counts[i]--;
            if (ms->counts[i] == 0) {
                /* remove element by shifting */
                for (int j = i; j < ms->distinct - 1; j++) {
                    ms->elements[j] = ms->elements[j + 1];
                    ms->counts[j] = ms->counts[j + 1];
                }
                ms->distinct--;
            }
            return;
        }
    }
}

int multiset_count(Multiset* ms, int x) {
    for (int i = 0; i < ms->distinct; i++)
        if (ms->elements[i] == x)
            return ms->counts[i];
    return 0;
}

int multiset_total(Multiset* ms) {
    int total = 0;
    for (int i = 0; i < ms->distinct; i++)
        total += ms->counts[i];
    return total;
}

Multiset multiset_union(Multiset* a, Multiset* b) {
    Multiset result;
    multiset_init(&result);
    for (int i = 0; i < a->distinct; i++) {
        int x = a->elements[i];
        int cnt = a->counts[i];
        int b_cnt = multiset_count(b, x);
        int max_cnt = cnt > b_cnt ? cnt : b_cnt;
        result.elements[result.distinct] = x;
        result.counts[result.distinct] = max_cnt;
        result.distinct++;
    }
    for (int i = 0; i < b->distinct; i++) {
        int x = b->elements[i];
        if (multiset_count(&result, x) == 0) {
            result.elements[result.distinct] = x;
            result.counts[result.distinct] = b->counts[i];
            result.distinct++;
        }
    }
    return result;
}

Multiset multiset_intersection(Multiset* a, Multiset* b) {
    Multiset result;
    multiset_init(&result);
    for (int i = 0; i < a->distinct; i++) {
        int x = a->elements[i];
        int b_cnt = multiset_count(b, x);
        if (b_cnt > 0) {
            result.elements[result.distinct] = x;
            result.counts[result.distinct] = a->counts[i] < b_cnt ? a->counts[i] : b_cnt;
            result.distinct++;
        }
    }
    return result;
}

Multiset multiset_sum(Multiset* a, Multiset* b) {
    Multiset result = *a;
    for (int i = 0; i < b->distinct; i++) {
        int x = b->elements[i];
        bool found = false;
        for (int j = 0; j < result.distinct; j++) {
            if (result.elements[j] == x) {
                result.counts[j] += b->counts[i];
                found = true;
                break;
            }
        }
        if (!found) {
            result.elements[result.distinct] = x;
            result.counts[result.distinct] = b->counts[i];
            result.distinct++;
        }
    }
    return result;
}

/* Set partitions generation (Bell number enumeration)
 * Uses recursive algorithm: for each element, either start new block or join existing.
 * Reference: Knuth ACP Vol.4, §7.2.1.5 */
typedef struct {
    Set* orig;
    void (*callback)(SetPartition*);
} PartGenCtx;

static void part_gen_rec(PartGenCtx* ctx, SetPartition* sp, int pos) {
    int n = ctx->orig->size;
    if (pos == n) {
        ctx->callback(sp);
        return;
    }
    int elem = ctx->orig->elements[pos];

    /* Option 1: put elem in its own new block */
    int new_block = sp->n_subsets;
    sp->subsets[new_block][0] = elem;
    sp->subset_sizes[new_block] = 1;
    sp->n_subsets++;
    part_gen_rec(ctx, sp, pos + 1);
    sp->n_subsets--;

    /* Option 2: add elem to each existing block */
    for (int b = 0; b < sp->n_subsets; b++) {
        int sz = sp->subset_sizes[b];
        sp->subsets[b][sz] = elem;
        sp->subset_sizes[b] = sz + 1;
        part_gen_rec(ctx, sp, pos + 1);
        sp->subset_sizes[b] = sz; /* backtrack */
    }
}

void set_partitions_generate(Set* s, void (*callback)(SetPartition*)) {
    SetPartition sp;
    sp.n_subsets = 0;
    memset(sp.subset_sizes, 0, sizeof(sp.subset_sizes));
    PartGenCtx ctx = {s, callback};
    part_gen_rec(&ctx, &sp, 0);
}

/* Anti-chain check: no element of the power set is a proper subset of another
 * Reference: Sperner (1928), MIT 6.042J §15 */
bool is_antichain(PowerSet* ps) {
    for (int i = 0; i < ps->count; i++) {
        for (int j = i + 1; j < ps->count; j++) {
            /* skip if sizes differ, since proper ⊂ requires |A| < |B| */
            if (ps->sets[i].size == ps->sets[j].size) continue;
            if (ps->sets[i].size < ps->sets[j].size) {
                if (set_subset(&ps->sets[i], &ps->sets[j])) return false;
            } else {
                if (set_subset(&ps->sets[j], &ps->sets[i])) return false;
            }
        }
    }
    return true;
}

/* Russell's Paradox: Let R = {x | x ∉ x}. Then R ∈ R ↔ R ∉ R.
 * In ZFC, R is not a well-defined set (proper class). Demonstrates the paradox.
 * Reference: Russell (1901), MIT 6.042J §8.1 */
void russell_paradox_demo(void) {
    printf("=== Russell's Paradox ===\n");
    printf("Let R = { x | x ∉ x } (the set of all sets not containing themselves)\n");
    printf("Q: Does R ∈ R?\n\n");

    printf("If R ∈ R, then by definition R ∉ R.  [contradiction]\n");
    printf("If R ∉ R, then by definition R ∈ R.  [contradiction]\n\n");

    printf("Resolution (ZFC): R is a \"proper class\", not a set.\n");
    printf("The Axiom of Regularity (Foundation) prevents x ∈ x for any set x.\n");
    printf("Thus in ZFC, R is the universal class V, which is not a set.\n");
}
