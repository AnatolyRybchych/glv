#ifndef __GLV_VEC_H
#define __GLV_VEC_H

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
