#include "relation.h"
#include "combo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void relation_init(Relation* r, int size) {
    r->size = size;
    memset(r->matrix, 0, sizeof(r->matrix));
}

void relation_add_pair(Relation* r, int a, int b) {
    if (a >= 0 && a < r->size && b >= 0 && b < r->size)
        r->matrix[a][b] = 1;
}

bool relation_has(Relation* r, int a, int b) {
    return a >= 0 && a < r->size && b >= 0 && b < r->size && r->matrix[a][b];
}

void relation_print_matrix(Relation* r) {
    printf("  ");
    for (int j = 0; j < r->size; j++) printf("%d ", j);
    printf("\n");
    for (int i = 0; i < r->size; i++) {
        printf("%d ", i);
        for (int j = 0; j < r->size; j++)
            printf("%d ", r->matrix[i][j]);
        printf("\n");
    }
}

void relation_print_pairs(Relation* r) {
    printf("{");
    int first = 1;
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j]) {
                if (!first) printf(", ");
                printf("(%d,%d)", i, j);
                first = 0;
            }
    printf("}\n");
}

bool relation_is_reflexive(Relation* r) {
    for (int i = 0; i < r->size; i++)
        if (!r->matrix[i][i]) return false;
    return true;
}

bool relation_is_symmetric(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j] && !r->matrix[j][i]) return false;
    return true;
}

bool relation_is_transitive(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j])
                for (int k = 0; k < r->size; k++)
                    if (r->matrix[j][k] && !r->matrix[i][k])
                        return false;
    return true;
}

bool relation_is_antisymmetric(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (i != j && r->matrix[i][j] && r->matrix[j][i])
                return false;
    return true;
}

bool relation_is_equivalence(Relation* r) {
    return relation_is_reflexive(r) && relation_is_symmetric(r) && relation_is_transitive(r);
}

bool relation_is_partial_order(Relation* r) {
    return relation_is_reflexive(r) && relation_is_antisymmetric(r) && relation_is_transitive(r);
}

Relation relation_reflexive_closure(Relation* r) {
    Relation result = *r;
    for (int i = 0; i < r->size; i++)
        result.matrix[i][i] = 1;
    return result;
}

Relation relation_symmetric_closure(Relation* r) {
    Relation result = *r;
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j])
                result.matrix[j][i] = 1;
    return result;
}

Relation relation_transitive_closure(Relation* r) {
    Relation result = *r;

    for (int k = 0; k < r->size; k++)
        for (int i = 0; i < r->size; i++)
            for (int j = 0; j < r->size; j++)
                if (result.matrix[i][k] && result.matrix[k][j])
                    result.matrix[i][j] = 1;
    return result;
}

EquivalencePartition equivalence_classes(Relation* r) {
    EquivalencePartition ep;
    ep.nclasses = 0;
    bool visited[MAX_REL_SIZE] = {false};

    for (int i = 0; i < r->size; i++) {
        if (visited[i]) continue;
        ep.class_sizes[ep.nclasses] = 0;
        for (int j = 0; j < r->size; j++) {
            if (!visited[j] && r->matrix[i][j]) {
                visited[j] = true;
                ep.classes[ep.nclasses][ep.class_sizes[ep.nclasses]++] = j;
            }
        }
        ep.nclasses++;
    }
    return ep;
}

void hasse_diagram_print(Relation* r) {
    if (!relation_is_partial_order(r)) {
        printf("不是偏序关系，无法绘制 Hasse 图\n");
        return;
    }

    int cover[MAX_REL_SIZE][MAX_REL_SIZE] = {{0}};
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (i != j && r->matrix[i][j]) {
                cover[i][j] = 1;
                for (int k = 0; k < r->size; k++)
                    if (k != i && k != j && r->matrix[i][k] && r->matrix[k][j]) {
                        cover[i][j] = 0;
                        break;
                    }
            }

    printf("Hasse 图的覆盖边:\n");
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (cover[i][j])
                printf("  %d → %d\n", i, j);
}

int* topological_sort(Relation* r) {
    int indegree[MAX_REL_SIZE] = {0};
    int* order = malloc(r->size * sizeof(int));
    int oidx = 0;

    for (int j = 0; j < r->size; j++)
        for (int i = 0; i < r->size; i++)
            if (i != j && r->matrix[i][j])
                indegree[j]++;

    int queue[MAX_REL_SIZE];
    int qhead = 0, qtail = 0;
    for (int i = 0; i < r->size; i++)
        if (indegree[i] == 0)
            queue[qtail++] = i;

    while (qhead < qtail) {
        int u = queue[qhead++];
        order[oidx++] = u;
        for (int v = 0; v < r->size; v++)
            if (u != v && r->matrix[u][v]) {
                indegree[v]--;
                if (indegree[v] == 0)
                    queue[qtail++] = v;
            }
    }
    return order;
}

/* ===== Lattice Theory ===== */

/* Check if relation is a lattice: for all a, b, unique least upper bound exists.
 * In a PO, x is an upper bound of a,b iff a≤x and b≤x.
 * The join (supremum) is the least upper bound.
 * The meet (infimum) is the greatest lower bound.
 * Reference: Birkhoff (1940), Lattice Theory, MIT 6.042J */

bool is_lattice(Relation* r) {
    if (!relation_is_partial_order(r)) return false;
    int n = r->size;
    for (int a = 0; a < n; a++) {
        for (int b = 0; b < n; b++) {
            /* find upper bounds of {a,b} */
            int ubs[MAX_REL_SIZE], nub = 0;
            for (int x = 0; x < n; x++) {
                if (r->matrix[a][x] && r->matrix[b][x])
                    ubs[nub++] = x;
            }
            if (nub == 0) return false; /* no upper bound */
            /* find least upper bound */
            int lub = -1;
            for (int i = 0; i < nub; i++) {
                int candidate = ubs[i];
                bool is_least = true;
                for (int j = 0; j < nub; j++) {
                    if (i != j && r->matrix[ubs[j]][candidate] && !r->matrix[candidate][ubs[j]])
                        { is_least = false; break; }
                }
                if (is_least) { lub = candidate; break; }
            }
            if (lub == -1) return false;
        }
    }
    return true;
}

int lattice_join(Relation* r, int a, int b) {
    if (a < 0 || a >= r->size || b < 0 || b >= r->size) return -1;
    int ubs[MAX_REL_SIZE], nub = 0;
    for (int x = 0; x < r->size; x++)
        if (r->matrix[a][x] && r->matrix[b][x])
            ubs[nub++] = x;
    if (nub == 0) return -1;
    /* find minimum in upper bounds */
    for (int i = 0; i < nub; i++) {
        bool is_min = true;
        for (int j = 0; j < nub; j++)
            if (i != j && r->matrix[ubs[j]][ubs[i]] && !r->matrix[ubs[i]][ubs[j]])
                { is_min = false; break; }
        if (is_min) return ubs[i];
    }
    return -1;
}

int lattice_meet(Relation* r, int a, int b) {
    if (a < 0 || a >= r->size || b < 0 || b >= r->size) return -1;
    int lbs[MAX_REL_SIZE], nlb = 0;
    for (int x = 0; x < r->size; x++)
        if (r->matrix[x][a] && r->matrix[x][b])
            lbs[nlb++] = x;
    if (nlb == 0) return -1;
    /* find maximum in lower bounds */
    for (int i = 0; i < nlb; i++) {
        bool is_max = true;
        for (int j = 0; j < nlb; j++)
            if (i != j && r->matrix[lbs[i]][lbs[j]] && !r->matrix[lbs[j]][lbs[i]])
                { is_max = false; break; }
        if (is_max) return lbs[i];
    }
    return -1;
}

/* Generate Boolean lattice B^n (power set lattice under ⊆)
 * Elements 0..2^n-1 correspond to subsets of {0,1,...,n-1}.
 * Relation structure: x ≤ y iff x ⊆ y (bitwise: (x & y) == x). */
void boolean_lattice_generate(int n, Relation* r) {
    int size = 1 << n;
    relation_init(r, size);
    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++)
            if ((x & y) == x) /* x ⊆ y */
                r->matrix[x][y] = 1;
}

/* Distributive lattice: x∧(y∨z) = (x∧y)∨(x∧z) for all x,y,z.
 * Checks identity for all triples in the lattice.
 * Reference: Birkhoff's representation theorem */
bool is_distributive_lattice(Relation* r) {
    if (!is_lattice(r)) return false;
    int n = r->size;
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            for (int z = 0; z < n; z++) {
                int jyz = lattice_join(r, y, z);     /* y ∨ z */
                int lhs = lattice_meet(r, x, jyz);   /* x ∧ (y ∨ z) */
                int mxy = lattice_meet(r, x, y);     /* x ∧ y */
                int mxz = lattice_meet(r, x, z);     /* x ∧ z */
                int rhs = lattice_join(r, mxy, mxz); /* (x ∧ y) ∨ (x ∧ z) */
                if (lhs != rhs) return false;
            }
        }
    }
    return true;
}

/* Complemented lattice: for every element x, ∃x' s.t. x∧x'=⊥ and x∨x'=⊤.
 * ⊥ = minimum element, ⊤ = maximum element.
 * Reference: Birkhoff (1940) */
bool is_complemented_lattice(Relation* r) {
    if (!is_lattice(r)) return false;
    int n = r->size;
    /* find bottom (min) and top (max) */
    int bottom = -1, top = -1;
    for (int x = 0; x < n; x++) {
        bool is_bot = true, is_top = true;
        for (int y = 0; y < n; y++) {
            if (x != y && !r->matrix[x][y]) is_bot = false;
            if (x != y && !r->matrix[y][x]) is_top = false;
        }
        if (is_bot) bottom = x;
        if (is_top) top = x;
    }
    if (bottom == -1 || top == -1) return false;

    for (int x = 0; x < n; x++) {
        bool has_compl = false;
        for (int xp = 0; xp < n; xp++) {
            int m = lattice_meet(r, x, xp);
            int j = lattice_join(r, x, xp);
            if (m == bottom && j == top) { has_compl = true; break; }
        }
        if (!has_compl) return false;
    }
    return true;
}

/* Dilworth's Theorem: In any finite PO, max antichain size = min chain decomposition.
 * We find max antichain via minimum path cover in a bipartite graph
 * constructed from the comparability graph.
 * Reference: Dilworth (1950), Annals of Math. */
int dilworth_max_antichain(Relation* r) {
    int n = r->size;
    if (!relation_is_partial_order(r)) return -1;
    /* König's theorem: max matching in bipartite graph gives min vertex cover.
     * Build bipartite graph: left=L, right=R, edge (u_L, v_R) if u < v. */
    bool adj[MAX_REL_SIZE][MAX_REL_SIZE] = {{false}};
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++)
            if (u != v && r->matrix[u][v])
                adj[u][v] = true;

    /* Find max matching (DFS augmenting paths) */
    int matchR[MAX_REL_SIZE];
    for (int i = 0; i < n; i++) matchR[i] = -1;
    int matching = 0;
    for (int u = 0; u < n; u++) {
        bool seen[MAX_REL_SIZE] = {false};
        /* DFS for augmenting path */
        int stack[MAX_REL_SIZE], top = 0;
        int parent[MAX_REL_SIZE];
        for (int i = 0; i < n; i++) parent[i] = -1;
        stack[top++] = u;
        seen[u] = true;
        bool found = false;
        int target = -1;
        while (top > 0 && !found) {
            int ul = stack[--top];
            for (int v = 0; v < n; v++) {
                if (adj[ul][v] && !seen[n + v]) {
                    seen[n + v] = true;
                    parent[n + v] = ul;
                    if (matchR[v] == -1) {
                        target = v;
                        found = true;
                        break;
                    } else if (!seen[matchR[v]]) {
                        parent[matchR[v]] = n + v;
                        seen[matchR[v]] = true;
                        stack[top++] = matchR[v];
                    }
                }
            }
        }
        if (found) {
            /* augment */
            int v = target;
            while (v != -1) {
                int ul = parent[n + v];
                int prev_v = (parent[ul] >= n) ? parent[ul] - n : -1;
                matchR[v] = ul;
                v = prev_v;
            }
            matching++;
        }
    }

    /* Dilworth: max antichain = n - size_of_min_chain_decomposition = n - max_matching */
    return n - matching;
}

/* Sperner's Theorem: The largest antichain in B^n has size C(n, floor(n/2)).
 * Reference: Sperner (1928), MIT 6.042J §15 */
long long sperner_bound(int n) {
    return combination(n, n / 2);
}

/* Kleene closure: = (R ∪ I)^* (reflexive-transitive then symmetric?)
 * Actually: Kleene star R* = reflexive transitive closure.
 * Kleene closure in relation algebra: R^* = ∪_{k≥0} R^k. */
Relation kleene_closure(Relation* r) {
    Relation result = relation_transitive_closure(r);
    /* make reflexive */
    for (int i = 0; i < result.size; i++)
        result.matrix[i][i] = 1;
    return result;
}

/* Warshall's transitive closure algorithm (explicit matrix version).
 * O(n³). Reference: Warshall (1962), JACM */
void warshall_transitive_closure(Relation* r, int result[MAX_REL_SIZE][MAX_REL_SIZE]) {
    int n = r->size;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i][j] = r->matrix[i][j];
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (result[i][k] && result[k][j])
                    result[i][j] = 1;
}

/* Relation composition: R∘S = {(a,c) | ∃b: (a,b)∈R ∧ (b,c)∈S}
 * Reference: Tarski (1941), Relation Algebra */
Relation relation_compose(Relation* r, Relation* s) {
    Relation result;
    relation_init(&result, r->size);
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            for (int k = 0; k < r->size; k++)
                if (r->matrix[i][k] && s->matrix[k][j])
                    result.matrix[i][j] = 1;
    return result;
}

/* Relation inverse: R^{-1} = {(b,a) | (a,b) ∈ R}
 * Reference: MIT 6.042J §9 */
Relation relation_inverse(Relation* r) {
    Relation result;
    relation_init(&result, r->size);
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            result.matrix[j][i] = r->matrix[i][j];
    return result;
}

/* Total order (linear order): partial order where all elements comparable.
 * i.e., for all a,b, either a≤b or b≤a. */
bool relation_is_total_order(Relation* r) {
    if (!relation_is_partial_order(r)) return false;
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (i != j && !r->matrix[i][j] && !r->matrix[j][i])
                return false;
    return true;
}

/* Count linear extensions of a partial order (backtracking)
 * A linear extension is a total order consistent with the PO.
 * Reference: Brightwell-Winkler (1991), Stanley's Enum. Comb. */
static int le_count;

static void le_backtrack(Relation* r, int* perm, int pos, bool* used) {
    int n = r->size;
    if (pos == n) { le_count++; return; }
    for (int v = 0; v < n; v++) {
        if (used[v]) continue;
        /* check: all predecessors of v must already be placed */
        bool ok = true;
        for (int u = 0; u < n; u++)
            if (u != v && r->matrix[u][v] && !used[u])
                { ok = false; break; }
        if (ok) {
            used[v] = true;
            perm[pos] = v;
            le_backtrack(r, perm, pos + 1, used);
            used[v] = false;
        }
    }
}

int count_linear_extensions(Relation* r) {
    if (!relation_is_partial_order(r)) return 0;
    int n = r->size;
    if (n > 10) return -1; /* too large for brute force */
    int* perm = malloc(n * sizeof(int));
    bool* used = calloc(n, sizeof(bool));
    le_count = 0;
    le_backtrack(r, perm, 0, used);
    free(perm);
    free(used);
    return le_count;
}
