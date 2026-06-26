#ifndef DISCRETE_EVENT_H
#define DISCRETE_EVENT_H

#include <stdbool.h>

/* L1: Discrete-event simulation event */
typedef struct {
    double time;
    int type;
    int entity_id;
    void* data;
} DESEvent;

/* L1: Priority event queue for discrete-event simulation */
typedef struct {
    DESEvent* events;
    int size;
    int capacity;
} EventQueue;

/* L2: Core discrete-event simulation engine */
typedef struct {
    EventQueue* queue;
    double current_time;
    double end_time;
    int n_events_processed;
} DESEngine;

/* Event queue operations */
EventQueue* eq_create(int capacity);
void eq_free(EventQueue* q);
void eq_push(EventQueue* q, DESEvent ev);
DESEvent eq_pop(EventQueue* q);
bool eq_empty(EventQueue* q);
double eq_peek_time(EventQueue* q);

/* L3: DES engine — event scheduling and processing loop
 * Reference: Banks, Carson, Nelson & Nicol (2010) Discrete-Event System Simulation
 */
DESEngine* des_create(double end_time);
void des_free(DESEngine* engine);
void des_schedule(DESEngine* engine, double time, int type, int entity_id, void* data);
void des_run(DESEngine* engine, void (*handler)(DESEngine*, DESEvent*));

/* L1: Finite State Machine for discrete-event control */
typedef struct {
    int n_states;
    int n_events;
    int current_state;
    int** transition_table;  /* [state][event] -> next_state */
    void (**actions)(void*); /* flat array: actions[state * n_events + event] */
} FSM;

/* L4: FSM operations — deterministic automaton
 * Reference: Hopcroft, Motwani & Ullman (2006) Automata Theory
 */
FSM* fsm_create(int n_states, int n_events);
void fsm_free(FSM* fsm);
void fsm_set_transition(FSM* fsm, int state, int event, int next_state);
void fsm_add_action(FSM* fsm, int state, int event, void (*action)(void*));
int fsm_step(FSM* fsm, int event, void* context);

/* L1: Petri Net for concurrent system modeling */
typedef struct {
    int* marking;       /* Current number of tokens in each place */
    int** pre;          /* Pre-incidence matrix [transition][place] */
    int** post;         /* Post-incidence matrix [transition][place] */
    int n_places;
    int n_transitions;
} PetriNet;

/* L5: Petri net firing rules
 * A transition is enabled if all input places have enough tokens.
 * Firing: removes tokens from input places, adds to output places.
 * Reference: Petri, C.A. (1962) "Kommunikation mit Automaten"
 */
PetriNet* pn_create(int n_places, int n_transitions);
void pn_free(PetriNet* pn);
void pn_set_pre(PetriNet* pn, int t, int p, int weight);
void pn_set_post(PetriNet* pn, int t, int p, int weight);
void pn_set_marking(PetriNet* pn, int p, int tokens);
bool pn_is_enabled(PetriNet* pn, int transition);
void pn_fire(PetriNet* pn, int transition);
int* pn_enabled_transitions(PetriNet* pn, int* count);
void pn_print(PetriNet* pn);

/* L6: Classic DES examples */

/* Traffic light controller FSM — 4 states, 2 timers */
typedef struct {
    FSM* fsm;
    double timer;
    double green_time;
    double yellow_time;
    double red_time;
    int east_state;   /* 0=G, 1=Y, 2=R */
    int north_state;
} TrafficLight;

TrafficLight* tl_create(double green, double yellow, double red);
void tl_update(TrafficLight* tl, double dt);
void tl_print(TrafficLight* tl);
void tl_free(TrafficLight* tl);

/* L7: Manufacturing system simulation — machine + buffer + jobs */
typedef struct {
    double process_time;
    double busy_until;
    int jobs_processed;
    double total_busy_time;
} Machine;

typedef struct {
    int* jobs;
    int head;
    int tail;
    int size;
    int capacity;
} Buffer;

typedef struct {
    Machine* machines;
    Buffer** buffers;
    int n_machines;
    int n_buffers;
    DESEngine* engine;
    double total_completion_time;
    int total_jobs;
} ManufacturingSystem;

ManufacturingSystem* mfg_create(int n_machines, int n_buffers);
void mfg_free(ManufacturingSystem* mfg);
void mfg_run(ManufacturingSystem* mfg, double duration, 
             double arrival_rate, double* process_times);
void mfg_print_stats(ManufacturingSystem* mfg);

/* Simple circular buffer queue */
Buffer* buffer_create(int capacity);
void buffer_free(Buffer* b);
bool buffer_push(Buffer* b, int item);
int buffer_pop(Buffer* b);
bool buffer_empty(Buffer* b);
bool buffer_full(Buffer* b);
int buffer_count(Buffer* b);

#endif
