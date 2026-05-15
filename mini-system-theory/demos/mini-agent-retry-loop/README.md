# mini-agent-retry-loop: Exponential Backoff with Adaptive Feedback

Agent retry loop demo implementing exponential backoff with jitter,
feedback-based adaptive retry, and queue-based request management.

## Overview

Models a retry mechanism for unreliable service calls (e.g., HTTP requests,
message delivery) using exponential backoff, jitter for avoiding thundering
herd, and a feedback controller that adapts retry strategy based on observed
success rates and downstream health.

## Concepts Demonstrated

### 1. Exponential Backoff with Jitter

When a request fails, the client waits before retrying. The wait time doubles
after each failure (exponential backoff), with random jitter to prevent
synchronized retries from many clients.

#### Basic Exponential Backoff

```
wait_time = base_delay * 2^(attempt - 1)
capped at max_delay

Attempt 1 fails → wait 1s
Attempt 2 fails → wait 2s
Attempt 3 fails → wait 4s
Attempt 4 fails → wait 8s
...
Attempt N fails → wait min(max_delay, base * 2^(N-1))
```

#### Jitter Strategies

**Full Jitter** (recommended by AWS):
```
wait_time = random(0, base_delay * 2^(attempt - 1))
```

**Equal Jitter:**
```
temp = base_delay * 2^(attempt - 1)
wait_time = temp / 2 + random(0, temp / 2)
```

**Decorrelated Jitter:**
```
wait_time = min(max_delay, random(base_delay, previous_wait * 3))
```

```c
double backoff_full_jitter(int attempt, double base_delay, double max_delay) {
    double cap = base_delay * pow(2.0, attempt - 1);
    if (cap > max_delay) cap = max_delay;
    return ((double)rand() / RAND_MAX) * cap;
}

double backoff_equal_jitter(int attempt, double base_delay, double max_delay) {
    double temp = base_delay * pow(2.0, attempt - 1);
    if (temp > max_delay) temp = max_delay;
    return temp / 2.0 + ((double)rand() / RAND_MAX) * (temp / 2.0);
}
```

### 2. Feedback-Based Adaptive Retry

Instead of fixed backoff parameters, the retry strategy adapts based on
observed success rates. A feedback controller adjusts `base_delay` and
`max_attempts` dynamically.

```
                   ┌─────────────────────────┐
                   │    Adaptive Controller    │
                   │  adjust base_delay, max_attempts │
                   └──────────┬──────────────┘
                              │
                    target_success_rate (95%)
                    actual_success_rate (measured)
                              │
                              ↓
  Requests ──→ ┌──────────┐  ┌───────────┐  ┌───────────┐
               │   Retry   │→│ Downstream │→│  Success / │
               │  Engine   │  │  Service   │  │  Failure   │
               └──────────┘  └─────┬─────┘  └─────┬─────┘
                     ↑             │               │
                     └─ backoff ───┘    feedback ──┘
                        params
```

**Control Laws:**

```
If success_rate < target:
    Increase base_delay (reduce load, give downstream time to recover)
    Increase max_attempts (don't give up too fast when degraded)

If success_rate > target:
    Decrease base_delay (faster retries, lower latency)
    Decrease max_attempts (fewer wasted attempts)
```

```c
RetryController *rc = retry_controller_create(
    target_success_rate, // 0.95
    initial_base_delay,  // 1.0 s
    initial_max_attempts // 5
);

for (each time_window) {
    double success_rate = rc->recent_successes / (double)rc->recent_attempts;

    // Update backoff parameters
    retry_controller_adapt(rc, success_rate);

    printf("Window %d: success=%.1f%%, base_delay=%.2fs, max_attempts=%d\n",
           window, success_rate * 100, rc->base_delay, rc->max_attempts);
}
```

### 3. Queue-Based Request Management

Failed requests are placed in retry queues. Multiple retry queues with
different priorities prevent head-of-line blocking.

```
                    ┌──────────────────┐
                    │   Primary Queue   │  (new requests)
                    └────────┬─────────┘
                             │
                             ↓
                    ┌──────────────────┐
                    │   Active Attempt  │ → Success → Done
                    └────────┬─────────┘
                             │ (failure)
                             ↓
                    ┌──────────────────┐
                    │   Retry Queue 1   │  (1st retry attempts)
                    └────────┬─────────┘
                             │
                    ┌──────────────────┐
                    │   Retry Queue 2   │  (2nd retry attempts)
                    └────────┬─────────┘
                             │
                          ...
                    ┌──────────────────┐
                    │   Dead Letter     │  (exhausted retries)
                    └──────────────────┘
```

```c
RetryQueue *rq = retry_queue_create(max_total_size);

// Submit request
RetryRequest *req = retry_request_create(payload, max_attempts);
retry_queue_enqueue(rq, req);

// Process loop
while (!retry_queue_all_done(rq)) {
    RetryRequest *next = retry_queue_next_ready(rq, current_time);
    if (!next) continue;

    Result r = attempt(next);
    if (r.success) {
        retry_queue_complete(rq, next);
    } else if (next->attempt >= next->max_attempts) {
        retry_queue_dead_letter(rq, next); // exhausted
        printf("Request %d abandoned after %d attempts\n",
               next->id, next->attempt);
    } else {
        double delay = backoff_full_jitter(next->attempt + 1,
                                           base_delay, max_delay);
        retry_queue_reschedule(rq, next, current_time + delay);
    }
}
```

## Simulation Flow

```
1. Configure: target success rate, initial backoff params, jitter strategy
2. Generate stream of requests over time
3. Simulate downstream with configurable failure rate
4. For each request:
   a. First attempt through primary queue
   b. On failure: compute backoff, enqueue in retry queue
   c. On retry: attempt again, track attempts
   d. After max_attempts: move to dead letter
5. Every control interval:
   a. Compute recent success rate
   b. Update base_delay via feedback controller
6. Report metrics: success rate, avg attempts, avg latency, dead letters
```

## Expected Output

```
=== Agent Retry Loop Demo ===
Target success rate: 95%
Initial: base_delay=1.0s, max_attempts=5
Jitter: Full jitter
Total requests: 10,000

--- Steady State (downstream 99% reliable) ---
Window   | Success% | Avg Attempts | Avg Latency | Dead Letters | base_delay
   1     |  98.9%   |    1.01      |   45 ms     |      0       |   1.00s
   2     |  99.1%   |    1.01      |   43 ms     |      0       |   0.95s
   3     |  98.8%   |    1.02      |   44 ms     |      0       |   0.97s

--- Degraded Mode (downstream 70% reliable, t=30-60s) ---
   4     |  72.3%   |    3.42      |  850 ms     |     12       |   1.50s
   5     |  74.1%   |    3.21      |  780 ms     |      8       |   1.80s
   6     |  76.8%   |    2.98      |  720 ms     |      5       |   2.10s
   7     |  78.2%   |    2.85      |  680 ms     |      3       |   2.30s

--- Recovery (downstream back to 99%) ---
   8     |  92.1%   |    1.45      |  120 ms     |      0       |   1.80s
   9     |  96.5%   |    1.08      |   58 ms     |      0       |   1.20s
  10     |  98.9%   |    1.01      |   42 ms     |      0       |   0.90s

--- Final Summary ---
Overall success rate: 97.2% (including retries)
Average attempts per request: 1.15
Average latency: 87 ms
Dead letters: 28 / 10,000 (0.28%)
```

## Thundering Herd Problem

Without jitter, many clients retrying simultaneously after a service outage
can overwhelm the recovering service — a "thundering herd."

```
Without jitter:  ~~~~||~~~~||~~~~||  (synchronized bursts)
With full jitter: ~ ~  ~ ~ ~  ~  ~   (spread over time)
```

```c
// Demonstrated by comparing jitter strategies
compare_strategies(1000, 10, base_delay, max_delay);
// Output:
// No jitter:     peak_concurrent = 987 (burst)
// Equal jitter:  peak_concurrent = 234
// Full jitter:   peak_concurrent = 98  (best spread)
```

## C Implementation Outline

```c
typedef struct {
    int id;
    void *payload;
    int attempt;
    int max_attempts;
    double next_retry_time;
} RetryRequest;

typedef struct {
    double base_delay;
    double max_delay;
    double target_success_rate;
    double recent_successes;
    double recent_attempts;
    int max_attempts;
    double Kp; // proportional gain for adaptation
} RetryController;

double compute_backoff(RetryController *rc, int attempt) {
    double cap = fmin(rc->base_delay * pow(2.0, attempt - 1), rc->max_delay);
    return ((double)rand() / RAND_MAX) * cap; // full jitter
}

void retry_controller_adapt(RetryController *rc, double success_rate) {
    double error = rc->target_success_rate - success_rate;
    double delta = rc->Kp * error;

    rc->base_delay += delta;
    rc->base_delay = fmax(0.1, fmin(rc->base_delay, rc->max_delay));

    if (success_rate < rc->target_success_rate - 0.1) {
        rc->max_attempts = fmin(rc->max_attempts + 1, 10);
    } else if (success_rate > rc->target_success_rate + 0.05) {
        rc->max_attempts = fmax(rc->max_attempts - 1, 2);
    }
}

int main(void) {
    RetryController *rc = retry_controller_create(0.95, 1.0, 30.0, 5);
    RetryQueue *rq = retry_queue_create(1000);

    // Simulation loop...
}
```

## Real-World Application

- **AWS SDK retry behavior:** Full jitter backoff for S3, DynamoDB
- **gRPC retry:** Configurable backoff with jitter
- **Message queues:** RabbitMQ/Redis retry with exponential backoff
- **Mobile apps:** Offline retry with adaptive backoff
- **Stripe API client:** Idempotency keys + retry with backoff
