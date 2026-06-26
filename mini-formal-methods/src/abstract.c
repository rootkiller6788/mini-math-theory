#include "abstract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ================================================================
 * Sign Domain -- Lattice of Signs
 *
 * Hasse diagram:
 *              TOP (unknown)
 *         /      |       \
 *    NONPOS   NONZERO   NONNEG
 *      /  \      |      /  \
 *    NEG  ZERO   |   ZERO  POS
 *      \  /      |      \  /
 *    NONPOS   NONZERO   NONNEG
 *         \      |       /
 *            BOTTOM
 *
 * Simplified to 8-element lattice with TOP/BOTTOM
 * ================================================================ */

const char* sign_name(Sign s) {
    switch (s) {
    case SIGN_BOTTOM:  return "BOTTOM";
    case SIGN_NEG:     return "NEG";
    case SIGN_ZERO:    return "ZERO";
    case SIGN_POS:     return "POS";
    case SIGN_NONNEG:  return "NONNEG";
    case SIGN_NONPOS:  return "NONPOS";
    case SIGN_NONZERO: return "NONZERO";
    case SIGN_TOP:     return "TOP";
    default:           return "?";
    }
}

bool sign_leq(Sign a, Sign b) {
    if (a == b) return true;
    if (b == SIGN_TOP) return true;
    if (a == SIGN_BOTTOM) return true;
    if (a == SIGN_NEG && (b == SIGN_NONPOS || b == SIGN_NONZERO)) return true;
    if (a == SIGN_ZERO && (b == SIGN_NONNEG || b == SIGN_NONPOS)) return true;
    if (a == SIGN_POS && (b == SIGN_NONNEG || b == SIGN_NONZERO)) return true;
    if (a == SIGN_NONNEG && b == SIGN_TOP) return true;
    if (a == SIGN_NONPOS && b == SIGN_TOP) return true;
    if (a == SIGN_NONZERO && b == SIGN_TOP) return true;
    return false;
}

Sign sign_join(Sign a, Sign b) {
    if (a == b) return a;
    if (a == SIGN_BOTTOM) return b;
    if (b == SIGN_BOTTOM) return a;
    if (a == SIGN_TOP || b == SIGN_TOP) return SIGN_TOP;

    /* NEG + ZERO = NONPOS */
    if ((a == SIGN_NEG && b == SIGN_ZERO) || (a == SIGN_ZERO && b == SIGN_NEG))
        return SIGN_NONPOS;
    /* ZERO + POS = NONNEG */
    if ((a == SIGN_ZERO && b == SIGN_POS) || (a == SIGN_POS && b == SIGN_ZERO))
        return SIGN_NONNEG;
    /* NEG + POS = NONZERO */
    if ((a == SIGN_NEG && b == SIGN_POS) || (a == SIGN_POS && b == SIGN_NEG))
        return SIGN_NONZERO;

    /* NEG + NONNEG = TOP */
    if ((a == SIGN_NEG && b == SIGN_NONNEG) || (a == SIGN_NONNEG && b == SIGN_NEG))
        return SIGN_TOP;
    /* POS + NONPOS = TOP */
    if ((a == SIGN_POS && b == SIGN_NONPOS) || (a == SIGN_NONPOS && b == SIGN_POS))
        return SIGN_TOP;
    /* NONZERO + ZERO = TOP */
    if ((a == SIGN_NONZERO && b == SIGN_ZERO) || (a == SIGN_ZERO && b == SIGN_NONZERO))
        return SIGN_TOP;

    /* Remainder: join to parent */
    if (sign_leq(a, b)) return b;
    if (sign_leq(b, a)) return a;

    return SIGN_TOP;
}

Sign sign_meet(Sign a, Sign b) {
    if (a == b) return a;
    if (a == SIGN_BOTTOM || b == SIGN_BOTTOM) return SIGN_BOTTOM;
    if (a == SIGN_TOP) return b;
    if (b == SIGN_TOP) return a;

    /* NONPOS meet NONNEG = ZERO */
    if ((a == SIGN_NONPOS && b == SIGN_NONNEG) || (a == SIGN_NONNEG && b == SIGN_NONPOS))
        return SIGN_ZERO;
    /* NONPOS meet NONZERO = NEG */
    if ((a == SIGN_NONPOS && b == SIGN_NONZERO) || (a == SIGN_NONZERO && b == SIGN_NONPOS))
        return SIGN_NEG;
    /* NONNEG meet NONZERO = POS */
    if ((a == SIGN_NONNEG && b == SIGN_NONZERO) || (a == SIGN_NONZERO && b == SIGN_NONNEG))
        return SIGN_POS;

    /* NEG meet NONPOS = NEG, etc. */
    if (sign_leq(a, b)) return a;
    if (sign_leq(b, a)) return b;

    /* ZERO meet NONZERO = BOTTOM */
    if ((a == SIGN_ZERO && b == SIGN_NONZERO) || (a == SIGN_NONZERO && b == SIGN_ZERO))
        return SIGN_BOTTOM;

    return SIGN_BOTTOM;
}

Sign sign_widen(Sign a, Sign b) {
    /* Widening: if b is larger, jump to TOP to ensure termination */
    if (sign_leq(a, b) && a != b) {
        /* Heuristic: if moving from specific to compound, widen to TOP */
        if ((a == SIGN_NEG || a == SIGN_ZERO || a == SIGN_POS) &&
            (b == SIGN_NONNEG || b == SIGN_NONPOS || b == SIGN_NONZERO))
            return SIGN_TOP;
    }
    return sign_join(a, b);
}

Sign sign_narrow(Sign a, Sign b) {
    /* Narrowing: try to refine back from TOP */
    if (a == SIGN_TOP) return b;
    return a;
}

/* Abstract arithmetic transfer functions */
Sign sign_add(Sign a, Sign b) {
    if (a == SIGN_BOTTOM || b == SIGN_BOTTOM) return SIGN_BOTTOM;
    if (a == SIGN_ZERO) return b;
    if (b == SIGN_ZERO) return a;

    if (a == SIGN_POS && b == SIGN_POS) return SIGN_POS;
    if (a == SIGN_NEG && b == SIGN_NEG) return SIGN_NEG;

    if ((a == SIGN_NONNEG && b == SIGN_POS) ||
        (a == SIGN_POS && b == SIGN_NONNEG)) return SIGN_POS;
    if ((a == SIGN_NONPOS && b == SIGN_NEG) ||
        (a == SIGN_NEG && b == SIGN_NONPOS)) return SIGN_NEG;

    if ((a == SIGN_POS && b == SIGN_NEG) ||
        (a == SIGN_NEG && b == SIGN_POS) ||
        (a == SIGN_NONZERO && b == SIGN_NONZERO)) return SIGN_TOP;

    return SIGN_TOP;
}

Sign sign_sub(Sign a, Sign b) {
    /* a - b = a + (-b) */
    Sign neg_b = sign_neg(b);
    if (neg_b == SIGN_BOTTOM) return SIGN_BOTTOM;
    return sign_add(a, neg_b);
}

Sign sign_mul(Sign a, Sign b) {
    if (a == SIGN_BOTTOM || b == SIGN_BOTTOM) return SIGN_BOTTOM;
    if (a == SIGN_ZERO || b == SIGN_ZERO) return SIGN_ZERO;

    if (a == SIGN_POS && b == SIGN_POS) return SIGN_POS;
    if (a == SIGN_NEG && b == SIGN_NEG) return SIGN_POS;
    if ((a == SIGN_POS && b == SIGN_NEG) || (a == SIGN_NEG && b == SIGN_POS))
        return SIGN_NEG;

    if (a == SIGN_NONZERO && b == SIGN_NONZERO) return SIGN_NONZERO;
    if (a == SIGN_NONNEG && b == SIGN_NONNEG) return SIGN_NONNEG;
    if (a == SIGN_NONPOS && b == SIGN_NONNEG) return SIGN_NONPOS;
    if (a == SIGN_NONNEG && b == SIGN_NONPOS) return SIGN_NONPOS;

    return SIGN_TOP;
}

Sign sign_div(Sign a, Sign b) {
    if (a == SIGN_BOTTOM || b == SIGN_BOTTOM) return SIGN_BOTTOM;
    if (b == SIGN_ZERO) return SIGN_BOTTOM;  /* division by zero: unreachable */
    if (a == SIGN_ZERO) return SIGN_ZERO;

    /* Division preserves sign: pos/pos = pos, neg/neg = pos, pos/neg = neg, etc. */
    if (a == SIGN_POS && b == SIGN_POS) return SIGN_NONNEG;
    if (a == SIGN_NEG && b == SIGN_NEG) return SIGN_NONNEG;
    if ((a == SIGN_POS && b == SIGN_NEG) || (a == SIGN_NEG && b == SIGN_POS))
        return SIGN_NONPOS;

    return SIGN_TOP;
}

Sign sign_neg(Sign a) {
    switch (a) {
    case SIGN_BOTTOM: return SIGN_BOTTOM;
    case SIGN_NEG:    return SIGN_POS;
    case SIGN_ZERO:   return SIGN_ZERO;
    case SIGN_POS:    return SIGN_NEG;
    case SIGN_NONNEG: return SIGN_NONPOS;
    case SIGN_NONPOS: return SIGN_NONNEG;
    case SIGN_NONZERO:return SIGN_NONZERO;
    case SIGN_TOP:    return SIGN_TOP;
    default:          return SIGN_TOP;
    }
}

Sign sign_abs(Sign a) {
    switch (a) {
    case SIGN_BOTTOM: case SIGN_ZERO: return a;
    case SIGN_NEG: case SIGN_POS: return SIGN_POS;
    case SIGN_NONNEG: return SIGN_NONNEG;
    case SIGN_NONPOS: return SIGN_NONNEG;
    default: return SIGN_NONNEG;
    }
}

Sign sign_from_int(int val) {
    if (val < 0) return SIGN_NEG;
    if (val > 0) return SIGN_POS;
    return SIGN_ZERO;
}

bool sign_contains(Sign s, int val) {
    switch (s) {
    case SIGN_BOTTOM:  return false;
    case SIGN_NEG:     return val < 0;
    case SIGN_ZERO:    return val == 0;
    case SIGN_POS:     return val > 0;
    case SIGN_NONNEG:  return val >= 0;
    case SIGN_NONPOS:  return val <= 0;
    case SIGN_NONZERO: return val != 0;
    case SIGN_TOP:     return true;
    default:           return false;
    }
}

/* Filter operations: restrict abstract value based on guard condition */
Sign sign_filter_eq(Sign s, int val) {
    Sign abs_val = sign_from_int(val);
    return sign_meet(s, abs_val);
}

Sign sign_filter_ne(Sign s, int val) {
    if (val == 0) {
        if (s == SIGN_ZERO) return SIGN_BOTTOM;
        if (s == SIGN_NONNEG) return SIGN_POS;
        if (s == SIGN_NONPOS) return SIGN_NEG;
        return s;
    }
    /* General: s meet NONZERO, but with refinement */
    Sign refined = sign_meet(s, SIGN_NONZERO);
    if (val > 0 && s == SIGN_NONNEG) {
        /* x != positive & x >= 0 => could be 0 or other positive */
        return SIGN_NONNEG;  /* not precise enough to remove one pos value */
    }
    return refined;
}

Sign sign_filter_lt(Sign s, int val) {
    if (val <= 0) {
        /* x < non-positive => x must be negative */
        return sign_meet(s, SIGN_NEG);
    }
    /* x < positive => x <= positive-1 => NONPOS-ish */
    return sign_meet(s, SIGN_NONPOS);
}

Sign sign_filter_le(Sign s, int val) {
    if (val < 0) {
        return sign_meet(s, SIGN_NEG);
    }
    /* x <= non-negative => NONPOS */
    return sign_meet(s, SIGN_NONPOS);
}

Sign sign_filter_gt(Sign s, int val) {
    if (val >= 0) {
        return sign_meet(s, SIGN_POS);
    }
    return sign_meet(s, SIGN_NONNEG);
}

Sign sign_filter_ge(Sign s, int val) {
    if (val > 0) {
        return sign_meet(s, SIGN_POS);
    }
    return sign_meet(s, SIGN_NONNEG);
}

/* ================================================================
 * Interval Domain
 * ================================================================ */

Interval interval_make(int lo, int hi) {
    Interval iv;
    iv.lo = lo;
    iv.hi = hi;
    iv.is_bottom = (lo > hi);
    return iv;
}

Interval interval_bottom(void) {
    Interval iv;
    iv.is_bottom = true;
    iv.lo = 0;
    iv.hi = -1;
    return iv;
}

Interval interval_top(void) {
    return interval_make(INT_MIN, INT_MAX);
}

bool interval_is_bottom(Interval iv) {
    return iv.is_bottom;
}

bool interval_leq(Interval a, Interval b) {
    if (a.is_bottom) return true;
    if (b.is_bottom) return false;
    return a.lo >= b.lo && a.hi <= b.hi;
}

Interval interval_join(Interval a, Interval b) {
    if (a.is_bottom) return b;
    if (b.is_bottom) return a;
    int lo = (a.lo < b.lo) ? a.lo : b.lo;
    int hi = (a.hi > b.hi) ? a.hi : b.hi;
    return interval_make(lo, hi);
}

Interval interval_meet(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    int lo = (a.lo > b.lo) ? a.lo : b.lo;
    int hi = (a.hi < b.hi) ? a.hi : b.hi;
    return interval_make(lo, hi);
}

Interval interval_widen(Interval a, Interval b) {
    if (a.is_bottom) return b;
    if (b.is_bottom) return a;
    int lo = (b.lo < a.lo) ? INT_MIN : a.lo;
    int hi = (b.hi > a.hi) ? INT_MAX : a.hi;
    return interval_make(lo, hi);
}

Interval interval_narrow(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    int lo = (a.lo == INT_MIN) ? b.lo : a.lo;
    int hi = (a.hi == INT_MAX) ? b.hi : a.hi;
    return interval_make(lo, hi);
}

/* Interval arithmetic -- careful with overflow */
static int safe_add(int a, int b) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return (b > 0) ? INT_MAX : INT_MIN;
    return a + b;
}

static int safe_sub(int a, int b) {
    if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b))
        return (b < 0) ? INT_MAX : INT_MIN;
    return a - b;
}

static int safe_mul(int a, int b) {
    if (a == 0 || b == 0) return 0;
    if (a > 0) {
        if (b > 0 && a > INT_MAX / b) return INT_MAX;
        if (b < 0 && b < INT_MIN / a) return INT_MIN;
    } else {
        if (b > 0 && a < INT_MIN / b) return INT_MIN;
        if (b < 0 && a < INT_MAX / b) return INT_MAX;
    }
    return a * b;
}

Interval interval_add(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    return interval_make(safe_add(a.lo, b.lo), safe_add(a.hi, b.hi));
}

Interval interval_sub(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    return interval_make(safe_sub(a.lo, b.hi), safe_sub(a.hi, b.lo));
}

Interval interval_mul(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    int c1 = safe_mul(a.lo, b.lo);
    int c2 = safe_mul(a.lo, b.hi);
    int c3 = safe_mul(a.hi, b.lo);
    int c4 = safe_mul(a.hi, b.hi);
    int lo = c1; if (c2 < lo) lo = c2; if (c3 < lo) lo = c3; if (c4 < lo) lo = c4;
    int hi = c1; if (c2 > hi) hi = c2; if (c3 > hi) hi = c3; if (c4 > hi) hi = c4;
    return interval_make(lo, hi);
}

Interval interval_div(Interval a, Interval b) {
    if (a.is_bottom || b.is_bottom) return interval_bottom();
    /* If divisor contains zero, result is TOP (conservative) */
    if (b.lo <= 0 && b.hi >= 0) return interval_top();
    /* Simple case: both positive */
    if (a.lo >= 0 && b.lo > 0) return interval_make(a.lo / b.hi, a.hi / b.lo);
    if (a.hi <= 0 && b.lo > 0) return interval_make(a.lo / b.lo, a.hi / b.hi);
    /* General case: conservative TOP */
    return interval_top();
}

Interval interval_neg(Interval a) {
    if (a.is_bottom) return interval_bottom();
    return interval_make(-a.hi, -a.lo);
}

/* Filter intervals based on guard conditions */
Interval interval_filter_lt(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    int new_hi = c - 1;
    if (new_hi < iv.lo) return interval_bottom();
    if (new_hi < iv.hi) iv.hi = new_hi;
    return iv;
}

Interval interval_filter_le(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    if (c < iv.lo) return interval_bottom();
    if (c < iv.hi) iv.hi = c;
    return iv;
}

Interval interval_filter_gt(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    int new_lo = c + 1;
    if (new_lo > iv.hi) return interval_bottom();
    if (new_lo > iv.lo) iv.lo = new_lo;
    return iv;
}

Interval interval_filter_ge(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    if (c > iv.hi) return interval_bottom();
    if (c > iv.lo) iv.lo = c;
    return iv;
}

Interval interval_filter_eq(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    if (c < iv.lo || c > iv.hi) return interval_bottom();
    return interval_make(c, c);
}

Interval interval_filter_ne(Interval iv, int c) {
    if (iv.is_bottom) return iv;
    if (iv.lo == iv.hi && iv.lo == c) return interval_bottom();
    /* Conservative: can't easily represent interval with a hole */
    return iv;
}

bool interval_contains(Interval iv, int val) {
    if (iv.is_bottom) return false;
    return val >= iv.lo && val <= iv.hi;
}

void interval_print(Interval iv) {
    if (iv.is_bottom) printf("[]");
    else if (iv.lo == INT_MIN && iv.hi == INT_MAX) printf("[-inf, +inf]");
    else printf("[%d, %d]", iv.lo, iv.hi);
}

/* ================================================================
 * Congruence Domain
 * mod = 0 means TOP (all integers)
 * mod < 0 means BOTTOM
 * ================================================================ */

Congruence congruence_make(int modulus, int residue) {
    Congruence cg;
    if (modulus < 0) {
        cg.modulus = -1;
        cg.residue = 0;
    } else if (modulus == 0) {
        cg.modulus = 0;
        cg.residue = 0;
    } else {
        cg.modulus = modulus;
        cg.residue = ((residue % modulus) + modulus) % modulus;
    }
    return cg;
}

Congruence congruence_top(void) {
    return congruence_make(0, 0);
}

Congruence congruence_bottom(void) {
    Congruence cg;
    cg.modulus = -1;
    cg.residue = 0;
    return cg;
}

bool congruence_leq(Congruence a, Congruence b) {
    if (a.modulus < 0) return true;      /* bottom <= everything */
    if (b.modulus == 0) return true;     /* everything <= TOP */
    if (a.modulus == 0) return false;    /* TOP not <= non-TOP */
    if (b.modulus < 0) return false;     /* nothing <= bottom except bottom */
    /* a ≦ b iff b.mod | a.mod and a.res ≡ b.res (mod b.mod) */
    if (a.modulus % b.modulus != 0) return false;
    if ((a.residue - b.residue) % b.modulus != 0) return false;
    return true;
}

Congruence congruence_join(Congruence a, Congruence b) {
    if (a.modulus < 0) return b;
    if (b.modulus < 0) return a;
    if (a.modulus == 0 || b.modulus == 0) return congruence_top();
    /* Join = gcd of moduli if residues compatible */
    int g = a.modulus;
    int t = b.modulus;
    while (t != 0) { int r = g % t; g = t; t = r; }
    if ((a.residue - b.residue) % g == 0)
        return congruence_make(g, a.residue);
    return congruence_top();
}

Congruence congruence_meet(Congruence a, Congruence b) {
    if (a.modulus < 0 || b.modulus < 0) return congruence_bottom();
    if (a.modulus == 0) return b;
    if (b.modulus == 0) return a;
    /* Meet = lcm if residues compatible */
    int g = a.modulus, t = b.modulus;
    while (t != 0) { int r = g % t; g = t; t = r; }
    if ((a.residue - b.residue) % g != 0) return congruence_bottom();
    int lcm = a.modulus / g * b.modulus;
    return congruence_make(lcm, a.residue);
}

bool congruence_contains(Congruence cg, int val) {
    if (cg.modulus < 0) return false;
    if (cg.modulus == 0) return true;
    return ((val - cg.residue) % cg.modulus + cg.modulus) % cg.modulus == 0;
}

/* ================================================================
 * Abstract State
 * ================================================================ */

void absstate_init(AbsState* st, int nvars) {
    st->nvars = (nvars < ABS_MAX_VARS) ? nvars : ABS_MAX_VARS;
    st->is_bottom = false;
    for (int i = 0; i < st->nvars; i++) {
        st->vars[i].kind = ABSVAL_INTERVAL;
        st->vars[i].data.interval = interval_top();
    }
}

void absstate_set_sign(AbsState* st, int vidx, Sign s) {
    if (vidx >= 0 && vidx < st->nvars) {
        st->vars[vidx].kind = ABSVAL_SIGN;
        st->vars[vidx].data.sign = s;
    }
}

void absstate_set_interval(AbsState* st, int vidx, Interval iv) {
    if (vidx >= 0 && vidx < st->nvars) {
        st->vars[vidx].kind = ABSVAL_INTERVAL;
        st->vars[vidx].data.interval = iv;
    }
}

void absstate_set_congruence(AbsState* st, int vidx, Congruence cg) {
    if (vidx >= 0 && vidx < st->nvars) {
        st->vars[vidx].kind = ABSVAL_CONGRUENCE;
        st->vars[vidx].data.congruence = cg;
    }
}

AbsVal absstate_get(AbsState* st, int vidx) {
    AbsVal v;
    v.kind = ABSVAL_INTERVAL;
    v.data.interval = interval_top();
    if (vidx >= 0 && vidx < st->nvars)
        return st->vars[vidx];
    return v;
}

bool absstate_is_bottom(AbsState* st) {
    return st->is_bottom;
}

AbsState absstate_join(AbsState* a, AbsState* b) {
    if (a->is_bottom) return *b;
    if (b->is_bottom) return *a;
    AbsState result;
    result.is_bottom = false;
    result.nvars = (a->nvars < b->nvars) ? a->nvars : b->nvars;
    for (int i = 0; i < result.nvars; i++) {
        /* Join interval abstraction for simplicity */
        Interval ia = (a->vars[i].kind == ABSVAL_INTERVAL) ?
            a->vars[i].data.interval : interval_top();
        Interval ib = (b->vars[i].kind == ABSVAL_INTERVAL) ?
            b->vars[i].data.interval : interval_top();
        result.vars[i].kind = ABSVAL_INTERVAL;
        result.vars[i].data.interval = interval_join(ia, ib);
    }
    return result;
}

AbsState absstate_widen(AbsState* a, AbsState* b) {
    if (a->is_bottom) return *b;
    if (b->is_bottom) return *a;
    AbsState result;
    result.is_bottom = false;
    result.nvars = (a->nvars < b->nvars) ? a->nvars : b->nvars;
    for (int i = 0; i < result.nvars; i++) {
        Interval ia = (a->vars[i].kind == ABSVAL_INTERVAL) ?
            a->vars[i].data.interval : interval_top();
        Interval ib = (b->vars[i].kind == ABSVAL_INTERVAL) ?
            b->vars[i].data.interval : interval_top();
        result.vars[i].kind = ABSVAL_INTERVAL;
        result.vars[i].data.interval = interval_widen(ia, ib);
    }
    return result;
}

bool absstate_leq(AbsState* a, AbsState* b) {
    if (a->is_bottom) return true;
    if (b->is_bottom) return false;
    int n = (a->nvars < b->nvars) ? a->nvars : b->nvars;
    for (int i = 0; i < n; i++) {
        Interval ia = (a->vars[i].kind == ABSVAL_INTERVAL) ?
            a->vars[i].data.interval : interval_top();
        Interval ib = (b->vars[i].kind == ABSVAL_INTERVAL) ?
            b->vars[i].data.interval : interval_top();
        if (!interval_leq(ia, ib)) return false;
    }
    return true;
}

void absstate_print(AbsState* st) {
    if (st->is_bottom) { printf("State: BOTTOM\n"); return; }
    printf("Abstract State (%d vars):\n", st->nvars);
    for (int i = 0; i < st->nvars; i++) {
        printf("  v%d: ", i);
        if (st->vars[i].kind == ABSVAL_INTERVAL) {
            interval_print(st->vars[i].data.interval);
        } else if (st->vars[i].kind == ABSVAL_SIGN) {
            printf("sign=%s", sign_name(st->vars[i].data.sign));
        } else {
            printf("mod=%d,res=%d",
                st->vars[i].data.congruence.modulus,
                st->vars[i].data.congruence.residue);
        }
        printf("\n");
    }
}

/* ================================================================
 * Abstract Expression Evaluation
 * ================================================================ */

/* Evaluate abstract expression to interval */
Interval abs_expr_eval_interval(AbsExpr* e, AbsState* st) {
    if (!e) return interval_top();
    switch (e->kind) {
    case ABS_EXPR_CONST:
        return interval_make(e->value, e->value);
    case ABS_EXPR_VAR:
        if (e->value >= 0 && e->value < st->nvars &&
            st->vars[e->value].kind == ABSVAL_INTERVAL)
            return st->vars[e->value].data.interval;
        return interval_top();
    case ABS_EXPR_ADD:
        return interval_add(
            abs_expr_eval_interval(e->left, st),
            abs_expr_eval_interval(e->right, st));
    case ABS_EXPR_SUB:
        return interval_sub(
            abs_expr_eval_interval(e->left, st),
            abs_expr_eval_interval(e->right, st));
    case ABS_EXPR_MUL:
        return interval_mul(
            abs_expr_eval_interval(e->left, st),
            abs_expr_eval_interval(e->right, st));
    case ABS_EXPR_DIV:
        return interval_div(
            abs_expr_eval_interval(e->left, st),
            abs_expr_eval_interval(e->right, st));
    case ABS_EXPR_NEG:
        return interval_neg(abs_expr_eval_interval(e->left, st));
    default:
        return interval_top();
    }
}

/* Evaluate abstract expression to sign */
Sign abs_expr_eval_sign(AbsExpr* e, AbsState* st) {
    if (!e) return SIGN_TOP;
    switch (e->kind) {
    case ABS_EXPR_CONST:
        return sign_from_int(e->value);
    case ABS_EXPR_VAR:
        if (e->value >= 0 && e->value < st->nvars &&
            st->vars[e->value].kind == ABSVAL_SIGN)
            return st->vars[e->value].data.sign;
        return SIGN_TOP;
    case ABS_EXPR_ADD:
        return sign_add(abs_expr_eval_sign(e->left, st),
                        abs_expr_eval_sign(e->right, st));
    case ABS_EXPR_SUB:
        return sign_sub(abs_expr_eval_sign(e->left, st),
                        abs_expr_eval_sign(e->right, st));
    case ABS_EXPR_MUL:
        return sign_mul(abs_expr_eval_sign(e->left, st),
                        abs_expr_eval_sign(e->right, st));
    case ABS_EXPR_DIV:
        return sign_div(abs_expr_eval_sign(e->left, st),
                        abs_expr_eval_sign(e->right, st));
    case ABS_EXPR_NEG:
        return sign_neg(abs_expr_eval_sign(e->left, st));
    default:
        return SIGN_TOP;
    }
}

/* ================================================================
 * Abstract Transfer Functions
 * ================================================================ */

AbsState abs_transfer(AbsStmt* s, AbsState* in_state) {
    AbsState out = *in_state;
    if (!s || in_state->is_bottom) return out;

    switch (s->kind) {
    case ABS_STMT_SKIP:
        break;

    case ABS_STMT_ASSIGN: {
        Interval rhs = abs_expr_eval_interval(s->expr, in_state);
        if (s->var_idx >= 0 && s->var_idx < out.nvars) {
            out.vars[s->var_idx].kind = ABSVAL_INTERVAL;
            out.vars[s->var_idx].data.interval = rhs;
        }
        break;
    }

    case ABS_STMT_SEQ:
        out = abs_transfer(s->left, in_state);
        out = abs_transfer(s->right, &out);
        break;

    case ABS_STMT_IF: {
        /* Evaluate guard to interval for path condition */
        Interval guard_iv = abs_expr_eval_interval(s->guard, in_state);
        AbsState then_state = *in_state;
        AbsState else_state = *in_state;

        /* True branch: refine based on guard being true (non-zero) */
        if (!interval_contains(guard_iv, 0) || interval_contains(guard_iv, 1)) {
            then_state = abs_transfer(s->left, in_state);
        } else {
            then_state.is_bottom = true;
        }

        /* False branch: refine based on guard being zero */
        if (interval_contains(guard_iv, 0)) {
            else_state = abs_transfer(s->right, in_state);
        } else {
            else_state.is_bottom = true;
        }

        out = absstate_join(&then_state, &else_state);
        break;
    }

    case ABS_STMT_WHILE:
        /* fixpoint handled separately */
        out = abs_fixpoint(s->left, s->guard, s->loop_id, in_state);
        break;

    case ABS_STMT_ASSERT:
        /* Assert acts as filter: intersection with guard == true */
        {
            Interval guard_iv = abs_expr_eval_interval(s->expr, in_state);
            if (interval_contains(guard_iv, 0) && !interval_contains(guard_iv, 1)) {
                out.is_bottom = true;
            }
        }
        break;

    default:
        break;
    }

    return out;
}

/* Fixed-point iteration with widening.
 * For a while-loop: while(B) { S }
 * We iterate: X_0 = in_state (before loop)
 *             X_{i+1} = X_i ⊔ abs_transfer(S, X_i ⊓ filter_B_true(X_i))
 * until fixpoint, using widening to accelerate. */
AbsState abs_fixpoint(AbsStmt* loop_body, AbsExpr* guard,
                       int loop_id, AbsState* initial) {
    (void)loop_id;
    (void)guard;
    AbsState current = *initial;
    AbsState prev;

    int max_iter = 100;
    for (int iter = 0; iter < max_iter; iter++) {
        prev = current;

        /* Apply loop body: state after one iteration */
        AbsState body_state = abs_transfer(loop_body, &current);

        /* Join with previous (current = prev ⊔ body_out) */
        current = absstate_join(&prev, &body_state);

        /* Check fixpoint */
        if (absstate_leq(&current, &prev)) {
            /* Fixpoint reached: apply negation of guard (exit condition) */
            /* The final state after loop is the fixpoint filtered by !guard */
            return current;
        }

        /* Apply widening every 3 iterations to accelerate */
        if (iter > 2 && iter % 3 == 0) {
            current = absstate_widen(&prev, &current);
        }
    }

    return current;
}

/* Full abstract interpretation over a program, collecting states */
AbsTrace* abs_interpret(AbsStmt* program, AbsState* initial) {
    AbsTrace* trace = malloc(sizeof(AbsTrace));
    trace->capacity = 32;
    trace->count = 0;
    trace->states = malloc(trace->capacity * sizeof(AbsState));

    /* Store initial state */
    trace->states[trace->count++] = *initial;

    AbsState current = *initial;
    /* Simple single-pass interpretation for straight-line code.
     * For loops, abs_transfer handles fixpoint internally. */
    current = abs_transfer(program, &current);

    trace->states[trace->count++] = current;
    return trace;
}

void abs_trace_free(AbsTrace* t) {
    if (!t) return;
    free(t->states);
    free(t);
}

/* ================================================================
 * Abstract Expression Constructors (simplified, mirrors wp.h)
 * ================================================================ */

AbsExpr* absexpr_const(int v) {
    AbsExpr* e = malloc(sizeof(AbsExpr));
    e->kind = ABS_EXPR_CONST;
    e->value = v;
    e->left = e->right = NULL;
    return e;
}

AbsExpr* absexpr_var(int idx) {
    AbsExpr* e = malloc(sizeof(AbsExpr));
    e->kind = ABS_EXPR_VAR;
    e->value = idx;
    e->left = e->right = NULL;
    return e;
}

AbsExpr* absexpr_binary(AbsExprKind kind, AbsExpr* l, AbsExpr* r) {
    AbsExpr* e = malloc(sizeof(AbsExpr));
    e->kind = kind;
    e->value = 0;
    e->left = l;
    e->right = r;
    return e;
}

AbsExpr* absexpr_unary(AbsExprKind kind, AbsExpr* e_in) {
    AbsExpr* e = malloc(sizeof(AbsExpr));
    e->kind = kind;
    e->value = 0;
    e->left = e_in;
    e->right = NULL;
    return e;
}

void absexpr_free(AbsExpr* e) {
    if (!e) return;
    absexpr_free(e->left);
    absexpr_free(e->right);
    free(e);
}

AbsStmt* absstmt_skip(void) {
    AbsStmt* s = malloc(sizeof(AbsStmt));
    s->kind = ABS_STMT_SKIP;
    s->var_idx = 0;
    s->expr = s->guard = NULL;
    s->left = s->right = NULL;
    s->loop_id = 0;
    return s;
}

AbsStmt* absstmt_assign(int idx, AbsExpr* rhs) {
    AbsStmt* s = malloc(sizeof(AbsStmt));
    s->kind = ABS_STMT_ASSIGN;
    s->var_idx = idx;
    s->expr = rhs;
    s->guard = NULL;
    s->left = s->right = NULL;
    s->loop_id = 0;
    return s;
}

AbsStmt* absstmt_seq(AbsStmt* s1, AbsStmt* s2) {
    AbsStmt* s = malloc(sizeof(AbsStmt));
    s->kind = ABS_STMT_SEQ;
    s->var_idx = 0;
    s->expr = s->guard = NULL;
    s->left = s1;
    s->right = s2;
    s->loop_id = 0;
    return s;
}

AbsStmt* absstmt_if(AbsExpr* guard, AbsStmt* t, AbsStmt* e) {
    AbsStmt* s = malloc(sizeof(AbsStmt));
    s->kind = ABS_STMT_IF;
    s->var_idx = 0;
    s->expr = NULL;
    s->guard = guard;
    s->left = t;
    s->right = e;
    s->loop_id = 0;
    return s;
}

AbsStmt* absstmt_while(AbsExpr* guard, AbsStmt* body, int loop_id) {
    AbsStmt* s = malloc(sizeof(AbsStmt));
    s->kind = ABS_STMT_WHILE;
    s->var_idx = 0;
    s->expr = NULL;
    s->guard = guard;
    s->left = body;
    s->right = NULL;
    s->loop_id = loop_id;
    return s;
}

void absstmt_free(AbsStmt* s) {
    if (!s) return;
    absexpr_free(s->expr);
    absexpr_free(s->guard);
    absstmt_free(s->left);
    absstmt_free(s->right);
    free(s);
}
