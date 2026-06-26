#ifndef TEMPORAL_H
#define TEMPORAL_H
#include <stdbool.h>

/* ================================================================
 * Linear Temporal Logic (LTL) for Finite Traces
 * L8: Advanced Topics -- Temporal Logic Model Checking
 *
 * Based on Pnueli (1977): "The Temporal Logic of Programs"
 *
 * LTL Syntax:
 *   φ ::= atom | !φ | φ ∧ φ | φ ∨ φ | X φ | F φ | G φ | φ U ψ | φ R ψ
 *
 * Semantics over infinite traces π = s0, s1, s2, ...:
 *   π ⊧ atom    iff atom holds in s0
 *   π ⊧ X φ     iff π[1..] ⊧ φ
 *   π ⊧ F φ     iff ∃i≥0. π[i..] ⊧ φ
 *   π ⊧ G φ     iff ∀i≥0. π[i..] ⊧ φ
 *   π ⊧ φ U ψ   iff ∃j≥0. π[j..] ⊧ ψ and ∀0≤i<j. π[i..] ⊧ φ
 *   π ⊧ φ R ψ   iff (∀j≥0. π[j..] ⊧ ψ) or (∃j≥0. π[j..] ⊧ φ and ∀0≤i≤j. π[i..] ⊧ ψ)
 *
 * For finite traces of length N, we adapt the semantics:
 *   - G φ: φ holds at ALL positions in the trace
 *   - F φ: φ holds at SOME position in the trace
 *   - X φ: φ holds at the NEXT position (undefined at last → false)
 *   - φ U ψ: ψ eventually holds, φ holds until then
 *   - φ R ψ: ψ holds until φ releases it
 *
 * This module implements:
 *   1. LTL formula construction and parsing
 *   2. LTL semantics over finite traces
 *   3. LTL formula simplification
 *   4. LTL monitoring (runtime verification of LTL properties)
 * ================================================================ */

/* --- LTL Formula AST --- */
typedef enum {
    LTL_TRUE,
    LTL_FALSE,
    LTL_ATOM,         /* atomic proposition by index */
    LTL_NOT,          /* !φ */
    LTL_AND,          /* φ ∧ ψ */
    LTL_OR,           /* φ ∨ ψ */
    LTL_IMPLIES,      /* φ → ψ */
    LTL_NEXT,         /* X φ */
    LTL_FINALLY,      /* F φ */
    LTL_GLOBALLY,     /* G φ */
    LTL_UNTIL,        /* φ U ψ */
    LTL_RELEASE       /* φ R ψ */
} LTLKind;

typedef struct LTLFormula {
    LTLKind kind;
    int atom_idx;              /* for ATOM */
    struct LTLFormula* left;   /* unary operand or left binary operand */
    struct LTLFormula* right;  /* right binary operand */
} LTLFormula;

/* --- Atomic Proposition Evaluator --- */
/* User provides an array of bools indexed by atom_idx */
typedef bool (*AtomEval)(int state_idx, int atom_idx);

/* --- Trace --- */
#define MAX_TRACE_LEN 256
typedef struct {
    bool* atoms;       /* [state_idx * natoms + atom_idx] */
    int length;        /* number of states in trace */
    int natoms;        /* number of atomic propositions */
} Trace;

/* ================================================================
 * LTL Formula Constructors
 * ================================================================ */
LTLFormula* ltl_true(void);
LTLFormula* ltl_false(void);
LTLFormula* ltl_atom(int idx);
LTLFormula* ltl_not(LTLFormula* phi);
LTLFormula* ltl_and(LTLFormula* phi, LTLFormula* psi);
LTLFormula* ltl_or(LTLFormula* phi, LTLFormula* psi);
LTLFormula* ltl_implies(LTLFormula* phi, LTLFormula* psi);
LTLFormula* ltl_next(LTLFormula* phi);
LTLFormula* ltl_finally(LTLFormula* phi);
LTLFormula* ltl_globally(LTLFormula* phi);
LTLFormula* ltl_until(LTLFormula* phi, LTLFormula* psi);
LTLFormula* ltl_release(LTLFormula* phi, LTLFormula* psi);
void         ltl_free(LTLFormula* f);
LTLFormula*  ltl_copy(LTLFormula* f);
char*        ltl_to_string(LTLFormula* f);

/* Derived operators */
LTLFormula* ltl_eventually(LTLFormula* phi);  /* same as F */
LTLFormula* ltl_always(LTLFormula* phi);      /* same as G */

/* ================================================================
 * LTL Semantics over Finite Traces
 * ================================================================ */
bool ltl_eval(LTLFormula* f, Trace* trace, int position);

/* ================================================================
 * LTL Formula Transformations
 * ================================================================ */
/* Push negations inward (NNF) */
LTLFormula* ltl_to_nnf(LTLFormula* f);

/* Simplify formula (constant folding, double negation, etc.) */
LTLFormula* ltl_simplify(LTLFormula* f);

/* Convert to negation normal form with only ¬, ∧, ∨, X, U, R */
LTLFormula* ltl_normalize(LTLFormula* f);

/* Check if formula is a safety property (of form G φ) */
bool ltl_is_safety(LTLFormula* f);

/* Check if formula is a liveness property (of form F φ or G F φ) */
bool ltl_is_liveness(LTLFormula* f);

/* ================================================================
 * LTL Monitoring (Runtime Verification)
 * ================================================================ */
/* LTL3 monitor result: ⊤(true), ⊥(false), ?(unknown) */
typedef enum {
    LTL3_TRUE,
    LTL3_FALSE,
    LTL3_UNKNOWN
} LTL3Value;

LTL3Value ltl_monitor(LTLFormula* f, Trace* trace);
void      ltl3_print(LTL3Value v);

/* ================================================================
 * Common LTL Patterns
 * ================================================================ */
/* Response: G(request → F response) */
LTLFormula* ltl_pattern_response(int req_atom, int resp_atom);

/* Invariance: G p */
LTLFormula* ltl_pattern_invariance(int atom);

/* Precedence: !q U (p ∨ G !q)  -- p precedes q */
LTLFormula* ltl_pattern_precedence(int p_atom, int q_atom);

/* Absence: G !p  -- p never occurs */
LTLFormula* ltl_pattern_absence(int atom);

/* Existence: F p  -- p eventually occurs */
LTLFormula* ltl_pattern_existence(int atom);

/* ================================================================
 * Trace Operations
 * ================================================================ */
void  trace_init(Trace* t, int length, int natoms);
void  trace_set_atom(Trace* t, int state_idx, int atom_idx, bool value);
bool  trace_get_atom(Trace* t, int state_idx, int atom_idx);
void  trace_free(Trace* t);
void  trace_print(Trace* t);

#endif
