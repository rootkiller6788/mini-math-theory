#include "relation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void relation_init(Relation* r, int size) {
    r->size = size;
    memset(r->matrix, 0, sizeof(r->matrix));
}

void relation_add_pair(Relation* r, int a, int b) {
    if (a >= 0 && a < r->size && b >= 0 && b < r->size)
        r->matrix[a][b] = 1;
}

bool relation_has(Relation* r, int a, int b) {
    return a >= 0 && a < r->size && b >= 0 && b < r->size && r->matrix[a][b];
}

void relation_print_matrix(Relation* r) {
    printf("  ");
    for (int j = 0; j < r->size; j++) printf("%d ", j);
    printf("\n");
    for (int i = 0; i < r->size; i++) {
        printf("%d ", i);
        for (int j = 0; j < r->size; j++)
            printf("%d ", r->matrix[i][j]);
        printf("\n");
    }
}

void relation_print_pairs(Relation* r) {
    printf("{");
    int first = 1;
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j]) {
                if (!first) printf(", ");
                printf("(%d,%d)", i, j);
                first = 0;
            }
    printf("}\n");
}

bool relation_is_reflexive(Relation* r) {
    for (int i = 0; i < r->size; i++)
        if (!r->matrix[i][i]) return false;
    return true;
}

bool relation_is_symmetric(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j] && !r->matrix[j][i]) return false;
    return true;
}

bool relation_is_transitive(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j])
                for (int k = 0; k < r->size; k++)
                    if (r->matrix[j][k] && !r->matrix[i][k])
                        return false;
    return true;
}

bool relation_is_antisymmetric(Relation* r) {
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (i != j && r->matrix[i][j] && r->matrix[j][i])
                return false;
    return true;
}

bool relation_is_equivalence(Relation* r) {
    return relation_is_reflexive(r) && relation_is_symmetric(r) && relation_is_transitive(r);
}

bool relation_is_partial_order(Relation* r) {
    return relation_is_reflexive(r) && relation_is_antisymmetric(r) && relation_is_transitive(r);
}

Relation relation_reflexive_closure(Relation* r) {
    Relation result = *r;
    for (int i = 0; i < r->size; i++)
        result.matrix[i][i] = 1;
    return result;
}

Relation relation_symmetric_closure(Relation* r) {
    Relation result = *r;
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (r->matrix[i][j])
                result.matrix[j][i] = 1;
    return result;
}

Relation relation_transitive_closure(Relation* r) {
    Relation result = *r;

    for (int k = 0; k < r->size; k++)
        for (int i = 0; i < r->size; i++)
            for (int j = 0; j < r->size; j++)
                if (result.matrix[i][k] && result.matrix[k][j])
                    result.matrix[i][j] = 1;
    return result;
}

EquivalencePartition equivalence_classes(Relation* r) {
    EquivalencePartition ep;
    ep.nclasses = 0;
    bool visited[MAX_REL_SIZE] = {false};

    for (int i = 0; i < r->size; i++) {
        if (visited[i]) continue;
        ep.class_sizes[ep.nclasses] = 0;
        for (int j = 0; j < r->size; j++) {
            if (!visited[j] && r->matrix[i][j]) {
                visited[j] = true;
                ep.classes[ep.nclasses][ep.class_sizes[ep.nclasses]++] = j;
            }
        }
        ep.nclasses++;
    }
    return ep;
}

void hasse_diagram_print(Relation* r) {
    if (!relation_is_partial_order(r)) {
        printf("不是偏序关系，无法绘制 Hasse 图\n");
        return;
    }

    int cover[MAX_REL_SIZE][MAX_REL_SIZE] = {{0}};
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (i != j && r->matrix[i][j]) {
                cover[i][j] = 1;
                for (int k = 0; k < r->size; k++)
                    if (k != i && k != j && r->matrix[i][k] && r->matrix[k][j]) {
                        cover[i][j] = 0;
                        break;
                    }
            }

    printf("Hasse 图的覆盖边:\n");
    for (int i = 0; i < r->size; i++)
        for (int j = 0; j < r->size; j++)
            if (cover[i][j])
                printf("  %d → %d\n", i, j);
}

int* topological_sort(Relation* r) {
    int indegree[MAX_REL_SIZE] = {0};
    int* order = malloc(r->size * sizeof(int));
    int oidx = 0;

    for (int j = 0; j < r->size; j++)
        for (int i = 0; i < r->size; i++)
            if (i != j && r->matrix[i][j])
                indegree[j]++;

    int queue[MAX_REL_SIZE];
    int qhead = 0, qtail = 0;
    for (int i = 0; i < r->size; i++)
        if (indegree[i] == 0)
            queue[qtail++] = i;

    while (qhead < qtail) {
        int u = queue[qhead++];
        order[oidx++] = u;
        for (int v = 0; v < r->size; v++)
            if (u != v && r->matrix[u][v]) {
                indegree[v]--;
                if (indegree[v] == 0)
                    queue[qtail++] = v;
            }
    }
    return order;
}
