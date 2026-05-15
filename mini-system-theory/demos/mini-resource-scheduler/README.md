# mini-resource-scheduler: Queue-Based Resource Management

Resource scheduler demo applying queueing theory and rate limiting to server
farm capacity planning.

## Overview

Models a server farm (M/M/c queue) handling incoming requests with rate
limiting (token bucket) and capacity planning (Little's Law). Demonstrates
how queueing theory informs operational decisions about server provisioning
and overload protection.

## Concepts Demonstrated

### 1. M/M/c Queue Model

The M/M/c queue is the workhorse model for server farms:

- **M** (Markovian/Poisson) arrivals: requests arrive at rate λ
- **M** (Markovian/Exponential) service: each server processes at rate μ
- **c** servers: requests wait in FIFO queue, served by first available server

**Key Metrics:**

```
Traffic intensity:    ρ = λ / (c * μ)
Probability of idle:  P0 = [Σ(n=0 to c-1) (cρ)^n/n! + (cρ)^c/(c!(1-ρ))]^(-1)
Probability of queue: Pq = P0 * (cρ)^c / (c! * (1-ρ))
Avg queue length:     Lq = Pq * ρ / (1-ρ)
Avg system length:    L  = Lq + cρ
Avg wait in queue:    Wq = Lq / λ
Avg time in system:   W  = L / λ
```

```c
MMCQueue *q = queue_mmc_create(c_servers, arrival_rate, service_rate);
MMCMetrics m = queue_mmc_metrics(q);
printf("Utilization: %.2f%%\n", m.utilization * 100);
printf("Avg queue length: %.2f\n", m.Lq);
printf("Avg wait time: %.3f s\n", m.Wq);
printf("P(wait > 1s): %.4f\n", m.prob_wait_exceeds_target);
```

### 2. Token Bucket Rate Limiting

A token bucket limits the request rate accepted by the server farm:

- Bucket holds up to **max_tokens** tokens
- Tokens refill at **rate** tokens/second
- Each request costs 1 token
- If bucket is empty, request is rejected (or queued with backpressure)

Used for:
- API rate limiting
- Traffic shaping
- Protecting downstream services from overload

```
      Token Refill (rate r)
           ↓
    ┌─────────────┐
    │ Token Bucket │  ← max_tokens capacity
    │  (capacity)  │
    └──────┬──────┘
           ↓  (1 token per request)
    ┌─────────────┐
    │  M/M/c Queue │  → Servers → Responses
    └─────────────┘
           ↑
      Incoming Requests (rate λ)
```

```c
TokenBucket *tb = token_bucket_create(max_tokens, refill_rate);
for (int i = 0; i < n_requests; i++) {
    if (token_bucket_consume(tb, 1)) {
        queue_mmc_enqueue(q, request[i]);
    } else {
        reject_request(request[i]); // rate limited
    }
}
```

### 3. Little's Law for Capacity Planning

```
L = λ * W

L = average number of requests in system
λ = arrival rate
W = average response time
```

This fundamental law holds for ANY stable queueing system regardless of
arrival or service distributions. It provides a quick capacity estimate:

- Target: W ≤ 100ms, expected λ = 1000 req/s → need L ≤ 100 concurrent
- If each server handles 10 concurrent, need c ≥ 10 servers

```c
double required_capacity = queue_littles_law(target_lambda, target_latency);
double servers_needed = ceil(required_capacity / capacity_per_server);
printf("Need at least %.0f servers\n", servers_needed);
```

## Simulation Flow

```
1. Configure: servers, arrival rate, service rate, token bucket params
2. Generate Poisson arrivals over simulation time
3. For each arrival:
   a. Check token bucket — allow or rate-limit
   b. If allowed, enqueue in M/M/c
   c. Process completions at current time
4. Compute metrics:
   - Average response time
   - Queue length distribution
   - Rate-limited percentage
   - Server utilization
5. Compare against Little's Law prediction
```

## Expected Output

```
=== Resource Scheduler Demo ===
Servers: 4, Arrival rate: 50 req/s, Service rate: 15 req/s per server
Token bucket: max 100 tokens, refill 50 tokens/s

Simulation: 1000 seconds

--- Queue Metrics ---
Utilization: 83.33%
Avg queue length (Lq): 3.20
Avg system length (L): 6.53
Avg wait time (Wq): 0.064 s
Avg response time (W): 0.131 s
P(wait > 0.1s): 23.45%

--- Token Bucket Metrics ---
Requests allowed: 47,230 / 50,000 (94.46%)
Requests rate-limited: 2,770 (5.54%)

--- Little's Law Verification ---
Predicted L = lambda * W: 50 * 0.131 = 6.55
Actual L: 6.53 (error: 0.31%)

--- Capacity Planning ---
To achieve W < 100ms at lambda = 50 req/s:
  Required capacity L = 5.0
  At 15 req/s per server, need ceil(5.0/0.833) = 6 servers
```

## C Implementation Outline

```c
// Configure M/M/c queue
MMCQueue *q = queue_mmc_create(4, 50.0, 15.0);
TokenBucket *tb = token_bucket_create(100, 50.0);

// Simulation
double t = 0;
int served = 0, rate_limited = 0;
double total_response_time = 0;
double total_system_requests = 0;

while (t < sim_time) {
    double next_arrival = t + random_exponential(1.0 / arrival_rate);
    while (q->completions[0] < next_arrival && q->n_in_system > 0) {
        queue_mmc_complete_next(q);
        served++;
    }
    t = next_arrival;

    if (token_bucket_consume(tb, 1)) {
        queue_mmc_enqueue(q, t);
    } else {
        rate_limited++;
    }
    total_system_requests++;
}

MMCMetrics m = queue_mmc_metrics(q);
```

## Real-World Application

- **Web servers:** Provision Nginx/Apache instances behind a load balancer
- **Database connection pools:** M/M/c/c (limited waiting room)
- **Call centers:** Predict staffing needs for target service level
- **Cloud autoscaling:** Trigger scale-out when utilization exceeds threshold
