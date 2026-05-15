#include "matrix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

int main(void) {
    printf("=== 2D Transformation Demo ===\n\n");

    int n_points = 5;
    double points_data[] = {1, 0, 0, 1, 1, 1, 2, 1, -1, 2};
    Matrix* points = mat_from_array(2, n_points, points_data);

    printf("Original points (columns):\n");
    for (int j = 0; j < n_points; j++)
        printf("  p%d = [%6.2f %6.2f]\n", j + 1, points->data[j], points->data[2 + j]);

    double theta = 45.0 * PI / 180.0;
    double rot_data[] = {cos(theta), -sin(theta), sin(theta), cos(theta)};
    Matrix* rotation = mat_from_array(2, 2, rot_data);

    double scale_data[] = {2, 0, 0, 0.5};
    Matrix* scaling = mat_from_array(2, 2, scale_data);

    double shear_data[] = {1, 1, 0, 1};
    Matrix* shear = mat_from_array(2, 2, shear_data);

    printf("\n--- Rotation by 45 degrees ---\n");
    Matrix* rotated = mat_mul(rotation, points);
    for (int j = 0; j < n_points; j++)
        printf("  p%d: [%6.2f %6.2f] -> [%6.2f %6.2f]\n",
               j + 1, points->data[j], points->data[2 + j],
               rotated->data[j], rotated->data[2 + j]);

    printf("\n--- Scaling (2x in x, 0.5x in y) ---\n");
    Matrix* scaled = mat_mul(scaling, points);
    for (int j = 0; j < n_points; j++)
        printf("  p%d: [%6.2f %6.2f] -> [%6.2f %6.2f]\n",
               j + 1, points->data[j], points->data[2 + j],
               scaled->data[j], scaled->data[2 + j]);

    printf("\n--- Shear ---\n");
    Matrix* sheared = mat_mul(shear, points);
    for (int j = 0; j < n_points; j++)
        printf("  p%d: [%6.2f %6.2f] -> [%6.2f %6.2f]\n",
               j + 1, points->data[j], points->data[2 + j],
               sheared->data[j], sheared->data[2 + j]);

    printf("\n--- Combined: Rotate then Scale ---\n");
    Matrix* combined = mat_mul(scaling, rotation);
    Matrix* transformed = mat_mul(combined, points);
    printf("Combined matrix (scale * rotation):\n");
    mat_print(combined);
    for (int j = 0; j < n_points; j++)
        printf("  p%d: [%6.2f %6.2f] -> [%6.2f %6.2f]\n",
               j + 1, points->data[j], points->data[2 + j],
               transformed->data[j], transformed->data[2 + j]);

    mat_free(points);
    mat_free(rotation);
    mat_free(scaling);
    mat_free(shear);
    mat_free(rotated);
    mat_free(scaled);
    mat_free(sheared);
    mat_free(combined);
    mat_free(transformed);

    return 0;
}
