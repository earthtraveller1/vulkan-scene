#include <math.h>

#include "math.h"

struct matrix_4 perspective_projection_matrix(float left, float right, float top, float bottom, float far, float near)
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