#include "temporal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * LTL Formula Construction
 * ================================================================ */

LTLFormula* ltl_true(void) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_TRUE;
    f->atom_idx = 0;
    f->left = f->right = NULL;
    return f;
}

LTLFormula* ltl_false(void) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_FALSE;
    f->atom_idx = 0;
    f->left = f->right = NULL;
    return f;
}

LTLFormula* ltl_atom(int idx) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_ATOM;
    f->atom_idx = idx;
    f->left = f->right = NULL;
    return f;
}

LTLFormula* ltl_not(LTLFormula* phi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_NOT;
    f->atom_idx = 0;
    f->left = phi;
    f->right = NULL;
    return f;
}

LTLFormula* ltl_and(LTLFormula* phi, LTLFormula* psi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_AND;
    f->atom_idx = 0;
    f->left = phi;
    f->right = psi;
    return f;
}

LTLFormula* ltl_or(LTLFormula* phi, LTLFormula* psi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_OR;
    f->atom_idx = 0;
    f->left = phi;
    f->right = psi;
    return f;
}

LTLFormula* ltl_implies(LTLFormula* phi, LTLFormula* psi) {
    /* φ → ψ ≡ ¬φ ∨ ψ */
    return ltl_or(ltl_not(phi), psi);
}

LTLFormula* ltl_next(LTLFormula* phi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_NEXT;
    f->atom_idx = 0;
    f->left = phi;
    f->right = NULL;
    return f;
}

LTLFormula* ltl_finally(LTLFormula* phi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_FINALLY;
    f->atom_idx = 0;
    f->left = phi;
    f->right = NULL;
    return f;
}

LTLFormula* ltl_globally(LTLFormula* phi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_GLOBALLY;
    f->atom_idx = 0;
    f->left = phi;
    f->right = NULL;
    return f;
}

LTLFormula* ltl_until(LTLFormula* phi, LTLFormula* psi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_UNTIL;
    f->atom_idx = 0;
    f->left = phi;
    f->right = psi;
    return f;
}

LTLFormula* ltl_release(LTLFormula* phi, LTLFormula* psi) {
    LTLFormula* f = malloc(sizeof(LTLFormula));
    f->kind = LTL_RELEASE;
    f->atom_idx = 0;
    f->left = phi;
    f->right = psi;
    return f;
}

void ltl_free(LTLFormula* f) {
    if (!f) return;
    ltl_free(f->left);
    ltl_free(f->right);
    free(f);
}

LTLFormula* ltl_copy(LTLFormula* f) {
    if (!f) return NULL;
    LTLFormula* c = malloc(sizeof(LTLFormula));
    c->kind = f->kind;
    c->atom_idx = f->atom_idx;
    c->left  = ltl_copy(f->left);
    c->right = ltl_copy(f->right);
    return c;
}

LTLFormula* ltl_eventually(LTLFormula* phi) { return ltl_finally(phi); }
LTLFormula* ltl_always(LTLFormula* phi)     { return ltl_globally(phi); }

char* ltl_to_string(LTLFormula* f) {
    static char bufs[4][1024];
    static int idx = 0;
    idx = (idx + 1) % 4;
    char* buf = bufs[idx];

    if (!f) { snprintf(buf, 1024, "NULL"); return buf; }
    switch (f->kind) {
    case LTL_TRUE:     snprintf(buf, 1024, "true"); break;
    case LTL_FALSE:    snprintf(buf, 1024, "false"); break;
    case LTL_ATOM:     snprintf(buf, 1024, "p%d", f->atom_idx); break;
    case LTL_NOT:      snprintf(buf, 1024, "!(%s)", ltl_to_string(f->left)); break;
    case LTL_AND:     { char* l=ltl_to_string(f->left); char* r=ltl_to_string(f->right);
                         snprintf(buf,1024,"(%s /\\ %s)",l,r); } break;
    case LTL_OR:      { char* l=ltl_to_string(f->left); char* r=ltl_to_string(f->right);
                         snprintf(buf,1024,"(%s \\/ %s)",l,r); } break;
    case LTL_IMPLIES: { char* l=ltl_to_string(f->left); char* r=ltl_to_string(f->right);
                         snprintf(buf,1024,"(%s -> %s)",l,r); } break;
    case LTL_NEXT:     snprintf(buf, 1024, "X(%s)", ltl_to_string(f->left)); break;
    case LTL_FINALLY:  snprintf(buf, 1024, "F(%s)", ltl_to_string(f->left)); break;
    case LTL_GLOBALLY: snprintf(buf, 1024, "G(%s)", ltl_to_string(f->left)); break;
    case LTL_UNTIL:   { char* l=ltl_to_string(f->left); char* r=ltl_to_string(f->right);
                         snprintf(buf,1024,"(%s U %s)",l,r); } break;
    case LTL_RELEASE: { char* l=ltl_to_string(f->left); char* r=ltl_to_string(f->right);
                         snprintf(buf,1024,"(%s R %s)",l,r); } break;
    default:           snprintf(buf, 1024, "?"); break;
    }
    return buf;
}

/* ================================================================
 * LTL Semantics over Finite Traces
 *
 * π[position..] ⊧ φ where position is the current position in trace
 * ================================================================ */

bool ltl_eval(LTLFormula* f, Trace* trace, int position) {
    if (!f) return false;
    if (position < 0 || position >= trace->length) {
        /* Beyond trace end: X and U fail, others depend on semantics */
        switch (f->kind) {
        case LTL_TRUE:  return true;
        case LTL_FALSE: return false;
        case LTL_GLOBALLY: return true;  /* vacuously true */
        case LTL_FINALLY:  return false; /* nothing more to check */
        default: return false;
        }
    }

    switch (f->kind) {
    case LTL_TRUE:  return true;
    case LTL_FALSE: return false;

    case LTL_ATOM:
        return trace_get_atom(trace, position, f->atom_idx);

    case LTL_NOT:
        return !ltl_eval(f->left, trace, position);

    case LTL_AND:
        return ltl_eval(f->left, trace, position) &&
               ltl_eval(f->right, trace, position);

    case LTL_OR:
        return ltl_eval(f->left, trace, position) ||
               ltl_eval(f->right, trace, position);

    case LTL_IMPLIES:
        return !ltl_eval(f->left, trace, position) ||
                ltl_eval(f->right, trace, position);

    case LTL_NEXT:
        /* X φ: φ must hold at next position */
        if (position + 1 >= trace->length) return false;
        return ltl_eval(f->left, trace, position + 1);

    case LTL_FINALLY:
        /* F φ: φ must hold at some position >= current */
        for (int i = position; i < trace->length; i++) {
            if (ltl_eval(f->left, trace, i))
                return true;
        }
        return false;

    case LTL_GLOBALLY:
        /* G φ: φ must hold at ALL positions >= current */
        for (int i = position; i < trace->length; i++) {
            if (!ltl_eval(f->left, trace, i))
                return false;
        }
        return true;

    case LTL_UNTIL:
        /* φ U ψ: ∃j≥position. ψ at j AND ∀position≤i<j. φ at i */
        for (int j = position; j < trace->length; j++) {
            if (ltl_eval(f->right, trace, j)) {
                /* Check φ holds for all i in [position, j-1] */
                bool phi_holds = true;
                for (int i = position; i < j; i++) {
                    if (!ltl_eval(f->left, trace, i)) {
                        phi_holds = false;
                        break;
                    }
                }
                if (phi_holds) return true;
            }
        }
        return false;

    case LTL_RELEASE:
        /* φ R ψ: ψ holds until φ releases it.
         * ∀j≥position: ψ at j, or ∃k≥position: φ at k ∧ ∀position≤i≤k: ψ at i */
        {
            for (int j = position; j < trace->length; j++) {
                if (!ltl_eval(f->right, trace, j)) {
                    /* ψ failed at j — need φ to have held somewhere ≤ j */
                    bool phi_released = false;
                    for (int k = position; k <= j; k++) {
                        if (ltl_eval(f->left, trace, k)) {
                            /* Check ψ held for all positions up to k */
                            bool psi_holds = true;
                            for (int i = position; i <= k; i++) {
                                if (!ltl_eval(f->right, trace, i)) {
                                    psi_holds = false;
                                    break;
                                }
                            }
                            if (psi_holds) { phi_released = true; break; }
                        }
                    }
                    if (!phi_released) return false;
                }
            }
            return true; /* ψ held everywhere (or φ released appropriately) */
        }

    default:
        return false;
    }
}

/* ================================================================
 * LTL Negation Normal Form (NNF)
 * Push negations to atomic level using dualities:
 *   !true = false, !false = true
 *   !!φ = φ
 *   !(φ ∧ ψ) = !φ ∨ !ψ
 *   !(φ ∨ ψ) = !φ ∧ !ψ
 *   !X φ = X !φ
 *   !F φ = G !φ
 *   !G φ = F !φ
 *   !(φ U ψ) = (!φ) R (!ψ)
 *   !(φ R ψ) = (!φ) U (!ψ)
 * ================================================================ */

static LTLFormula* ltl_nnf_internal(LTLFormula* f, bool negated);

LTLFormula* ltl_to_nnf(LTLFormula* f) {
    return ltl_nnf_internal(f, false);
}

static LTLFormula* ltl_nnf_internal(LTLFormula* f, bool negated) {
    if (!f) return NULL;

    if (!negated) {
        switch (f->kind) {
        case LTL_TRUE:     return ltl_true();
        case LTL_FALSE:    return ltl_false();
        case LTL_ATOM:     return ltl_atom(f->atom_idx);
        case LTL_NOT:      return ltl_nnf_internal(f->left, true);
        case LTL_AND:
            return ltl_and(ltl_nnf_internal(f->left, false),
                          ltl_nnf_internal(f->right, false));
        case LTL_OR:
            return ltl_or(ltl_nnf_internal(f->left, false),
                         ltl_nnf_internal(f->right, false));
        case LTL_IMPLIES:
            /* φ → ψ ≡ ¬φ ∨ ψ */
            return ltl_or(ltl_nnf_internal(f->left, true),
                         ltl_nnf_internal(f->right, false));
        case LTL_NEXT:
            return ltl_next(ltl_nnf_internal(f->left, false));
        case LTL_FINALLY:
            return ltl_finally(ltl_nnf_internal(f->left, false));
        case LTL_GLOBALLY:
            return ltl_globally(ltl_nnf_internal(f->left, false));
        case LTL_UNTIL:
            return ltl_until(ltl_nnf_internal(f->left, false),
                            ltl_nnf_internal(f->right, false));
        case LTL_RELEASE:
            return ltl_release(ltl_nnf_internal(f->left, false),
                              ltl_nnf_internal(f->right, false));
        default: return ltl_true();
        }
    } else {
        /* negated = true: apply De Morgan dualities */
        switch (f->kind) {
        case LTL_TRUE:     return ltl_false();
        case LTL_FALSE:    return ltl_true();
        case LTL_ATOM:     return ltl_not(ltl_atom(f->atom_idx));
        case LTL_NOT:      return ltl_nnf_internal(f->left, false);  /* !!φ = φ */
        case LTL_AND:
            return ltl_or(ltl_nnf_internal(f->left, true),
                         ltl_nnf_internal(f->right, true));
        case LTL_OR:
            return ltl_and(ltl_nnf_internal(f->left, true),
                          ltl_nnf_internal(f->right, true));
        case LTL_IMPLIES:
            /* !(φ → ψ) = φ ∧ !ψ */
            return ltl_and(ltl_nnf_internal(f->left, false),
                          ltl_nnf_internal(f->right, true));
        case LTL_NEXT:
            return ltl_next(ltl_nnf_internal(f->left, true));
        case LTL_FINALLY:
            return ltl_globally(ltl_nnf_internal(f->left, true));
        case LTL_GLOBALLY:
            return ltl_finally(ltl_nnf_internal(f->left, true));
        case LTL_UNTIL:
            /* !(φ U ψ) = (!φ) R (!ψ) */
            return ltl_release(ltl_nnf_internal(f->left, true),
                              ltl_nnf_internal(f->right, true));
        case LTL_RELEASE:
            /* !(φ R ψ) = (!φ) U (!ψ) */
            return ltl_until(ltl_nnf_internal(f->left, true),
                            ltl_nnf_internal(f->right, true));
        default: return ltl_false();
        }
    }
}

/* ================================================================
 * LTL Simplification
 * ================================================================ */

LTLFormula* ltl_simplify(LTLFormula* f) {
    if (!f) return NULL;

    /* Simplify children first */
    LTLFormula* left  = f->left  ? ltl_simplify(f->left)  : NULL;
    LTLFormula* right = f->right ? ltl_simplify(f->right) : NULL;

    switch (f->kind) {
    case LTL_TRUE:
    case LTL_FALSE:
    case LTL_ATOM:
        ltl_free(left); ltl_free(right);
        return ltl_copy(f);

    case LTL_NOT:
        if (left && left->kind == LTL_TRUE) {
            ltl_free(left);
            return ltl_false();
        }
        if (left && left->kind == LTL_FALSE) {
            ltl_free(left);
            return ltl_true();
        }
        /* !!φ = φ */
        if (left && left->kind == LTL_NOT) {
            LTLFormula* inner = ltl_copy(left->left);
            ltl_free(left);
            return inner;
        }
        return ltl_not(left);

    case LTL_AND:
        if ((left && left->kind == LTL_FALSE) ||
            (right && right->kind == LTL_FALSE)) {
            ltl_free(left); ltl_free(right);
            return ltl_false();
        }
        if (left && left->kind == LTL_TRUE) { ltl_free(left); return right; }
        if (right && right->kind == LTL_TRUE) { ltl_free(right); return left; }
        return ltl_and(left, right);

    case LTL_OR:
        if ((left && left->kind == LTL_TRUE) ||
            (right && right->kind == LTL_TRUE)) {
            ltl_free(left); ltl_free(right);
            return ltl_true();
        }
        if (left && left->kind == LTL_FALSE) { ltl_free(left); return right; }
        if (right && right->kind == LTL_FALSE) { ltl_free(right); return left; }
        return ltl_or(left, right);

    case LTL_IMPLIES:
        return ltl_or(ltl_not(left), right);  /* reduce to ∨ */

    case LTL_FINALLY:
        if (left && left->kind == LTL_TRUE) {
            ltl_free(left); return ltl_true();
        }
        if (left && left->kind == LTL_FALSE) {
            ltl_free(left); return ltl_false();
        }
        return ltl_finally(left);

    case LTL_GLOBALLY:
        if (left && left->kind == LTL_TRUE) {
            ltl_free(left); return ltl_true();
        }
        if (left && left->kind == LTL_FALSE) {
            ltl_free(left); return ltl_false();
        }
        return ltl_globally(left);

    default:
        /* Preserve structure for NEXT, UNTIL, RELEASE */
        if (f->kind == LTL_NEXT)
            return ltl_next(left);
        if (f->kind == LTL_UNTIL)
            return ltl_until(left, right);
        if (f->kind == LTL_RELEASE)
            return ltl_release(left, right);
        ltl_free(left); ltl_free(right);
        return ltl_true();
    }
}

LTLFormula* ltl_normalize(LTLFormula* f) {
    LTLFormula* nnf = ltl_to_nnf(f);
    LTLFormula* simplified = ltl_simplify(nnf);
    ltl_free(nnf);
    return simplified;
}

/* ================================================================
 * Property Classification
 * ================================================================ */

bool ltl_is_safety(LTLFormula* f) {
    /* Safety: "something bad never happens"
     * Typically G φ or G(!φ) etc. */
    if (!f) return false;
    if (f->kind == LTL_GLOBALLY) return true;
    if (f->kind == LTL_NOT && f->left && f->left->kind == LTL_FINALLY)
        return true;  /* !F φ = G !φ */
    if (f->kind == LTL_AND)
        return ltl_is_safety(f->left) && ltl_is_safety(f->right);
    return false;
}

bool ltl_is_liveness(LTLFormula* f) {
    /* Liveness: "something good eventually happens"
     * Typically F φ or G F φ */
    if (!f) return false;
    if (f->kind == LTL_FINALLY) return true;
    if (f->kind == LTL_GLOBALLY && f->left && f->left->kind == LTL_FINALLY)
        return true;  /* G F φ */
    if (f->kind == LTL_OR)
        return ltl_is_liveness(f->left) || ltl_is_liveness(f->right);
    return false;
}

/* ================================================================
 * LTL3 Runtime Monitoring
 *
 * LTL3 semantics for finite traces:
 *   - ⊤ (true) if all possible completions satisfy φ
 *   - ⊥ (false) if no possible completion satisfies φ
 *   - ? (unknown) if some completions satisfy and some don't
 *
 * Simplified: we check if the formula has been deterministically
 * satisfied/violated given the prefix seen so far.
 * ================================================================ */

LTL3Value ltl_monitor(LTLFormula* f, Trace* trace) {
    if (!f) return LTL3_UNKNOWN;

    /* Evaluate on the observed prefix */
    bool prefix_result = ltl_eval(f, trace, 0);

    /* Also check: could future extensions change the result?
     * Simplified heuristic:
     *   - G φ: if any violation seen so far → false (permanent)
     *   - F φ: if satisfied so far → true (permanent)
     *   - Otherwise → unknown
     */
    switch (f->kind) {
    case LTL_GLOBALLY: {
        /* Check if all states so far satisfy φ */
        bool all_satisfy = true;
        for (int i = 0; i < trace->length; i++) {
            if (!ltl_eval(f->left, trace, i)) {
                all_satisfy = false;
                break;
            }
        }
        if (!all_satisfy) return LTL3_FALSE;
        return LTL3_UNKNOWN;
    }
    case LTL_FINALLY: {
        /* Check if φ was already satisfied */
        bool satisfied = false;
        for (int i = 0; i < trace->length; i++) {
            if (ltl_eval(f->left, trace, i)) {
                satisfied = true;
                break;
            }
        }
        if (satisfied) return LTL3_TRUE;
        return LTL3_UNKNOWN;
    }
    default:
        return prefix_result ? LTL3_TRUE : LTL3_UNKNOWN;
    }
}

void ltl3_print(LTL3Value v) {
    switch (v) {
    case LTL3_TRUE:    printf("TRUE (permanently satisfied)"); break;
    case LTL3_FALSE:   printf("FALSE (permanently violated)"); break;
    case LTL3_UNKNOWN: printf("UNKNOWN (not yet determined)"); break;
    }
}

/* ================================================================
 * Common LTL Patterns (Dwyer, Avrunin, Corbett 1999)
 * ================================================================ */

/* Response: G(request → F response) */
LTLFormula* ltl_pattern_response(int req_atom, int resp_atom) {
    return ltl_globally(
        ltl_implies(
            ltl_atom(req_atom),
            ltl_finally(ltl_atom(resp_atom))
        )
    );
}

/* Invariance: G p */
LTLFormula* ltl_pattern_invariance(int atom) {
    return ltl_globally(ltl_atom(atom));
}

/* Precedence: p precedes q → !q U (p ∨ G !q) */
LTLFormula* ltl_pattern_precedence(int p_atom, int q_atom) {
    /* "q is not true until p has been true" */
    return ltl_until(
        ltl_not(ltl_atom(q_atom)),
        ltl_or(ltl_atom(p_atom), ltl_globally(ltl_not(ltl_atom(q_atom))))
    );
}

/* Absence: G !p */
LTLFormula* ltl_pattern_absence(int atom) {
    return ltl_globally(ltl_not(ltl_atom(atom)));
}

/* Existence: F p */
LTLFormula* ltl_pattern_existence(int atom) {
    return ltl_finally(ltl_atom(atom));
}

/* ================================================================
 * Trace Operations
 * ================================================================ */

void trace_init(Trace* t, int length, int natoms) {
    if (length > MAX_TRACE_LEN) length = MAX_TRACE_LEN;
    t->length = length;
    t->natoms = natoms;
    t->atoms = calloc(length * natoms, sizeof(bool));
}

void trace_set_atom(Trace* t, int state_idx, int atom_idx, bool value) {
    if (state_idx >= 0 && state_idx < t->length &&
        atom_idx >= 0 && atom_idx < t->natoms)
        t->atoms[state_idx * t->natoms + atom_idx] = value;
}

bool trace_get_atom(Trace* t, int state_idx, int atom_idx) {
    if (state_idx >= 0 && state_idx < t->length &&
        atom_idx >= 0 && atom_idx < t->natoms)
        return t->atoms[state_idx * t->natoms + atom_idx];
    return false;
}

void trace_free(Trace* t) {
    free(t->atoms);
    t->atoms = NULL;
    t->length = 0;
}

void trace_print(Trace* t) {
    printf("Trace: %d states, %d atoms\n", t->length, t->natoms);
    for (int i = 0; i < t->length; i++) {
        printf("  s%d: [", i);
        for (int j = 0; j < t->natoms; j++) {
            printf("%c", trace_get_atom(t, i, j) ? '1' : '0');
            if (j < t->natoms - 1) printf(" ");
        }
        printf("]\n");
    }
}
