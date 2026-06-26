#ifndef ROBUSTNESS_H
#define ROBUSTNESS_H

#include <stdbool.h>

/* L1: Uncertainty model — additive and multiplicative uncertainty
 * Additive:  G_true = G_nominal + W_a * Δ,   ||Δ||_∞ ≤ 1
 * Multiplicative: G_true = G_nominal * (1 + W_m * Δ), ||Δ||_∞ ≤ 1
 */
typedef struct {
    int type;         /* 0=additive, 1=multiplicative input, 2=multiplicative output */
    double weight;    /* Uncertainty weight magnitude */
    double** A;       /* System dynamics of nominal plant */
    double** B;
    double** C;
    int n;
} UncertaintyModel;

/* L1: Robust stability test result */
typedef struct {
    bool is_robustly_stable;
    double stability_margin;   /* Structured singular value μ */
    double worst_case_pole;    /* Real part of worst-case eigenvalue */
} RobustStabilityResult;

/* L4: Small Gain Theorem — fundamental robust stability condition
 * If ||G(s)||_∞ * ||Δ(s)||_∞ < 1, then the interconnected system is stable
 * for all stable perturbations Δ with ||Δ||_∞ ≤ 1.
 * Reference: Zames, G. (1966) "On the Input-Output Stability of Nonlinear Feedback Systems"
 */
bool small_gain_test(double plant_gain, double uncertainty_gain);

/* L3: H-infinity norm estimation via frequency grid
 * ||G||_∞ = sup_ω σ_max(G(jω))
 * Computed by evaluating maximum singular value over a logarithmically spaced frequency grid
 */
double hinf_norm_estimate(double** A, double** B, double** C, int n,
                          double w_min, double w_max, int n_points);

/* L5: Structured singular value μ (lower bound via power iteration)
 * μ(M) = 1 / min{σ̅(Δ) : det(I - MΔ) = 0, Δ structured}
 * Reference: Doyle, J.C. (1982) "Analysis of Feedback Systems with Structured Uncertainty"
 */
double mu_lower_bound(double** M, int n, int n_blocks, int* block_sizes);

/* L8: Kharitonov's Theorem — robust stability of interval polynomials
 * For polynomial a(s) = a₀ + a₁s + a₂s² + ... + a_n s^n
 * with a_i ∈ [a_i⁻, a_i⁺], stability requires only 4 polynomials tested
 * Reference: Kharitonov, V.L. (1978) "Asymptotic Stability of an Equilibrium..."
 */
typedef struct {
    double* coeff_min;
    double* coeff_max;
    int degree;
} IntervalPolynomial;

bool kharitonov_test(IntervalPolynomial* ip);

/* L2: Gain and phase margin computation for robust stability assessment */
typedef struct {
    double gain_margin;    /* In dB */
    double phase_margin;   /* In degrees */
    double delay_margin;   /* Maximum allowable time delay in seconds */
} StabilityMargins;

/* L4: Nyquist stability criterion — encirclement test
 * Z = N + P, where Z = closed-loop RHP poles, N = CCW encirclements of -1,
 * P = open-loop RHP poles. System is stable iff Z = 0.
 * Reference: Nyquist, H. (1932) "Regeneration Theory"
 */
typedef struct {
    int n_encirclements;   /* Net counterclockwise encirclements of -1 */
    int open_loop_rhp;    /* Number of open-loop RHP poles */
    int closed_loop_rhp;  /* Predicted closed-loop RHP poles */
    bool is_stable;
} NyquistResult;

/* L5: Nyquist analysis — evaluates G(jω) along the Nyquist contour
 * Contour: jω for ω∈[-R,R] + semicircle Re^{jθ} for θ∈[-π/2,π/2], R→∞
 * Computes winding number around -1+0j
 */
NyquistResult nyquist_analysis(double* num, int num_len, double* den, int den_len);

/* L7: Robust PID tuning — find gains that maximize robustness margin */
typedef struct {
    double Kp, Ki, Kd;
    double robustness_margin;
} RobustPIDTuning;

RobustPIDTuning robust_pid_tune(double** A, double** B, double** C, int n,
                                 double w_min, double w_max);

/* L4: Passivity theorem — passivity-based stability
 * If both H1 and H2 are strictly passive (or one passive and one strictly passive),
 * then the feedback interconnection is stable.
 */
typedef struct {
    bool is_passive;       /* Passivity: ∫₀ᵗ y(τ)'u(τ) dτ ≥ 0 for all t */
    bool is_strictly_input_passive;  /* Has dissipation ρ > 0 */
    bool is_strictly_output_passive;
    double io_gain;        /* L2 gain bound */
} PassivityResult;

PassivityResult check_passivity(double** A, double** B, double** C, int n);

#endif
