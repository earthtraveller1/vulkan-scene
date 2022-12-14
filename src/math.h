#ifndef INCLUDED_MATH_H
#define INCLUDED_MATH_H

/* A set of mathematical functionalities. Mainly just vector and matrix math. */

/* PI */
#define PI                                                                     \
    3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066

struct vector_2
{
    float x;
    float y;
};

struct vector_3
{
    float x;
    float y;
    float z;
};

/* Represents a 4 by 4 matrix. */
struct matrix_4
{
    float mat[4][4];
};

/* Returns an identity matrix. */
struct matrix_4 identity_matrix();

/* Multiplies two matrices and returns the result. */
struct matrix_4 multiply_matrices(const struct matrix_4* a,
                                  const struct matrix_4* b);

/* A perspective project matrix. */
struct matrix_4 perspective_projection_matrix(float aspect_ratio, float fov,
                                              float far, float near);

/* Basically, translation. */
void translate_matrix(struct matrix_4* matrix, float x, float y, float z);

/* Converts degrees to radians. */
float deg2rad(float deg);

#endif