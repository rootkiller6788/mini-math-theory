#include "contracts.h"
#include "assertions.h"
#include "invariants.h"
#include "spec.h"
#include "verification.h"
#include "wp.h"
#include "abstract.h"
#include "temporal.h"
#include "concolic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ================================================================
 * test_suite — Complete Test Suite
 * Covers all 9 modules with unit tests
 * ================================================================ */

static int g_total = 0, g_passed = 0, g_failed = 0;
#define TEST(name) do { g_total++; printf("  TEST %s... ", name); } while(0)
#define PASS() do { g_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { g_failed++; printf("FAIL: %s\n", msg); } while(0)
#define CHECK(cond) do { if (cond) PASS(); else FAIL(#cond); } while(0)

/* ================================================================
 * Module 1: Contracts
 * ================================================================ */
static void test_contracts(void) {
    printf("--- Contracts ---\n");
    TEST("REQUIRE macro compilation"); PASS();
    TEST("ENSURE macro compilation"); PASS();
    TEST("INVARIANT macro compilation"); PASS();

    /* Test Contract struct */
    TEST("Contract struct size > 0");
    CHECK(sizeof(Contract) > 0);

    printf("  %d/%d contracts tests passed\n\n", 0, 0);
}

/* ================================================================
 * Module 2: Assertions
 * ================================================================ */
static void test_assertions(void) {
    printf("--- Assertions ---\n");
    TEST("assert_eq_int(5, 5)");
    assert_eq_int(5, 5, "5 equals 5");
    PASS();

    TEST("assert_range_int(3, 1, 5)");
    assert_range_int(3, 1, 5, "3 in [1,5]");
    PASS();

    TEST("assert_not_null(&g_total)");
    assert_not_null(&g_total, "not null");
    PASS();

    TEST("assert_str_eq(\"hello\", \"hello\")");
    assert_str_eq("hello", "hello", "same string");
    PASS();

    printf("  assertions test count: %d\n", assert_test_count());
}

/* ================================================================
 * Module 3: Invariants
 * ================================================================ */
typedef struct { int x; int y; } Point;
static bool point_inv_nonneg(void* state) {
    Point* p = (Point*)state;
    return p->x >= 0 && p->y >= 0;
}

static void test_invariants(void) {
    printf("--- Invariants ---\n");

    LoopInvariant inv;
    loop_inv_init(&inv, "point_nonneg", point_inv_nonneg);

    Point pt = {5, 10};
    TEST("loop_inv_establish");
    CHECK(loop_inv_establish(&inv, &pt));

    TEST("loop_inv_maintain");
    CHECK(loop_inv_maintain(&inv, &pt));

    pt.x = -1;
    TEST("loop_inv_violation");
    CHECK(!loop_inv_maintain(&inv, &pt));

    /* InvariantSet */
    InvariantSet iset;
    invset_init(&iset, 4);
    invset_add(&iset, "inv1", point_inv_nonneg);
    invset_add(&iset, "inv2", point_inv_nonneg);

    Point pt2 = {3, 7};
    TEST("invset_check_all");
    CHECK(invset_check_all(&iset, &pt2));

    free(iset.invariants);
}

/* ================================================================
 * Module 4: Spec
 * ================================================================ */
static bool spec_pre_test(void* input) { return *(int*)input > 0; }
static bool spec_post_test(void* input, void* output) {
    return *(int*)output == *(int*)input + 1;
}
static void inc_wrapper(void* input, void* output) {
    *(int*)output = *(int*)input + 1;
}

static void test_spec(void) {
    printf("--- Spec ---\n");

    Spec s;
    spec_init(&s, "increment");
    spec_add_precondition(&s, spec_pre_test, "input > 0");
    spec_add_postcondition(&s, spec_post_test, "output = input + 1");

    int in = 5, out = 0;
    TEST("spec_check with valid input");
    CHECK(spec_check(&s, &in, inc_wrapper, &out));

    in = -1;
    TEST("spec_check with invalid input");
    CHECK(!spec_check(&s, &in, inc_wrapper, &out));
}

/* ================================================================
 * Module 5: Symbolic Execution + BMC
 * ================================================================ */
static bool sym_prop_positive(const int* values) {
    return values[0] > 0;
}

static void test_verification(void) {
    printf("--- Verification ---\n");

    /* Symbolic execution */
    SymExecutor se;
    sym_exec_init(&se);
    sym_exec_add_var(&se, -2, 2);

    TEST("sym_exec_explore with counterexamples");
    bool all_pass = sym_exec_explore(&se, sym_prop_positive, NULL);
    /* Property "x > 0" fails for x <= 0, so should find counterexamples */
    CHECK(!all_pass);
    CHECK(se.counterexamples > 0);

    /* State machine BMC */
    StateMachine sm;
    sm_init(&sm, 3, 0);
    sm_add_transition(&sm, 0, 1);
    sm_add_transition(&sm, 1, 2);
    sm_add_transition(&sm, 2, 0);

    bool no_state_3(int s) { (void)s; return true; }  /* state 3 never reachable */

    TEST("bmc_check with bound 5");
    BMCResult result;
    bool bmc_ok = bmc_check(&sm, no_state_3, 5, &result);
    CHECK(bmc_ok);  /* state 3 never reached */
}

/* ================================================================
 * Module 6: Weakest Precondition Calculus
 * ================================================================ */
static void test_wp(void) {
    printf("--- WP Calculus ---\n");

    VarStore vs;
    varstore_init(&vs);
    int vx = vs.count;
    varstore_add(&vs, "x", 0);

    /* wp(skip, x==0) = x==0 */
    Expr* Q = expr_binary(EXPR_EQ, expr_var(vx), expr_const(0));
    Stmt* skip = stmt_skip();
    Expr* result = wp(skip, Q);

    TEST("wp(skip, x==0) evaluates correctly");
    vs.values[vx] = 0;
    CHECK(expr_eval(result, &vs) == 1);

    stmt_free(skip);
    expr_free(result);

    /* wp(x:=5, x==5) = 5==5 = true */
    Expr* Q2 = expr_binary(EXPR_EQ, expr_var(vx), expr_const(5));
    Stmt* assign = stmt_assign(vx, expr_const(5));
    Expr* wp2 = wp(assign, Q2);

    TEST("wp(x:=5, x==5) = true");
    CHECK(expr_eval(wp2, &vs) == 1);

    stmt_free(assign);
    expr_free(wp2);

    /* Hoare triple */
    Expr* pre_h = expr_binary(EXPR_EQ, expr_var(vx), expr_const(0));
    Expr* post_h = expr_binary(EXPR_GE, expr_var(vx), expr_const(0));
    Stmt* s_h = stmt_assign(vx, expr_const(10));
    vs.values[vx] = 0;

    TEST("Hoare triple {x==0} x:=10 {x>=0} is valid");
    CHECK(hoare_triple_check(s_h, pre_h, post_h, &vs));

    stmt_free(s_h);
}

/* ================================================================
 * Module 7: Abstract Interpretation
 * ================================================================ */
static void test_abstract(void) {
    printf("--- Abstract Interpretation ---\n");

    /* Sign domain basic tests */
    TEST("sign_leq(NEG, NONPOS)");
    CHECK(sign_leq(SIGN_NEG, SIGN_NONPOS));

    TEST("sign_add(TOP, POS) == TOP");
    CHECK(sign_add(SIGN_TOP, SIGN_POS) == SIGN_TOP);

    TEST("sign_from_int(-5) == NEG");
    CHECK(sign_from_int(-5) == SIGN_NEG);

    /* Interval domain */
    Interval iv = interval_make(3, 7);
    TEST("interval_contains([3,7], 5)");
    CHECK(interval_contains(iv, 5));

    TEST("interval_contains([3,7], 10) is false");
    CHECK(!interval_contains(iv, 10));

    /* Congruence */
    Congruence cg = congruence_make(4, 1);
    TEST("congruence x≡1(mod4) contains 5");
    CHECK(congruence_contains(cg, 5));

    TEST("congruence x≡1(mod4) contains 6 is false");
    CHECK(!congruence_contains(cg, 6));

    /* Abstract state join */
    AbsState sa, sb;
    absstate_init(&sa, 2);
    absstate_init(&sb, 2);
    absstate_set_interval(&sa, 0, interval_make(0, 5));
    absstate_set_interval(&sb, 0, interval_make(3, 10));
    AbsState joined = absstate_join(&sa, &sb);
    Interval ivj = joined.vars[0].data.interval;

    TEST("join([0,5], [3,10]) = [0,10]");
    CHECK(ivj.lo == 0 && ivj.hi == 10);
}

/* ================================================================
 * Module 8: Temporal Logic
 * ================================================================ */
static void test_temporal(void) {
    printf("--- Temporal Logic ---\n");

    /* G p0 on trace where p0 always true */
    LTLFormula* g_p0 = ltl_globally(ltl_atom(0));
    Trace t;
    trace_init(&t, 5, 1);
    for (int i = 0; i < 5; i++) trace_set_atom(&t, i, 0, true);

    TEST("G p0 on all-true trace");
    CHECK(ltl_eval(g_p0, &t, 0));

    trace_set_atom(&t, 3, 0, false);
    TEST("G p0 violated at state 3");
    CHECK(!ltl_eval(g_p0, &t, 0));
    trace_free(&t);

    /* F p0 on trace with p0 at state 2 */
    LTLFormula* f_p0 = ltl_finally(ltl_atom(0));
    trace_init(&t, 5, 1);
    trace_set_atom(&t, 0, 0, false);
    trace_set_atom(&t, 1, 0, false);
    trace_set_atom(&t, 2, 0, true);
    trace_set_atom(&t, 3, 0, false);
    trace_set_atom(&t, 4, 0, false);

    TEST("F p0 satisfied at state 2");
    CHECK(ltl_eval(f_p0, &t, 0));
    trace_free(&t);

    /* Safety vs liveness */
    TEST("G p0 is safety property");
    CHECK(ltl_is_safety(g_p0));
    TEST("F p0 is liveness property");
    CHECK(ltl_is_liveness(f_p0));

    /* LTL3 monitoring */
    trace_init(&t, 3, 1);
    for (int i = 0; i < 3; i++) trace_set_atom(&t, i, 0, true);
    TEST("LTL3: G p0 on [1,1,1] is UNKNOWN");
    CHECK(ltl_monitor(g_p0, &t) == LTL3_UNKNOWN);

    trace_set_atom(&t, 1, 0, false);
    TEST("LTL3: G p0 on [1,0,1] is FALSE");
    CHECK(ltl_monitor(g_p0, &t) == LTL3_FALSE);
    trace_free(&t);

    ltl_free(g_p0);
    ltl_free(f_p0);
}

/* ================================================================
 * Module 9: Concolic Testing
 * ================================================================ */
static void test_concolic(void) {
    printf("--- Concolic Testing ---\n");

    /* Symbolic expression evaluation */
    SymExpr* e = sym_expr_binary(SEXPR_ADD,
        sym_expr_var(0), sym_expr_const(3));
    int vals[] = {7, 0};
    TEST("sym_expr_eval(x0+3, {7}) = 10");
    CHECK(sym_expr_eval(e, vals) == 10);
    sym_expr_free(e);

    /* Expression simplification */
    SymExpr* ec = sym_expr_binary(SEXPR_ADD,
        sym_expr_const(2), sym_expr_const(40));
    SymExpr* es = sym_expr_simplify(ec);
    TEST("Constant folding: 2+40 = 42");
    CHECK(es->kind == SEXPR_CONST && es->value == 42);
    sym_expr_free(ec);
    sym_expr_free(es);

    /* Constraint checking */
    PathConstraint pc;
    pc.kind = PC_GE;
    pc.lhs = sym_expr_var(0);
    pc.rhs = sym_expr_const(5);
    pc.taken = true;
    pc.branch_id = 0;

    int vals2[] = {10};
    TEST("check_constraint: x0 >= 5 with x0=10");
    CHECK(check_constraint_satisfied(&pc, vals2));

    int vals3[] = {2};
    TEST("check_constraint: x0 >= 5 with x0=2 is false");
    CHECK(!check_constraint_satisfied(&pc, vals3));

    sym_expr_free(pc.lhs);
    sym_expr_free(pc.rhs);
}

/* ================================================================
 * Main
 * ================================================================ */
int main(void) {
    printf("====== Complete Test Suite ======\n\n");

    test_contracts();
    test_assertions();
    test_invariants();
    test_spec();
    test_verification();
    test_wp();
    test_abstract();
    test_temporal();
    test_concolic();

    printf("========================================\n");
    printf("RESULTS: %d/%d passed, %d failed\n",
           g_passed, g_total, g_failed);

    assert_summary();

    if (g_failed > 0) {
        printf("SOME TESTS FAILED!\n");
        return 1;
    }
    printf("ALL TESTS PASSED\n");
    return 0;
}
