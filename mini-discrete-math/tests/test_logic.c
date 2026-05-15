#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define CHECK(cond, msg) do { if (cond) PASS(); else FAIL(msg); } while(0)

static Formula* build_formula_2var(char v0, char v1) {
    Formula* f = formula_create();
    f->nvars = 2;
    f->var_names[0] = v0;
    f->var_names[1] = v1;
    return f;
}

static Formula* build_formula_3var(char v0, char v1, char v2) {
    Formula* f = formula_create();
    f->nvars = 3;
    f->var_names[0] = v0;
    f->var_names[1] = v1;
    f->var_names[2] = v2;
    return f;
}

static bool eval_all_assignments(Formula* f, int root, bool* expected) {
    int rows = 1 << f->nvars;
    for (int r = 0; r < rows; r++) {
        bool assign[MAX_VARS] = {0};
        for (int b = 0; b < f->nvars; b++)
            assign[b] = (r >> (f->nvars - 1 - b)) & 1;
        if (formula_eval(f, root, assign) != expected[r])
            return false;
    }
    return true;
}

static void test_p_and_q(void) {
    TEST("p AND q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_and(f, p, q);
    // p=F,q=F→F; p=F,q=T→F; p=T,q=F→F; p=T,q=T→T
    bool expected[] = {false, false, false, true};
    CHECK(eval_all_assignments(f, root, expected),
          "p ∧ q mismatch");
    free(f);
}

static void test_p_or_q(void) {
    TEST("p OR q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_or(f, p, q);
    bool expected[] = {false, true, true, true};
    CHECK(eval_all_assignments(f, root, expected),
          "p ∨ q mismatch");
    free(f);
}

static void test_not_p_or_q(void) {
    TEST("¬p ∨ q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int not_p = formula_add_not(f, p);
    int root = formula_add_or(f, not_p, q);
    // ¬p ∨ q ≡ p → q
    // Expected: T, T, F, T  (for p=F,q=F; p=F,q=T; p=T,q=F; p=T,q=T)
    bool expected[] = {true, true, false, true};
    CHECK(eval_all_assignments(f, root, expected),
          "¬p ∨ q mismatch");
    free(f);
}

static void test_p_implies_q(void) {
    TEST("p → q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_implies(f, p, q);
    bool expected[] = {true, true, false, true};
    CHECK(eval_all_assignments(f, root, expected),
          "p → q mismatch");
    free(f);
}

static void test_p_iff_q(void) {
    TEST("p ↔ q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_iff(f, p, q);
    bool expected[] = {true, false, false, true};
    CHECK(eval_all_assignments(f, root, expected),
          "p ↔ q mismatch");
    free(f);
}

static void test_p_xor_q(void) {
    TEST("p ⊕ q truth table");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_xor(f, p, q);
    bool expected[] = {false, true, true, false};
    CHECK(eval_all_assignments(f, root, expected),
          "p ⊕ q mismatch");
    free(f);
}

static void test_not_p_and_q(void) {
    TEST("¬(p ∧ q) truth table (De Morgan: ¬p ∨ ¬q)");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int pandq = formula_add_and(f, p, q);
    int root = formula_add_not(f, pandq);
    bool expected[] = {true, true, true, false};
    CHECK(eval_all_assignments(f, root, expected),
          "¬(p ∧ q) mismatch");
    free(f);
}

static void test_not_p_or_not_q(void) {
    TEST("¬p ∨ ¬q truth table (De Morgan equiv of ¬(p∧q))");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int not_p = formula_add_not(f, p);
    int not_q = formula_add_not(f, q);
    int root = formula_add_or(f, not_p, not_q);
    bool expected[] = {true, true, true, false};
    CHECK(eval_all_assignments(f, root, expected),
          "¬p ∨ ¬q mismatch");
    free(f);
}

static void test_tautology_excluded_middle(void) {
    TEST("p ∨ ¬p is a tautology");
    Formula* f = formula_create();
    f->nvars = 1;
    f->var_names[0] = 'p';
    int p = formula_add_var(f, 0);
    int not_p = formula_add_not(f, p);
    int root = formula_add_or(f, p, not_p);
    CHECK(is_tautology(f, root),
          "p ∨ ¬p should be tautology");
    free(f);
}

static void test_contradiction(void) {
    TEST("p ∧ ¬p is a contradiction");
    Formula* f = formula_create();
    f->nvars = 1;
    f->var_names[0] = 'p';
    int p = formula_add_var(f, 0);
    int not_p = formula_add_not(f, p);
    int root = formula_add_and(f, p, not_p);
    CHECK(is_contradiction(f, root),
          "p ∧ ¬p should be contradiction");
    free(f);
}

static void test_not_tautology(void) {
    TEST("p ∧ q is NOT a tautology");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_and(f, p, q);
    CHECK(!is_tautology(f, root),
          "p ∧ q should NOT be tautology");
    free(f);
}

static void test_not_contradiction(void) {
    TEST("p ∨ q is NOT a contradiction");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int root = formula_add_or(f, p, q);
    CHECK(!is_contradiction(f, root),
          "p ∨ q should NOT be contradiction");
    free(f);
}

static void test_implication_tautology(void) {
    TEST("(p ∧ (p → q)) → q is a tautology (modus ponens)");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int pq = formula_add_implies(f, p, q);
    int pand = formula_add_and(f, p, pq);
    int root = formula_add_implies(f, pand, q);
    CHECK(is_tautology(f, root),
          "modus ponens should be tautology");
    free(f);
}

static void test_de_morgan_equivalence(void) {
    TEST("¬(p ∧ q) ↔ (¬p ∨ ¬q) is a tautology (De Morgan)");
    Formula* f = build_formula_2var('p', 'q');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);

    // ¬(p ∧ q)
    int pandq = formula_add_and(f, p, q);
    int lhs = formula_add_not(f, pandq);

    // ¬p ∨ ¬q
    int not_p = formula_add_not(f, p);
    int not_q = formula_add_not(f, q);
    int rhs = formula_add_or(f, not_p, not_q);

    int root = formula_add_iff(f, lhs, rhs);
    CHECK(is_tautology(f, root),
          "De Morgan equivalence should be tautology");
    free(f);
}

static void test_3var_majority(void) {
    TEST("3-variable majority (p∧q)∨(q∧r)∨(r∧p) truth table");
    Formula* f = build_formula_3var('p', 'q', 'r');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int r = formula_add_var(f, 2);

    int pq = formula_add_and(f, p, q);
    int qr = formula_add_and(f, q, r);
    int rp = formula_add_and(f, r, p);
    int pq_qr = formula_add_or(f, pq, qr);
    int root = formula_add_or(f, pq_qr, rp);

    // Majority true when ≥2 of p,q,r are true
    bool expected[] = {
        false,  // 000
        false,  // 001
        false,  // 010
        true,   // 011 - q,r
        false,  // 100
        true,   // 101 - p,r
        true,   // 110 - p,q
        true    // 111
    };
    CHECK(eval_all_assignments(f, root, expected),
          "3-variable majority mismatch");
    free(f);
}

static void test_3var_complex(void) {
    TEST("(p → q) ∧ (q → r) truth table (hypothetical syllogism premises)");
    Formula* f = build_formula_3var('p', 'q', 'r');
    int p = formula_add_var(f, 0);
    int q = formula_add_var(f, 1);
    int r = formula_add_var(f, 2);

    int p_q = formula_add_implies(f, p, q);
    int q_r = formula_add_implies(f, q, r);
    int root = formula_add_and(f, p_q, q_r);

    // (p→q)∧(q→r) is true except when (p=T,q=T,r=F) or (p=T,q=F,*)
    // row: pqr=000→T, 001→T, 010→F, 011→F, 100→T, 101→T, 110→T, 111→T
    bool expected[] = {true, true, false, false, true, true, true, true};
    CHECK(eval_all_assignments(f, root, expected),
          "hypothetical syllogism premise mismatch");
    free(f);
}

static void test_induction_demo_runs(void) {
    TEST("proof_by_induction_demo runs without crash");
    proof_by_induction_demo(5);
    PASS();
}

int main(void) {
    printf("====== Logic Module Tests ======\n\n");

    printf("--- Truth Table Verification ---\n");
    test_p_and_q();
    test_p_or_q();
    test_not_p_or_q();
    test_p_implies_q();
    test_p_iff_q();
    test_p_xor_q();
    test_not_p_and_q();
    test_not_p_or_not_q();
    test_3var_majority();
    test_3var_complex();

    printf("\n--- Tautology / Contradiction ---\n");
    test_tautology_excluded_middle();
    test_contradiction();
    test_not_tautology();
    test_not_contradiction();
    test_implication_tautology();
    test_de_morgan_equivalence();

    printf("\n--- Induction Demo ---\n");
    test_induction_demo_runs();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
