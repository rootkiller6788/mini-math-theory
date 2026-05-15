# Computer Science Applications of Discrete Math Modules

Each module is paired with concrete CS applications and C code examples showing
how the mathematical primitives map to real-world engineering problems.

---

## Logic → If/Else Optimization, Assertion Checking

### Application: Tautology-Based Dead Code Elimination
If a conditional expression is a tautology (always true), the else-branch is dead code and can be removed.

```c
#include "logic.h"
#include <stdio.h>
#include <stdlib.h>

// Detect: if (p || !p) { A } else { B } → always takes A
void check_dead_branch(void) {
    Formula* f = formula_create();
    f->nvars = 1;
    f->var_names[0] = 'p';
    int p = formula_add_var(f, 0);
    int np = formula_add_not(f, p);
    int root = formula_add_or(f, p, np);  // p || !p

    if (is_tautology(f, root))
        printf("OPTIMIZATION: else-branch is dead code — expression is always true\n");
    free(f);
}
```

### Application: Assertion Precondition Checking at Compile Time
A candidate precondition can be tested against all possible inputs (up to MAX_VARS) to verify it never fails.

```c
// Verify: (x >= 0) -> (sqrt_called) never contradicts
void verify_precondition(void) {
    // Build precondition: x >= 0 (encode as 3-bit unsigned: x_2 x_1 x_0)
    Formula* f = formula_create();
    f->nvars = 3;
    f->var_names[0] = 'b'; f->var_names[1] = 'a'; f->var_names[2] = 'b';
    int b0 = formula_add_var(f, 0);
    int b1 = formula_add_var(f, 1);
    int b2 = formula_add_var(f, 2);
    // All values 0-7 are >= 0, so every assignment is valid - no contradiction
    if (is_contradiction(f, b0))
        printf("WARNING: precondition is contradictory — function never called legally\n");
    free(f);
}
```

### Application: Circuit Optimization via Boolean Simplification
Reduce gate count by simplifying Boolean expressions with Karnaugh maps.

```c
#include "boolalg.h"

void optimize_control_logic(void) {
    // Example: a control signal F that's true for inputs 0,1,3,7
    BoolFunction bf = {
        .func = func_sample,  // from circuit_demo.c
        .nvars = 3,
        .var_names = {'x', 'y', 'z'}
    };
    char simplified[256];
    karnaugh_map_3var(&bf, simplified);
    // "simplified" holds minimal SOP — fewer AND/OR gates in hardware
}
```

---

## Number Theory → RSA Keygen, Hashing

### Application: Toy RSA Key Generation
Generate a public/private key pair for small-number encryption.

```c
#include "number.h"
#include <stdio.h>

void rsa_demo(void) {
    long long p = 61, q = 53;  // small primes for demo
    long long n, e, d;
    rsa_generate_keys(&n, &e, &d, p, q);

    printf("Public key:  (n=%lld, e=%lld)\n", n, e);
    printf("Private key: (n=%lld, d=%lld)\n", n, d);

    long long msg = 42;
    long long cipher = rsa_encrypt(msg, e, n);
    long long decrypted = rsa_decrypt(cipher, d, n);

    printf("Original: %lld, Encrypted: %lld, Decrypted: %lld\n",
           msg, cipher, decrypted);
    printf("Roundtrip OK: %s\n", msg == decrypted ? "YES" : "NO");
}
```

### Application: Simple Hash Function using Modular Arithmetic
Use prime modulus and modular exponentiation to build a hash function.

```c
#include "number.h"
#include <string.h>

long long simple_hash(const char* str, long long mod) {
    long long h = 0;
    long long base = 31;  // small prime base
    for (int i = 0; str[i]; i++) {
        h = (h * base + (unsigned char)str[i]) % mod;
    }
    return h;
}

long long rolling_hash_update(long long old_hash, char old_char,
                               char new_char, long long pow_base, long long mod) {
    long long h = (old_hash * 31 - (unsigned char)old_char * pow_base + (unsigned char)new_char) % mod;
    return (h + mod) % mod;
}
```

### Application: Modular Inverse for Affine Cipher
Use the extended Euclidean algorithm to compute a multiplicative inverse.

```c
#include "number.h"
#include <stdio.h>

void affine_encrypt(const char* plain, char* cipher, int a, int b, int m) {
    int a_inv = mod_inverse(a, m);  // only used for decryption
    for (int i = 0; plain[i]; i++) {
        if (plain[i] >= 'A' && plain[i] <= 'Z')
            cipher[i] = ((a * (plain[i] - 'A') + b) % m) + 'A';
        else
            cipher[i] = plain[i];
    }
    cipher[strlen(plain)] = '\0';
}
```

---

## Sets → Set Data Structures, Database Intersection

### Application: Deduplication via Set (List → Unique)
```c
#include "set.h"
#include <stdio.h>

void deduplicate(int* arr, int n) {
    Set seen;
    set_init(&seen);
    printf("Unique elements: ");
    for (int i = 0; i < n; i++) {
        if (!set_contains(&seen, arr[i])) {
            set_add(&seen, arr[i]);
            printf("%d ", arr[i]);
        }
    }
    printf("\n");
}
```

### Application: Database Query — Table Intersection
Simulate `SELECT * FROM tableA INTERSECT SELECT * FROM tableB`.

```c
#include "set.h"
#include <stdio.h>

void table_intersect(void) {
    Set employees_with_access, employees_in_building;
    set_init(&employees_with_access);
    set_init(&employees_in_building);

    // Populate from two database tables (simulated)
    int access_ids[] = {101, 203, 305, 407, 509};
    int building_ids[] = {203, 306, 407, 508, 509};
    for (int i = 0; i < 5; i++) {
        set_add(&employees_with_access, access_ids[i]);
        set_add(&employees_in_building, building_ids[i]);
    }

    // INTERSECT: employees who both have access AND are in the building
    Set result = set_intersection(&employees_with_access, &employees_in_building);
    printf("Employees with access AND in building: ");
    set_print(&result);
    printf("\n");
}
```

### Application: Role-Based Access with Set Membership
```c
#include "set.h"

typedef struct {
    Set roles;  // set of role IDs that a user has
} User;

typedef struct {
    Set required_roles;  // roles needed to access this resource
} Resource;

bool can_access(User* user, Resource* res) {
    Set intersection = set_intersection(&user->roles, &res->required_roles);
    return intersection.size > 0;  // user has at least one required role
}
```

---

## Relations → SQL Joins Mapped to Relation Algebra

### Application: Inner Join as Relation Composition
In SQL, a JOIN on a foreign key is the composition of two relations:
if `R(A, B)` and `S(B, C)` share attribute B, then `R ⨝ S = {(a,c) | ∃b. (a,b)∈R ∧ (b,c)∈S}`.

```c
#include "relation.h"
#include <stdio.h>

void simulate_inner_join(void) {
    // R(EmployeeID, DepartmentID)
    Relation R;
    relation_init(&R, 5);  // domain: {0..4}
    // Employee 0 in Dept 2, Employee 1 in Dept 3, etc.
    relation_add_pair(&R, 0, 2);
    relation_add_pair(&R, 1, 3);
    relation_add_pair(&R, 2, 2);

    // S(DepartmentID, ManagerID)
    Relation S;
    relation_init(&S, 5);
    relation_add_pair(&S, 2, 4);  // Dept 2 managed by 4
    relation_add_pair(&S, 3, 1);  // Dept 3 managed by 1

    // JOIN: R ∘ S (composition): Employee → Manager via Department
    printf("Employee → Manager (via Department):\n");
    for (int i = 0; i < R.size; i++)
        for (int j = 0; j < S.size; j++)
            if (R.matrix[i][j])  // (i,j) in R
                for (int k = 0; k < S.size; k++)
                    if (S.matrix[j][k])  // (j,k) in S
                        printf("  Employee %d → Manager %d (via Dept %d)\n", i, k, j);
}
```

### Application: Equivalence Partition for Data Sharding
Use equivalence classes to partition users across database shards.

```c
#include "relation.h"

void shard_users_by_id_mod(int num_shards) {
    Relation mod_relation;
    relation_init(&mod_relation, 100);  // 100 users

    // Build equivalence: user i ~ user j iff i % num_shards == j % num_shards
    for (int i = 0; i < mod_relation.size; i++)
        for (int j = 0; j < mod_relation.size; j++)
            if (i % num_shards == j % num_shards)
                relation_add_pair(&mod_relation, i, j);

    EquivalencePartition shards = equivalence_classes(&mod_relation);
    printf("Data shards (by user ID mod %d):\n", num_shards);
    for (int s = 0; s < shards.nclasses; s++) {
        printf("  Shard %d: {", s);
        for (int j = 0; j < shards.class_sizes[s]; j++)
            printf("%d%s", shards.classes[s][j],
                   j < shards.class_sizes[s] - 1 ? ", " : "");
        printf("}\n");
    }
}
```

---

## Graph Theory → BFS in Web Crawler, Dijkstra in OSPF

### Application: Web Crawler — BFS for Link Discovery
A web crawler visits pages breadth-first: root URL → all linked pages → their links, etc.

```c
#include "graph.h"
#include <stdio.h>

void simulate_web_crawler(void) {
    // Pages as vertices, hyperlinks as edges
    Graph web;
    graph_init(&web, 6, true);  // directed: link from A to B doesn't imply reverse

    // Build link graph
    graph_add_edge(&web, 0, 1, 1);  // index.html → about.html
    graph_add_edge(&web, 0, 2, 1);  // index.html → products.html
    graph_add_edge(&web, 1, 3, 1);  // about.html → team.html
    graph_add_edge(&web, 1, 4, 1);  // about.html → contact.html
    graph_add_edge(&web, 2, 5, 1);  // products.html → cart.html
    graph_add_edge(&web, 4, 5, 1);  // contact.html → cart.html

    int crawl_order[MAX_VERTICES];
    bfs(&web, 0, crawl_order);  // start from index.html

    const char* pages[] = {"index", "about", "products", "team", "contact", "cart"};
    printf("Crawl order (BFS from index): ");
    for (int i = 0; i < web.nvertices; i++)
        printf("%s ", pages[crawl_order[i]]);
    printf("\n");
}
```

### Application: OSPF Routing — Dijkstra for Shortest Path
In link-state routing protocols (OSPF), each router runs Dijkstra to compute shortest paths.

```c
#include "graph.h"
#include <stdio.h>

void ospf_route_computation(void) {
    // Network topology: routers = vertices, links = edges with cost
    Graph network;
    graph_init(&network, 6, false);  // undirected: links are bidirectional

    const char* routers[] = {"R1", "R2", "R3", "R4", "R5", "R6"};
    // Link costs (bandwidth-weighted)
    graph_add_edge(&network, 0, 1, 10);  // R1-R2: cost 10
    graph_add_edge(&network, 0, 2, 15);  // R1-R3: cost 15
    graph_add_edge(&network, 1, 3, 12);  // R2-R4: cost 12
    graph_add_edge(&network, 1, 4, 14);  // R2-R5: cost 14
    graph_add_edge(&network, 2, 4, 5);   // R3-R5: cost 5
    graph_add_edge(&network, 3, 5, 11);  // R4-R6: cost 11
    graph_add_edge(&network, 4, 5, 8);   // R5-R6: cost 8

    int dist[MAX_VERTICES], prev[MAX_VERTICES];
    shortest_path_dijkstra(&network, 0, dist, prev);

    printf("Routing table for %s:\n", routers[0]);
    for (int i = 1; i < network.nvertices; i++) {
        printf("  → %s: cost=%d, via=", routers[i], dist[i]);
        // Reconstruct path
        int path[64], plen = 0, cur = i;
        while (cur != -1) {
            path[plen++] = cur;
            cur = prev[cur];
        }
        for (int p = plen - 1; p >= 0; p--)
            printf("%s%s", routers[path[p]], p > 0 ? " → " : "");
        printf("\n");
    }
}
```

### Application: Social Network Friend Suggestion (Mutual Friends)
```c
#include "graph.h"

void suggest_friends(Graph* social_graph, int user) {
    printf("Friend suggestions for user %d (friends of friends):\n", user);
    // For each friend f of user, for each friend g of f where g != user and g not already friend:
    //   increment g's suggestion score
}
```

---

## Trees → BST in Database Index, Huffman in gzip

### Application: B-Tree Index Simulation with BST
A database uses B-Trees to index rows by primary key; our BST demonstrates the core idea.

```c
#include "tree.h"
#include <stdio.h>

void database_index_demo(void) {
    TreeNode* index = NULL;

    // Insert row IDs as keys
    int row_ids[] = {500, 230, 780, 120, 360, 640, 900};
    for (int i = 0; i < 7; i++)
        index = bst_insert(index, row_ids[i]);

    // Range query: find all rows with ID in [200, 700]
    printf("Range query [200, 700]:\n");
    int inorder_arr[256], idx = 0;
    tree_inorder(index, inorder_arr, &idx);
    for (int i = 0; i < idx; i++)
        if (inorder_arr[i] >= 200 && inorder_arr[i] <= 700)
            printf("  Row ID: %d\n", inorder_arr[i]);

    // Point query
    TreeNode* found = bst_search(index, 360);
    printf("Lookup ID 360: %s\n", found ? "FOUND" : "NOT FOUND");

    tree_free(index);
}
```

### Application: Compiler AST Representation with Trees
Compilers represent source code as abstract syntax trees.

```c
#include "tree.h"
#include <stdio.h>

void compiler_ast_demo(void) {
    // Expression: (2 + 3) * 4
    // AST:     (*)
    //         /   \
    //       (+)    4
    //      /   \
    //     2     3

    TreeNode* n2 = tree_create_node(2);
    TreeNode* n3 = tree_create_node(3);
    TreeNode* n4 = tree_create_node(4);
    TreeNode* plus = tree_create_node('+');   // using int to store operator char
    TreeNode* mult = tree_create_node('*');

    plus->left = n2;
    plus->right = n3;
    mult->left = plus;
    mult->right = n4;

    // Postorder traversal gives postfix (RPN): 2 3 + 4 *
    int postfix[256], idx = 0;
    tree_postorder(mult, postfix, &idx);
    printf("AST postorder (RPN): ");
    for (int i = 0; i < idx; i++)
        if (postfix[i] == '+' || postfix[i] == '*')
            printf("%c ", postfix[i]);
        else
            printf("%d ", postfix[i]);
    printf("(= 20)\n");

    tree_free(n2); tree_free(n3); tree_free(n4); tree_free(plus); tree_free(mult);
}
```

### Application: Huffman Compression (gzip-like)
```c
#include "tree.h"
#include <string.h>
#include <stdio.h>

void mini_gzip_demo(void) {
    char* text = "she sells sea shells by the sea shore";

    int freq[256];
    HuffmanNode* root = huffman_build_tree(text, freq);
    HuffmanCode codes[256] = {0};
    char code_buf[256];
    huffman_generate_codes(root, code_buf, 0, codes);

    char* encoded = huffman_encode(text, codes);
    char* decoded = huffman_decode(encoded, root);

    size_t original_bits = strlen(text) * 8;
    size_t compressed_bits = strlen(encoded);
    double ratio = (double)compressed_bits / original_bits * 100.0;

    printf("Original: %zu bytes (%zu bits)\n", strlen(text), original_bits);
    printf("Encoded:  %zu bits\n", compressed_bits);
    printf("Ratio:    %.1f%%\n", ratio);
    printf("Valid:    %s\n", strcmp(text, decoded) == 0 ? "YES" : "NO");

    free(encoded);
    free(decoded);
    huffman_free(root);
}
```

---

## Combinatorics → Password Entropy, Test Case Enumeration

### Application: Password Entropy Calculation
```c
#include "combo.h"
#include <math.h>
#include <stdio.h>

void password_entropy(void) {
    // 8-character password from 62 chars (a-z, A-Z, 0-9)
    int charset_size = 62;
    int length = 8;
    // Total possible passwords: 62^8
    long long total = 1;
    for (int i = 0; i < length; i++) total *= charset_size;

    double entropy = log2((double)total);
    printf("Password space: 62^8 = %lld combinations\n", total);
    printf("Entropy: %.1f bits\n", entropy);

    // Using permutation formula for distinct characters:
    printf("If all 8 chars distinct: P(62,8) = %lld\n", permutation(62, 8));
}
```

### Application: Exhaustive Test Case Generation
Generate all possible combinations of test parameters for thorough testing.

```c
#include "combo.h"
#include <stdio.h>

int test_count = 0;
void run_test_case(int* params, int n) {
    printf("  Test %3d: [", ++test_count);
    for (int i = 0; i < n; i++)
        printf("%s%d", i > 0 ? ", " : "", params[i]);
    printf("]\n");
}

void generate_test_cases(void) {
    int browsers[] = {0, 1, 2};    // Chrome, Firefox, Safari
    int os[] = {3, 4, 5};          // Windows, macOS, Linux
    int screens[] = {6, 7};        // Desktop, Mobile

    Set B, O, S;
    set_init(&B); set_add(&B, 0); set_add(&B, 1); set_add(&B, 2);
    set_init(&O); set_add(&O, 3); set_add(&O, 4); set_add(&O, 5);
    set_init(&S); set_add(&S, 6); set_add(&S, 7);

    // Cartesian product = all test configurations
    CartesianProduct cp1 = cartesian_product(&B, &O);   // browser × os
    printf("Test matrix: %d browser × %d OS × %d screen = %d total tests\n",
           B.size, O.size, S.size, B.size * O.size * S.size);

    int test_case[3];
    for (int i = 0; i < B.size; i++)
        for (int j = 0; j < O.size; j++)
            for (int k = 0; k < S.size; k++) {
                test_case[0] = i;
                test_case[1] = j;
                test_case[2] = k;
                run_test_case(test_case, 3);
            }
}
```

---

## Boolean Algebra → ALU Design, Bit Flags

### Application: Half Adder / Full Adder (ALU Building Block)
```c
#include "boolalg.h"
#include <stdio.h>

void alu_half_adder_demo(void) {
    LogicCircuit ha;
    circuit_init(&ha, 2);  // inputs: A, B
    int sum = circuit_add_gate(&ha, GATE_XOR, 0, 1);
    int carry = circuit_add_gate(&ha, GATE_AND, 0, 1);

    printf("Half Adder truth table:\n");
    printf("  A B | Sum Carry\n");
    for (int i = 0; i < 4; i++) {
        bool in[2] = { (i >> 1) & 1, i & 1 };
        bool out[256];
        circuit_evaluate(&ha, in, out, 2);
        printf("  %d %d |  %d    %d\n", in[0], in[1], out[sum], out[carry]);
    }
}
```

### Application: Bit Flag Operations with Boolean Algebra
```c
#include <stdint.h>
#include <stdio.h>

#define FLAG_READ    (1 << 0)  // 0x01
#define FLAG_WRITE   (1 << 1)  // 0x02
#define FLAG_EXECUTE (1 << 2)  // 0x04
#define FLAG_ADMIN   (1 << 3)  // 0x08

void bit_flag_demo(void) {
    uint8_t permissions = FLAG_READ | FLAG_WRITE;  // rw-

    // Check permission (Boolean AND)
    bool can_read = (permissions & FLAG_READ) != 0;
    bool can_write = (permissions & FLAG_WRITE) != 0;
    bool can_execute = (permissions & FLAG_EXECUTE) != 0;
    bool is_admin = (permissions & FLAG_ADMIN) != 0;

    printf("Permissions: r=%d w=%d x=%d admin=%d\n",
           can_read, can_write, can_execute, is_admin);

    // Grant execute (Boolean OR)
    permissions |= FLAG_EXECUTE;
    printf("After chmod +x:   r=%d w=%d x=%d admin=%d\n",
           (permissions & FLAG_READ) != 0,
           (permissions & FLAG_WRITE) != 0,
           (permissions & FLAG_EXECUTE) != 0,
           (permissions & FLAG_ADMIN) != 0);

    // Revoke write (Boolean AND with complement)
    permissions &= ~FLAG_WRITE;
    printf("After chmod -w:   r=%d w=%d x=%d admin=%d\n",
           (permissions & FLAG_READ) != 0,
           (permissions & FLAG_WRITE) != 0,
           (permissions & FLAG_EXECUTE) != 0,
           (permissions & FLAG_ADMIN) != 0);

    // Toggle admin (Boolean XOR)
    permissions ^= FLAG_ADMIN;
    printf("After toggle admin: r=%d w=%d x=%d admin=%d\n",
           (permissions & FLAG_READ) != 0,
           (permissions & FLAG_WRITE) != 0,
           (permissions & FLAG_EXECUTE) != 0,
           (permissions & FLAG_ADMIN) != 0);
}
```

### Application: SQL WHERE Clause Optimization (Boolean Minimization)
```c
#include "boolalg.h"
#include <stdio.h>

void sql_where_optimization(void) {
    // SELECT * FROM orders WHERE (status='paid' AND priority='high')
    //                            OR (status='paid' AND NOT priority='high')
    //                            OR (status='paid')
    // This simplifies to just: status='paid'

    BoolFunction bf = {
        .func = bool_eval_or,  // placeholder — real function would encode the condition
        .nvars = 2,
        .var_names = {'S', 'P'}  // S=status, P=priority
    };

    char simplified[256];
    karnaugh_map_2var(&bf, simplified);
    printf("Simplified WHERE clause: %s\n", simplified);
    // Output: S  (status='paid' is sufficient)
}
```

### Application: Control Logic Synthesis via Karnaugh Map
Design a digital controller for a traffic light that changes state based on sensor inputs, simplified using K-map.
```c
#include "boolalg.h"

void traffic_light_controller(void) {
    // Inputs: C=car_present, T=timer_expired, E=emergency
    // Output: G=green_light (for NS direction)
    BoolFunction bf = {
        .func = bool_eval_majority,  // simplified example
        .nvars = 3,
        .var_names = {'C', 'T', 'E'}
    };

    char simplified[256];
    karnaugh_map_3var(&bf, simplified);
    printf("Traffic light controller: G = %s\n", simplified);
}
```
