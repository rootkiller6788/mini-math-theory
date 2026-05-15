#include "architecture_model.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846

void tradespace_add(TradeSpace* ts, double cost, double perf,
                    double reliability, double weight) {
    ts->n++;
    ts->points = (DesignPoint*)realloc(ts->points, ts->n * sizeof(DesignPoint));
    ts->points[ts->n - 1].cost = cost;
    ts->points[ts->n - 1].performance = perf;
    ts->points[ts->n - 1].reliability = reliability;
    ts->points[ts->n - 1].weight = weight;
}

static int dominates(DesignPoint* a, DesignPoint* b) {
    int better = 0;
    if (a->cost < b->cost) better |= 1;
    else if (a->cost > b->cost) return 0;
    if (a->performance > b->performance) better |= 2;
    else if (a->performance < b->performance) return 0;
    if (a->reliability > b->reliability) better |= 4;
    else if (a->reliability < b->reliability) return 0;
    if (a->weight < b->weight) better |= 8;
    else if (a->weight > b->weight) return 0;
    return better != 0;
}

int tradespace_pareto_frontier(TradeSpace* ts, int* frontier_indices) {
    int* dominated = (int*)calloc(ts->n, sizeof(int));
    for (int i = 0; i < ts->n; i++) {
        for (int j = 0; j < ts->n; j++) {
            if (i == j) continue;
            if (dominates(&ts->points[j], &ts->points[i])) {
                dominated[i] = 1;
                break;
            }
        }
    }
    int count = 0;
    for (int i = 0; i < ts->n; i++) {
        if (!dominated[i]) {
            frontier_indices[count++] = i;
        }
    }
    free(dominated);
    return count;
}

void tradespace_print(TradeSpace* ts) {
    printf("Trade Space (%d designs):\n", ts->n);
    printf("  %-6s %10s %12s %12s %10s\n", "Index", "Cost", "Perf", "Reliability", "Weight");
    for (int i = 0; i < ts->n; i++) {
        printf("  %-6d %10.3f %12.3f %12.3f %10.3f\n",
               i, ts->points[i].cost, ts->points[i].performance,
               ts->points[i].reliability, ts->points[i].weight);
    }
}

void reqmodel_add(RequirementsModel* rm, const char* id, double target) {
    rm->n++;
    rm->reqs = (Requirement*)realloc(rm->reqs, rm->n * sizeof(Requirement));
    rm->reqs[rm->n - 1].id = id;
    rm->reqs[rm->n - 1].target = target;
    rm->reqs[rm->n - 1].actual = 0;
    rm->reqs[rm->n - 1].passed = false;
}

void reqmodel_verify(RequirementsModel* rm, const char* id, double actual) {
    for (int i = 0; i < rm->n; i++) {
        if (strcmp(rm->reqs[i].id, id) == 0) {
            rm->reqs[i].actual = actual;
            rm->reqs[i].passed = (actual >= rm->reqs[i].target);
            return;
        }
    }
}

void reqmodel_print(RequirementsModel* rm) {
    printf("Requirements Model (%d requirements):\n", rm->n);
    printf("  %-20s %10s %10s %s\n", "ID", "Target", "Actual", "Status");
    for (int i = 0; i < rm->n; i++) {
        printf("  %-20s %10.3f %10.3f %s\n",
               rm->reqs[i].id, rm->reqs[i].target, rm->reqs[i].actual,
               rm->reqs[i].passed ? "PASS" : "FAIL");
    }
}
