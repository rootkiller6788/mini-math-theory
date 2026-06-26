#include "temporal.h"
#include <stdio.h>

/* ================================================================
 * ltl_model_check_demo — LTL Model Checking Demo
 * L8: Advanced Topics — Linear Temporal Logic
 *
 * Demonstrates:
 *   1. LTL formula construction (G, F, X, U, R)
 *   2. LTL semantics evaluation over finite traces
 *   3. Negation Normal Form (NNF) transformation
 *   4. Formula simplification
 *   5. LTL patterns (response, invariance, precedence, etc.)
 *   6. LTL3 runtime monitoring
 * ================================================================ */

int main(void) {
    printf("====== LTL Model Checking Demo ======\n\n");

    /* ================================================================
     * Demo 1: Basic LTL Evaluation — G p0
     * ================================================================ */
    printf("--- 1. Basic Evaluation: G p0 ---\n");
    /* Create formula: G p0 (globally, atomic proposition 0 holds) */
    LTLFormula* g_p0 = ltl_globally(ltl_atom(0));
    printf("Formula: %s\n", ltl_to_string(g_p0));

    /* Create trace: 5 states, p0 is [1, 1, 1, 1, 1] */
    Trace t1;
    trace_init(&t1, 5, 1);
    for (int i = 0; i < 5; i++)
        trace_set_atom(&t1, i, 0, true);
    printf("Trace: all p0=true -> %s\n",
           ltl_eval(g_p0, &t1, 0) ? "TRUE" : "FALSE");

    /* Violate G p0: set state 3 to false */
    trace_set_atom(&t1, 3, 0, false);
    printf("Trace: p0 false at state 3 -> %s\n\n",
           ltl_eval(g_p0, &t1, 0) ? "TRUE" : "FALSE");
    trace_free(&t1);

    /* ================================================================
     * Demo 2: Eventually — F p0
     * ================================================================ */
    printf("--- 2. Eventually: F p0 ---\n");
    LTLFormula* f_p0 = ltl_finally(ltl_atom(0));
    printf("Formula: %s\n", ltl_to_string(f_p0));

    Trace t2;
    trace_init(&t2, 4, 1);
    trace_set_atom(&t2, 0, 0, false);
    trace_set_atom(&t2, 1, 0, false);
    trace_set_atom(&t2, 2, 0, true);
    trace_set_atom(&t2, 3, 0, false);
    printf("Trace: [0,0,1,0] -> F p0: %s\n\n",
           ltl_eval(f_p0, &t2, 0) ? "TRUE" : "FALSE");
    trace_free(&t2);

    /* ================================================================
     * Demo 3: Next — X p0
     * ================================================================ */
    printf("--- 3. Next: X p0 ---\n");
    LTLFormula* x_p0 = ltl_next(ltl_atom(0));
    printf("Formula: %s\n", ltl_to_string(x_p0));

    Trace t3;
    trace_init(&t3, 3, 1);
    trace_set_atom(&t3, 0, 0, false);
    trace_set_atom(&t3, 1, 0, true);
    trace_set_atom(&t3, 2, 0, false);
    printf("Trace: [0,1,0] -> X p0 at pos 0: %s\n",
           ltl_eval(x_p0, &t3, 0) ? "TRUE" : "FALSE");
    printf("Trace: [0,1,0] -> X p0 at pos 1: %s\n",
           ltl_eval(x_p0, &t3, 1) ? "TRUE" : "FALSE");
    printf("Trace: [0,1,0] -> X p0 at pos 2 (last): %s\n\n",
           ltl_eval(x_p0, &t3, 2) ? "TRUE" : "FALSE");
    trace_free(&t3);

    /* ================================================================
     * Demo 4: Until — p0 U p1
     * ================================================================ */
    printf("--- 4. Until: p0 U p1 ---\n");
    LTLFormula* u_form = ltl_until(ltl_atom(0), ltl_atom(1));
    printf("Formula: %s\n", ltl_to_string(u_form));

    Trace t4;
    trace_init(&t4, 5, 2);
    /* p0: [1, 1, 0, 0, 0], p1: [0, 0, 1, 0, 0] */
    int p0_seq[] = {1, 1, 0, 0, 0};
    int p1_seq[] = {0, 0, 1, 0, 0};
    for (int i = 0; i < 5; i++) {
        trace_set_atom(&t4, i, 0, p0_seq[i]);
        trace_set_atom(&t4, i, 1, p1_seq[i]);
    }
    trace_print(&t4);
    printf("p0 U p1: %s\n\n",
           ltl_eval(u_form, &t4, 0) ? "TRUE" : "FALSE");
    trace_free(&t4);

    /* ================================================================
     * Demo 5: Release — p0 R p1
     * ================================================================ */
    printf("--- 5. Release: p0 R p1 ---\n");
    LTLFormula* r_form = ltl_release(ltl_atom(0), ltl_atom(1));
    printf("Formula: %s\n", ltl_to_string(r_form));

    Trace t5;
    trace_init(&t5, 4, 2);
    /* p0: [0, 1, 0, 0], p1: [1, 1, 0, 0] */
    trace_set_atom(&t5, 0, 0, false); trace_set_atom(&t5, 0, 1, true);
    trace_set_atom(&t5, 1, 0, true);  trace_set_atom(&t5, 1, 1, true);
    trace_set_atom(&t5, 2, 0, false); trace_set_atom(&t5, 2, 1, false);
    trace_set_atom(&t5, 3, 0, false); trace_set_atom(&t5, 3, 1, false);
    trace_print(&t5);
    printf("p0 R p1: %s\n\n",
           ltl_eval(r_form, &t5, 0) ? "TRUE" : "FALSE");
    trace_free(&t5);

    /* ================================================================
     * Demo 6: Negation Normal Form (NNF)
     * ================================================================ */
    printf("--- 6. Negation Normal Form ---\n");
    LTLFormula* nf1 = ltl_not(ltl_and(ltl_atom(0), ltl_atom(1)));
    printf("Original:      %s\n", ltl_to_string(nf1));
    LTLFormula* nf1_nnf = ltl_to_nnf(nf1);
    printf("NNF:           %s\n", ltl_to_string(nf1_nnf));

    LTLFormula* nf2 = ltl_not(ltl_finally(ltl_atom(0)));
    printf("Original:      %s\n", ltl_to_string(nf2));
    LTLFormula* nf2_nnf = ltl_to_nnf(nf2);
    printf("NNF:           %s\n", ltl_to_string(nf2_nnf));

    LTLFormula* nf3 = ltl_not(ltl_until(ltl_atom(0), ltl_atom(1)));
    printf("Original:      %s\n", ltl_to_string(nf3));
    LTLFormula* nf3_nnf = ltl_to_nnf(nf3);
    printf("NNF:           %s\n\n", ltl_to_string(nf3_nnf));

    /* ================================================================
     * Demo 7: Formula Simplification
     * ================================================================ */
    printf("--- 7. Formula Simplification ---\n");
    LTLFormula* sf1 = ltl_and(ltl_true(), ltl_atom(0));
    printf("true /\\ p0  =>  %s\n", ltl_to_string(ltl_simplify(sf1)));
    LTLFormula* sf2 = ltl_or(ltl_false(), ltl_atom(0));
    printf("false \\/ p0 =>  %s\n", ltl_to_string(ltl_simplify(sf2)));
    LTLFormula* sf3 = ltl_and(ltl_false(), ltl_atom(0));
    printf("false /\\ p0 =>  %s\n\n", ltl_to_string(ltl_simplify(sf3)));

    /* ================================================================
     * Demo 8: Common LTL Patterns
     * ================================================================ */
    printf("--- 8. Common LTL Specification Patterns ---\n");

    /* Response pattern: G(request → F response) */
    LTLFormula* resp = ltl_pattern_response(0, 1);
    printf("Response G(req -> F resp):  %s\n", ltl_to_string(resp));

    /* Invariance: G p */
    LTLFormula* inv = ltl_pattern_invariance(0);
    printf("Invariance G p:             %s\n", ltl_to_string(inv));

    /* Absence: G !p */
    LTLFormula* abs = ltl_pattern_absence(0);
    printf("Absence G !p:               %s\n", ltl_to_string(abs));

    /* Existence: F p */
    LTLFormula* ex = ltl_pattern_existence(0);
    printf("Existence F p:              %s\n", ltl_to_string(ex));

    /* Precedence */
    LTLFormula* prec = ltl_pattern_precedence(0, 1);
    printf("Precedence p before q:      %s\n\n", ltl_to_string(prec));

    /* ================================================================
     * Demo 9: LTL3 Runtime Monitoring
     * ================================================================ */
    printf("--- 9. LTL3 Runtime Monitoring ---\n");
    Trace t9;
    trace_init(&t9, 3, 1);
    trace_set_atom(&t9, 0, 0, true);
    trace_set_atom(&t9, 1, 0, true);
    trace_set_atom(&t9, 2, 0, true);
    printf("G p0 with prefix [1,1,1]: ");
    ltl3_print(ltl_monitor(g_p0, &t9));
    printf("\n");

    trace_set_atom(&t9, 2, 0, false);
    printf("G p0 with prefix [1,1,0]: ");
    ltl3_print(ltl_monitor(g_p0, &t9));
    printf("\n\n");
    trace_free(&t9);

    /* ================================================================
     * Demo 10: Property Classification
     * ================================================================ */
    printf("--- 10. Safety vs Liveness ---\n");
    printf("G p0 is safety:    %s\n", ltl_is_safety(g_p0) ? "YES" : "NO");
    printf("G p0 is liveness:  %s\n", ltl_is_liveness(g_p0) ? "YES" : "NO");
    printf("F p0 is safety:    %s\n", ltl_is_safety(f_p0) ? "YES" : "NO");
    printf("F p0 is liveness:  %s\n\n", ltl_is_liveness(f_p0) ? "YES" : "NO");

    /* Cleanup */
    ltl_free(g_p0); ltl_free(f_p0); ltl_free(x_p0);
    ltl_free(u_form); ltl_free(r_form);
    ltl_free(nf1); ltl_free(nf1_nnf);
    ltl_free(nf2); ltl_free(nf2_nnf);
    ltl_free(nf3); ltl_free(nf3_nnf);
    ltl_free(sf1); ltl_free(sf2); ltl_free(sf3);
    ltl_free(resp); ltl_free(inv); ltl_free(abs); ltl_free(ex); ltl_free(prec);

    printf("====== All LTL model checking tests complete ======\n");
    return 0;
}
