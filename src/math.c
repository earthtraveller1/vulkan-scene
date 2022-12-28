#include <math.h>

#include "math.h"

#define MULT_ROW_COL(a, b, x, y)                                               \
    ((a[x][0] * b[0][y]) + (a[x][1] * b[1][y]) + (a[x][2] * b[2][y]) +         \
     (a[x][3] * b[3][y]))

struct matrix_4 identity_matrix()
{
    struct matrix_4 result;

    result.mat[0][0] = 1;
    result.mat[0][1] = 0;
    result.mat[0][2] = 0;
    result.mat[0][3] = 0;

    result.mat[1][0] = 0;
    result.mat[1][1] = 1;
    result.mat[1][2] = 0;
    result.mat[1][3] = 0;

    result.mat[2][0] = 0;
    result.mat[2][1] = 0;
    result.mat[2][2] = 1;
    result.mat[2][3] = 0;

    result.mat[3][0] = 0;
    result.mat[3][1] = 0;
    result.mat[3][2] = 0;
    result.mat[3][3] = 1;

    return result;
}

struct matrix_4 multiply_matrices(const struct matrix_4* a,
                                  const struct matrix_4* b)
{
    struct matrix_4 result;

    result.mat[0][0] = MULT_ROW_COL(a->mat, b->mat, 0, 0);
    result.mat[0][1] = MULT_ROW_COL(a->mat, b->mat, 0, 1);
    result.mat[0][2] = MULT_ROW_COL(a->mat, b->mat, 0, 2);
    result.mat[0][3] = MULT_ROW_COL(a->mat, b->mat, 0, 3);

    result.mat[1][0] = MULT_ROW_COL(a->mat, b->mat, 1, 0);
    result.mat[1][1] = MULT_ROW_COL(a->mat, b->mat, 1, 1);
    result.mat[1][2] = MULT_ROW_COL(a->mat, b->mat, 1, 2);
    result.mat[1][3] = MULT_ROW_COL(a->mat, b->mat, 1, 3);

    result.mat[2][0] = MULT_ROW_COL(a->mat, b->mat, 2, 0);
    result.mat[2][1] = MULT_ROW_COL(a->mat, b->mat, 2, 1);
    result.mat[2][2] = MULT_ROW_COL(a->mat, b->mat, 2, 2);
    result.mat[2][3] = MULT_ROW_COL(a->mat, b->mat, 2, 3);

    result.mat[3][0] = MULT_ROW_COL(a->mat, b->mat, 3, 0);
    result.mat[3][1] = MULT_ROW_COL(a->mat, b->mat, 3, 1);
    result.mat[3][2] = MULT_ROW_COL(a->mat, b->mat, 3, 2);
    result.mat[3][3] = MULT_ROW_COL(a->mat, b->mat, 3, 3);

    return result;
}

struct matrix_4 perspective_projection_matrix(float left, float right,
                                              float top, float bottom,
                                              float far, float near)
{
    struct matrix_4 result;

    result.mat[0][0] = (2 * near) / (right - left);
    result.mat[0][1] = 0;
    result.mat[0][2] = (right + left) / (right - left);
    result.mat[0][3] = 0;

    result.mat[1][0] = 0;
    result.mat[1][1] = (2 * near) / (top - bottom);
    result.mat[1][2] = (top + bottom) / (top - bottom);
    result.mat[1][3] = 0;

    result.mat[2][0] = 0;
    result.mat[2][1] = 0;
    result.mat[2][2] = -((far + near) / (far - near));
    result.mat[2][3] = -((2 * far * near) / (far - near));

    result.mat[3][0] = 0;
    result.mat[3][1] = 0;
    result.mat[3][2] = -1;
    result.mat[3][3] = 0;

    return result;
}