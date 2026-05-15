#ifndef ARCHITECTURE_MODEL_H
#define ARCHITECTURE_MODEL_H

#include <stdbool.h>

typedef struct {
    double cost;
    double performance;
    double reliability;
    double weight;
} DesignPoint;

typedef struct {
    DesignPoint* points;
    int n;
} TradeSpace;

typedef struct {
    const char* id;
    double target;
    double actual;
    bool passed;
} Requirement;

typedef struct {
    Requirement* reqs;
    int n;
} RequirementsModel;

void tradespace_add(TradeSpace* ts, double cost, double perf, double reliability, double weight);
int tradespace_pareto_frontier(TradeSpace* ts, int* frontier_indices);
void tradespace_print(TradeSpace* ts);

void reqmodel_add(RequirementsModel* rm, const char* id, double target);
void reqmodel_verify(RequirementsModel* rm, const char* id, double actual);
void reqmodel_print(RequirementsModel* rm);

#endif
