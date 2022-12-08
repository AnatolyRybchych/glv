#ifndef MVP_H
#define MVP_H

#include <stdlib.h>
#include <memory.h>
#include <math.h>

void mvp_identity(float mat[4*4]);
void mvp_translate(float mat[4*4], float translation[3]);
void mvp_scale(float mat[4*4], float scale[3]);
void mvp_rotate_x(float mat[4*4], float angle);
void mvp_rotate_y(float mat[4*4], float angle);
void mvp_rotate_z(float mat[4*4], float angle);

void mat_mul(float *result, const float *mat1, const float *mat2, unsigned int n);


#endif //MVP_H
