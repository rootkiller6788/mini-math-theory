#include "control_loop.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

void bangbang_init(BangBang* bb, double on_t, double off_t) {
    bb->on_threshold = on_t;
    bb->off_threshold = off_t;
    bb->state = false;
}

bool bangbang_update(BangBang* bb, double measurement) {
    if (!bb->state && measurement > bb->on_threshold)
        bb->state = true;
    else if (bb->state && measurement < bb->off_threshold)
        bb->state = false;
    return bb->state;
}

void leadlag_init(LeadLag* ll, double a, double b) {
    ll->a = a;
    ll->b = b;
    ll->prev_in = 0;
    ll->prev_out = 0;
}

double leadlag_update(LeadLag* ll, double input) {
    double output = ll->a * input + ll->b * ll->prev_in - ll->b * ll->prev_out;
    ll->prev_in = input;
    ll->prev_out = output;
    return output;
}

void ratelimit_init(RateLimiter* rl, double max_rate, double dt) {
    rl->max_rate = max_rate;
    rl->dt = dt;
    rl->prev_out = 0;
}

double ratelimit_update(RateLimiter* rl, double input) {
    double max_change = rl->max_rate * rl->dt;
    double delta = input - rl->prev_out;
    if (delta > max_change)
        rl->prev_out += max_change;
    else if (delta < -max_change)
        rl->prev_out -= max_change;
    else
        rl->prev_out = input;
    return rl->prev_out;
}

double deadzone_apply(double input, double threshold) {
    if (fabs(input) < threshold) return 0;
    if (input > 0) return input - threshold;
    return input + threshold;
}

double saturation_apply(double input, double lo, double hi) {
    if (input > hi) return hi;
    if (input < lo) return lo;
    return input;
}
