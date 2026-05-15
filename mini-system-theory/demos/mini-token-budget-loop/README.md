# mini-token-budget-loop: Adaptive Rate Limiting with Feedback

Token bucket demo with a feedback control loop for adaptive API rate limiting
and burst handling.

## Overview

Implements a token bucket rate limiter with an adaptive feedback loop that
dynamically adjusts the token refill rate based on downstream service health.
Includes burst handling strategies for handling traffic spikes.

## Concepts Demonstrated

### 1. Token Bucket Algorithm

The token bucket polices an average rate while allowing short bursts:

```
Tokens accumulate at a constant rate R (tokens/second)
Bucket capacity B (maximum tokens)
Each request consumes 1 token (or T tokens for weighted requests)

Rules:
- If tokens >= 1 at request time → request is accepted, token consumed
- If tokens < 1 → request is rejected or queued

Burst capacity: B tokens allow B consecutive requests without waiting
```

```c
TokenBucket *tb = token_bucket_create(capacity, refill_rate);

// Per-request check
if (token_bucket_consume(tb, tokens_required)) {
    // Request allowed
    forward_request(req);
} else {
    // Rate limited
    reject_request(req, 429);
}

// Token bucket state query
BucketState s = token_bucket_state(tb);
printf("Tokens: %.1f / %.0f\n", s.current_tokens, s.max_tokens);
printf("Fill rate: %.1f tok/s\n", s.refill_rate);
```

### 2. Feedback Loop for Adaptive Rate Control

The token refill rate is dynamically adjusted based on feedback from the
downstream service:

```
                    ┌──────────────────────────────────┐
                    │        Feedback Controller        │
                    │  (PI control of refill rate)      │
                    │                                   │
  target_latency ──→│  error = target - actual_latency  │
                    │  new_rate = old_rate + Kp*e + Ki*∫e │
                    └──────────────┬───────────────────┘
                                   │ refill_rate
                                   ↓
  Requests ──→ ┌──────────┐  ┌───────────┐  ┌───────────┐
               │  Token    │→│  Request  │→│ Downstream │→ Response
               │  Bucket   │  │  Queue    │  │  Service   │
               └──────────┘  └───────────┘  └─────┬─────┘
                                                   │
                          ←── latency feedback ────┘
```

**Control Law:**

```
e(t) = target_latency - measured_latency(t)
refill_rate(t) = refill_rate(t-1) + Kp * e(t) + Ki * integral_of_e(t)

With anti-windup: clamp refill_rate ∈ [min_rate, max_rate]
```

```c
PI_Controller *ctrl = pi_create(Kp, Ki, dt, min_rate, max_rate);

for (each time_step) {
    // Measure downstream latency
    double latency = get_avg_latency(window);

    // Compute new refill rate
    double new_rate = pi_update(ctrl, target_latency, latency);

    // Update token bucket
    token_bucket_set_rate(tb, new_rate);
}
```

### 3. Burst Handling

Three strategies for handling traffic bursts:

#### Strategy A: Immediate Rejection

Simplest — reject if not enough tokens.

```c
if (!token_bucket_consume(tb, 1)) {
    return HTTP_TOO_MANY_REQUESTS;
}
```

#### Strategy B: Burst Queue with Timeout

Queue requests that exceed token capacity, serve when tokens become available.

```c
if (!token_bucket_consume(tb, 1)) {
    if (burst_queue_len < max_queue) {
        burst_queue_enqueue(req, timeout_ms);
    } else {
        reject_request(req);
    }
}
// In a separate loop:
while (burst_queue_not_empty() && token_bucket_consume(tb, 1)) {
    forward_request(burst_queue_dequeue());
}
```

#### Strategy C: Priority-Based Burst

Critical requests always get tokens; best-effort requests share remaining.

```c
if (req->priority == PRIORITY_CRITICAL) {
    token_bucket_consume(tb, 1); // always consume (can go negative)
    forward_request(req);
} else {
    if (token_bucket_consume(tb, 1)) {
        forward_request(req);
    } else {
        reject_request(req);
    }
}
```

## Adaptive Algorithm

```
1. Initialize token bucket with capacity B, refill rate R0
2. Every control interval T:
   a. Measure average downstream latency over last T seconds
   b. Compute latency error: e = target_latency - actual_latency
   c. Update refill rate via PI controller
   d. Clamp refill rate to [Rmin, Rmax]
3. On each request:
   a. If critical: bypass token bucket
   b. Else: check tokens, accept or reject
   c. Track accept/reject ratio
```

## Expected Output

```
=== Token Budget Control Loop ===
Initial rate: 100 tok/s, Capacity: 200 tokens
Target latency: 50 ms
PI gains: Kp=0.5, Ki=0.1

--- Phase 1: Steady State ---
Time    Rate (tok/s)  Latency (ms)   Accept %   Queue Len
0.0     100.0         45.2           100.0%      0
2.0     100.0         48.7           100.0%      0
4.0     100.0         49.1           100.0%      1
6.0     100.0         50.3           100.0%      2

--- Phase 2: Traffic Spike (3x normal) ---
8.0     100.0         78.5           89.2%       45
10.0    89.3          62.3           82.1%       38
12.0    78.1          54.1           81.5%       22
14.0    74.2          49.8           83.0%       12
16.0    72.5          50.1           84.2%       8
18.0    71.8          49.5           84.8%       6

--- Phase 3: Service Degradation ---
20.0    71.8          72.4           78.3%       18
22.0    60.2          58.7           72.1%       15
24.0    53.1          50.8           73.5%       10
26.0    52.4          49.3           74.2%       8

--- Summary ---
Final adapted rate: ~52 tok/s (from initial 100 tok/s)
Latency converged to target within 6s of disturbance
Total requests: 2,340, Accepted: 1,783 (76.2%)
```

## C Implementation Outline

```c
int main(void) {
    // Token bucket: 200 capacity, 100 tok/s initial
    TokenBucket *tb = token_bucket_create(200.0, 100.0);

    // PI controller for rate adaptation
    PI_Controller *pi = pi_create(0.5, 0.1, 0.1, 10.0, 200.0);

    // Burst queue
    Queue *burst_q = queue_create(500);

    double time = 0;
    double target_latency = 0.050; // 50 ms

    while (time < sim_duration) {
        // Generate requests (with traffic spike at t=8)
        int n_requests = (time > 8.0 && time < 20.0) ? 300 : 100;
        for (int i = 0; i < n_requests; i++) {
            Request req = generate_request(time);

            // Try token bucket
            if (token_bucket_consume(tb, 1)) {
                double latency = simulate_downstream(req);
                record_latency(latency);
            } else if (queue_len(burst_q) < 500) {
                queue_enqueue(burst_q, req);
            } else {
                rejected++;
            }
            total++;
        }

        // Drain burst queue
        while (!queue_empty(burst_q) && token_bucket_consume(tb, 1)) {
            Request req = queue_dequeue(burst_q);
            double latency = simulate_downstream(req);
            record_latency(latency);
        }

        // Feedback control every control_interval
        if (fmod(time, control_interval) < dt) {
            double avg_latency = get_window_avg();
            double new_rate = pi_update(pi, target_latency, avg_latency);
            token_bucket_set_rate(tb, new_rate);
        }

        time += dt;
    }
}
```

## Real-World Application

- **API gateways:** Kong, Nginx rate limiting with adaptive thresholds
- **CDN edge:** Protect origin servers from traffic spikes
- **Microservices:** Circuit breaker with adaptive recovery rate
- **Database connection pooling:** Adaptive pool size based on query latency
