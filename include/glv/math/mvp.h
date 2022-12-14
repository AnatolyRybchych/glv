#ifndef __GLV_MVP_H
#define __GLV_MVP_H

#include <stdlib.h>
#include <memory.h>
#include <math.h>

#define mvp_identity __glv_mvp_identity
#define mvp_translate __glv_mvp_translate
#define mvp_scale __glv_mvp_scale
#define mvp_rotate_x __glv_mvp_rotate_x
#define mvp_rotate_y __glv_mvp_rotate_y
#define mvp_rotate_z __glv_mvp_rotate_z
#define mat_mul __glv_mat_mul


void mvp_identity(float mat[4*4]);
void mvp_translate(float mat[4*4], float translation[3]);
void mvp_scale(float mat[4*4], float scale[3]);
void mvp_rotate_x(float mat[4*4], float angle);
void mvp_rotate_y(float mat[4*4], float angle);
void mvp_rotate_z(float mat[4*4], float angle);

void mat_mul(float *result, const float *mat1, const float *mat2, unsigned int n);


#endif //__GLV_MVP_H
