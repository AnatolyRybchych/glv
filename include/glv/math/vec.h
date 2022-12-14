#ifndef __GLV_VEC_H
#define __GLV_VEC_H

#define lerpf __glv_lerpf

#define vec2_scale __glv_vec2_scale
#define vec2_offset __glv_vec2_offset
#define vec2_add __glv_vec2_add
#define vec2_sub __glv_vec2_sub
#define vec2_mul __glv_vec2_mul
#define vec2_div __glv_vec2_div
#define vec2_max __glv_vec2_max
#define vec2_min __glv_vec2_min
#define vec2_lerp __glv_vec2_lerp

#define vec3_scale __glv_vec3_scale
#define vec3_offset __glv_vec3_offset
#define vec3_add __glv_vec3_add
#define vec3_sub __glv_vec3_sub
#define vec3_mul __glv_vec3_mul
#define vec3_div __glv_vec3_div
#define vec3_max __glv_vec3_max
#define vec3_min __glv_vec3_min
#define vec3_lerp __glv_vec3_lerp

#define vec4_scale __glv_vec4_scale
#define vec4_offset __glv_vec4_offset
#define vec4_add __glv_vec4_add
#define vec4_sub __glv_vec4_sub
#define vec4_mul __glv_vec4_mul
#define vec4_div __glv_vec4_div
#define vec4_max __glv_vec4_max
#define vec4_min __glv_vec4_min
#define vec4_lerp __glv_vec4_lerp

float lerpf(float from, float to, float progress);

void vec2_scale(float result[2], const float vec[2], float value);
void vec2_offset(float result[2], const float vec[2], float value);
void vec2_add(float result[2], const float vec1[2], const float vec2[2]);
void vec2_sub(float result[2], const float vec1[2], const float vec2[2]);
void vec2_mul(float result[2], const float vec1[2], const float vec2[2]);
void vec2_div(float result[2], const float vec1[2], const float vec2[2]);
void vec2_max(float result[2], const float vec1[2], const float vec2[2]);
void vec2_min(float result[2], const float vec1[2], const float vec2[2]);
void vec2_lerp(float result[2], const float vec1[2], const float vec2[2], float progress);

void vec3_scale(float result[3], const float vec[3], float value);
void vec3_offset(float result[3], const float vec[3], float value);
void vec3_add(float result[3], const float vec1[3], const float vec2[3]);
void vec3_sub(float result[3], const float vec1[3], const float vec2[3]);
void vec3_mul(float result[3], const float vec1[3], const float vec2[3]);
void vec3_div(float result[3], const float vec1[3], const float vec2[3]);
void vec3_max(float result[3], const float vec1[3], const float vec2[3]);
void vec3_min(float result[3], const float vec1[3], const float vec2[3]);
void vec3_lerp(float result[3], const float vec1[3], const float vec2[3], float progress);

void vec4_scale(float result[4], const float vec[4], float value);
void vec4_offset(float result[4], const float vec[4], float value);
void vec4_add(float result[4], const float vec1[4], const float vec2[4]);
void vec4_sub(float result[4], const float vec1[4], const float vec2[4]);
void vec4_mul(float result[4], const float vec1[4], const float vec2[4]);
void vec4_div(float result[4], const float vec1[4], const float vec2[4]);
void vec4_max(float result[4], const float vec1[4], const float vec2[4]);
void vec4_min(float result[4], const float vec1[4], const float vec2[4]);
void vec4_lerp(float result[4], const float vec1[4], const float vec2[4], float progress);

#endif //__GLV_VEC_H
