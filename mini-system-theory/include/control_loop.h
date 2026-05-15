#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <stdbool.h>

typedef struct {
    double on_threshold, off_threshold;
    bool state;
} BangBang;

typedef struct {
    double a, b, prev_in, prev_out;
} LeadLag;

typedef struct {
    double max_rate;
    double prev_out;
    double dt;
} RateLimiter;

void bangbang_init(BangBang* bb, double on_t, double off_t);
bool bangbang_update(BangBang* bb, double measurement);

void leadlag_init(LeadLag* ll, double a, double b);
double leadlag_update(LeadLag* ll, double input);

void ratelimit_init(RateLimiter* rl, double max_rate, double dt);
double ratelimit_update(RateLimiter* rl, double input);

double deadzone_apply(double input, double threshold);
double saturation_apply(double input, double lo, double hi);

#endif
