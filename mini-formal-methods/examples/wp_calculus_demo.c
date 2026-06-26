#include "wp.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("====== Weakest Precondition Calculus Demo ======\n\n");

    VarStore vs;
    varstore_init(&vs);
    int vx = varstore_add(&vs, "x", 0);
    int vy = varstore_add(&vs, "y", 0);
    (void)vy;
    (void)varstore_add(&vs, "z", 0);

    /* Demo 1: wp(skip, x == 5) */
    printf("--- 1. wp(skip, Q) = Q ---\n");
    Expr* Q1 = expr_binary(EXPR_EQ, expr_var(vx), expr_const(5));
    Stmt* S1 = stmt_skip();
    Expr* wp1 = wp(S1, Q1);
    printf("wp(skip, x==5) computed.\n");

    stmt_free(S1);
    expr_free(wp1);

    /* Demo 2: wp(x := x + 1, x == 3) */
    printf("--- 2. wp(x := e, Q) = Q[x/e] ---\n");
    Expr* Q2 = expr_binary(EXPR_EQ, expr_var(vx), expr_const(3));
    Stmt* S2 = stmt_assign(vx, expr_binary(EXPR_ADD, expr_var(vx), expr_const(1)));
    Expr* wp2 = wp(S2, expr_copy(Q2));
    printf("wp computed. Executing...\n");
    vs.values[vx] = 2;
    stmt_execute(S2, &vs);
    printf("x=%d (expect 3) -> %s\n", vs.values[vx], vs.values[vx] == 3 ? "PASS" : "FAIL");
    stmt_free(S2);
    expr_free(Q2);
    expr_free(wp2);

    /* Demo 3: wp(S1;S2, Q) */
    printf("--- 3. Sequence ---\n");
    vs.values[vx] = 0;
    Expr* Q3 = expr_binary(EXPR_EQ, expr_var(vx), expr_const(5));
    Stmt* s3a = stmt_assign(vx, expr_const(3));
    Stmt* s3b = stmt_assign(vx, expr_binary(EXPR_ADD, expr_var(vx), expr_const(2)));
    Stmt* S3 = stmt_seq(s3a, s3b);
    Expr* wp3 = wp(S3, expr_copy(Q3));
    printf("wp(S1;S2) computed.\n");
    stmt_execute(S3, &vs);
    printf("x=%d -> %s\n", vs.values[vx], vs.values[vx] == 5 ? "PASS" : "FAIL");
    stmt_free(S3);
    expr_free(Q3);
    expr_free(wp3);

    /* Demo 4: Hoare triple */
    printf("--- 4. Hoare Triple ---\n");
    vs.values[vx] = 10;
    Expr* pre6  = expr_binary(EXPR_GE, expr_var(vx), expr_const(0));
    Expr* post6 = expr_binary(EXPR_GE, expr_var(vx), expr_const(0));
    Stmt* S6 = stmt_assign(vx, expr_binary(EXPR_ADD, expr_var(vx), expr_const(2)));
    bool ht6 = hoare_triple_check(S6, pre6, post6, &vs);
    printf("{x>=0} x:=x+2 {x>=0}: %s\n", ht6 ? "VALID" : "INVALID");
    stmt_free(S6);

    /* Demo 5: Tautology */
    printf("--- 5. Tautology ---\n");
    Expr* t1 = expr_binary(EXPR_EQ, expr_var(vx), expr_var(vx));
    printf("x==x tautology: %s\n", expr_is_tautology(t1) ? "YES" : "NO");
    expr_free(t1);

    /* Demo 6: GCD */
    printf("--- 6. GCD Algorithm ---\n");
    int va = varstore_add(&vs, "a", 12);
    int vb = varstore_add(&vs, "b", 8);
    Expr* guard9 = expr_binary(EXPR_NE, expr_var(va), expr_var(vb));
    Expr* if_guard = expr_binary(EXPR_GT, expr_var(va), expr_var(vb));
    Stmt* then_b = stmt_assign(va, expr_binary(EXPR_SUB, expr_var(va), expr_var(vb)));
    Stmt* else_b = stmt_assign(vb, expr_binary(EXPR_SUB, expr_var(vb), expr_var(va)));
    Stmt* if_body = stmt_if(if_guard, then_b, else_b);
    Stmt* while9 = stmt_while(guard9, if_body);
    vs.values[va] = 12; vs.values[vb] = 8;
    stmt_execute(while9, &vs);
    printf("GCD(12,8): a=%d b=%d -> %s\n",
           vs.values[va], vs.values[vb],
           (vs.values[va] == 4 && vs.values[vb] == 4) ? "PASS" : "FAIL");
    stmt_free(while9);

    printf("\n====== All wp_calculus_demo tests complete ======\n");
    return 0;
}
