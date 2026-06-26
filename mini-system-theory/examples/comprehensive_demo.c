/* comprehensive_demo.c - Exercises all new public API modules
 * signal_flow, optimal_control, nonlinear, discrete_event, robustness
 */
#include "signal_flow.h"
#include "optimal_control.h"
#include "nonlinear.h"
#include "discrete_event.h"
#include "robustness.h"
#include "state_space.h"
#include "feedback.h"
#include "stability.h"
#include "queue_model.h"
#include "system_dynamics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static double** mat_create(int r, int c) {
    double** m = (double**)malloc(r * sizeof(double*));
    for (int i = 0; i < r; i++)
        m[i] = (double*)calloc(c, sizeof(double));
    return m;
}

static void mat_free(double** m, int r) {
    for (int i = 0; i < r; i++) free(m[i]);
    free(m);
}

int main(void) {
    printf("=== Comprehensive System Theory Demo ===\n\n");

    /* 1. Signal Flow Graph */
    printf("--- 1. Signal Flow Graph ---\n");
    SignalFlowGraph* g = sfg_create();
    int n0 = sfg_add_node(g);
    int n1 = sfg_add_node(g);
    int n2 = sfg_add_node(g);
    int n3 = sfg_add_node(g);
    sfg_add_edge(g, n0, n1, 2.0);
    sfg_add_edge(g, n1, n2, 3.0);
    sfg_add_edge(g, n2, n3, 4.0);
    sfg_add_edge(g, n2, n1, -1.0);
    sfg_add_edge(g, n1, n0, 0.5);
    double tf = sfg_masons_gain(g, n0, n3);
    printf("Mason gain: %.6f\n", tf);
    printf("Series(2,3)=%.1f Feedback(6,-1)=%.4f\n",
           block_diagram_series(2.0,3.0), block_diagram_feedback(6.0,-1.0,true));
    sfg_free(g);

    /* 2. Block Diagram Algebra + Nyquist */
    printf("\n--- 2. Block Diagram & Nyquist ---\n");
    double num[]={1.0}, den[]={1.0,2.0,1.0};
    NyquistResult ny = nyquist_analysis(num,1,den,3);
    printf("Nyquist: OLRHP=%d CLRHP=%d stable=%s\n",
           ny.open_loop_rhp, ny.closed_loop_rhp, ny.is_stable?"yes":"no");
    double cmin[]={1,3,3,1}, cmax[]={2,4,4,1};
    IntervalPolynomial ip={cmin,cmax,3};
    printf("Kharitonov: %s\n", kharitonov_test(&ip)?"stable":"unstable");

    /* 3. H-inf norm + Passivity */
    printf("\n--- 3. H-inf & Passivity ---\n");
    double** Ah=mat_create(1,1); double** Bh=mat_create(1,1); double** Ch=mat_create(1,1);
    Ah[0][0]=-1.0; Bh[0][0]=1.0; Ch[0][0]=1.0;
    printf("H-inf norm 1/(s+1): %.6f\n", hinf_norm_estimate(Ah,Bh,Ch,1,0.01,100.0,50));
    printf("Small gain(0.8,0.5): %s\n", small_gain_test(0.8,0.5)?"stable":"unstable");
    PassivityResult pas = check_passivity(Ah,Bh,Ch,1);
    printf("Passivity(1/(s+1)): %s gain=%.4f\n", pas.is_passive?"passive":"not",pas.io_gain);
    mat_free(Ah,1);mat_free(Bh,1);mat_free(Ch,1);

    /* 4. Nonlinear Systems */
    printf("\n--- 4. Nonlinear Systems ---\n");
    double xv[]={2.0,0.0}, uv[]={2.0}, xn[2];
    rk4_step(vdp_dynamics,xv,uv,0.0,0.01,2,1,xn);
    printf("VdP RK4 step: [%.6f %.6f]\n", xn[0], xn[1]);
    printf("DF sat=%.4f relay=%.4f deadzone=%.4f\n",
           describing_function_saturation(2.0,1.0,1.0),
           describing_function_relay(1.0,1.0),
           describing_function_deadzone(2.0,1.0,0.5));
    EquilibriumPoint* eq = vdp_equilibrium();
    NonlinearSystem ns; ns.dynamics=vdp_dynamics; ns.n_states=2; ns.n_inputs=1;
    double** Al; double** Bl;
    linearize_at_eq(&ns,eq,&Al,&Bl);
    double eig[4];
    int stab = lyapunov_indirect(Al,2,eig);
    printf("Lyapunov indirect: dominant=%.4f%+.4fi stable=%s\n",
           eig[0],eig[1], stab==1?"yes":(stab==-1?"unstable":"marginal"));
    mat_free(Al,2); mat_free(Bl,2);
    free(eq->x_eq); free(eq->u_eq); free(eq);

    /* 5. Discrete-Event */
    printf("\n--- 5. Discrete-Event Simulation ---\n");
    FSM* fsm = fsm_create(3,2);
    fsm_set_transition(fsm,0,0,1); fsm_set_transition(fsm,1,1,2); fsm_set_transition(fsm,2,0,0);
    printf("FSM: 0-ev0->%d ev1->%d ev0->%d\n",
           fsm_step(fsm,0,NULL), fsm_step(fsm,1,NULL), fsm_step(fsm,0,NULL));
    fsm_free(fsm);
    PetriNet* pn = pn_create(3,2);
    pn_set_pre(pn,0,0,1); pn_set_post(pn,0,1,1);
    pn_set_pre(pn,1,1,1); pn_set_post(pn,1,2,1);
    pn_set_marking(pn,0,1);
    if(pn_is_enabled(pn,0)) pn_fire(pn,0);
    printf("Petri net: P0=%d P1=%d P2=%d\n",pn->marking[0],pn->marking[1],pn->marking[2]);
    pn_free(pn);
    TrafficLight* tl = tl_create(5.0,1.0,5.0);
    tl_update(tl,6.0);
    printf("Traffic light after 6s: E=%d N=%d\n",tl->east_state,tl->north_state);
    tl_free(tl);

    /* 6. Gramians */
    printf("\n--- 6. Controllability & Observability ---\n");
    StateSpace* ssg = ss_create(2,1,1);
    ssg->A[0][0]=-2; ssg->A[0][1]=1; ssg->A[1][0]=0; ssg->A[1][1]=-3;
    ssg->B[0][0]=0; ssg->B[1][0]=1; ssg->C[0][0]=1; ssg->C[0][1]=0;
    double** Wc = ss_controllability_gramian(ssg);
    printf("Ctrl Gramian tr=%.6f ctrl=%s obs=%s\n",
           Wc[0][0]+Wc[1][1],
           ss_is_controllable(ssg)?"yes":"no", ss_is_observable(ssg)?"yes":"no");
    ss_free_gramian(Wc,2); ss_free(ssg);

    /* 7. Queueing */
    printf("\n--- 7. Queueing Networks ---\n");
    printf("M/G/1(cv^2=0.5): L=%.4f W=%.4f\n",
           mg1_avg_queue_length(4.0,10.0,0.5), mg1_avg_wait_time(4.0,10.0,0.5));
    PriorityQueue pq;
    pq_init(&pq,3.0,2.0,10.0);
    printf("Priority Q: rho=%.4f W1=%.6f W2=%.6f\n",
           pq_rho_total(&pq),pq_avg_wait_type1(&pq),pq_avg_wait_type2(&pq));

    /* 8. PID Advanced */
    printf("\n--- 8. PID Advanced ---\n");
    double Kpz,Kiz,Kdz;
    ziegler_nichols_pid(2.0,3.0,&Kpz,&Kiz,&Kdz);
    printf("Z-N PID(Ku=2,Tu=3): Kp=%.4f Ki=%.4f Kd=%.4f\n",Kpz,Kiz,Kdz);
    AntiWindupPID aw;
    aw_pid_init(&aw,1.0,0.5,0.1,0.01,-1.0,1.0,0.1);
    double u1=aw_pid_update(&aw,1.0,0.0);
    double u2=aw_pid_update(&aw,1.0,2.0);
    printf("Anti-windup: u1=%.4f u2=%.4f\n",u1,u2);
    CascadePID cp;
    cascade_pid_init(&cp,1.0,0.1,0.0,5.0,0.0,0.1,0.01);
    double cpo=cascade_pid_update(&cp,10.0,0.0,0.0);
    printf("Cascade: outer=%.4f inner=%.4f u=%.4f\n",cp.outer_output,cp.inner_output,cpo);

    /* 9. System Dynamics */
    printf("\n--- 9. System Dynamics ---\n");
    PipelineDelay pd;
    pd_init(&pd,3,6.0,0.1);
    double pdo=0;
    for(int i=0;i<30;i++) pdo=pd_update(&pd,1.0);
    printf("Pipeline delay: outflow=%.6f\n",pdo);
    pd_free(&pd);
    SIRModel sir;
    sir_init(&sir,1000.0,1.0,0.3,0.1);
    for(int i=0;i<100;i++) sir_step(&sir,0.1);
    double S,I,R;
    sir_get_state(&sir,&S,&I,&R);
    printf("SIR: S=%.0f I=%.0f R=%.0f\n",S,I,R);
    AgingChain* ac = ac_create(5,80.0);
    ac->cohorts[0]=200;ac->cohorts[1]=180;ac->cohorts[2]=160;ac->cohorts[3]=140;ac->cohorts[4]=120;
    ac_update(ac,1.0,0.02,ac_total_population(ac));
    printf("Aging chain total: %.0f\n", ac_total_population(ac));
    ac_free(ac);

    /* 10. Robust PID Tuning */
    printf("\n--- 10. Robust PID ---\n");
    double** Ar=mat_create(2,2); double** Br=mat_create(2,1); double** Cr=mat_create(1,2);
    Ar[0][1]=1; Ar[1][0]=-2; Ar[1][1]=-0.5; Br[1][0]=1; Cr[0][0]=1;
    RobustPIDTuning tune = robust_pid_tune(Ar,Br,Cr,2,0.01,100.0);
    printf("Robust PID: Kp=%.4f Ki=%.4f Kd=%.4f margin=%.4f\n",
           tune.Kp,tune.Ki,tune.Kd,tune.robustness_margin);
    mat_free(Ar,2);mat_free(Br,2);mat_free(Cr,1);

    /* 11. Manufacturing System */
    printf("\n--- 11. Manufacturing ---\n");
    double pt[]={1.0,0.5};
    ManufacturingSystem* mfg = mfg_create(2,1);
    mfg_run(mfg,50.0,1.5,pt);
    mfg_print_stats(mfg);
    mfg_free(mfg);

    printf("\n=== All modules exercised successfully ===\n");
    return 0;
}
