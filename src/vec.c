#include <glv/math/vec.h>

#define APPLY1_UN(OP, ...) OP(result + 0, vec + 0,##__VA_ARGS__)
#define APPLY2_UN(OP, ...) OP(result + 1, vec + 1,##__VA_ARGS__); APPLY1_UN(OP,##__VA_ARGS__)
#define APPLY3_UN(OP, ...) OP(result + 2, vec + 2,##__VA_ARGS__); APPLY2_UN(OP,##__VA_ARGS__)
#define APPLY4_UN(OP, ...) OP(result + 3, vec + 3,##__VA_ARGS__); APPLY3_UN(OP,##__VA_ARGS__)

#define APPLY1_BIN(OP, ...) OP(result + 0, vec1 + 0, vec2 + 0,##__VA_ARGS__)
#define APPLY2_BIN(OP, ...) OP(result + 1, vec1 + 1, vec2 + 1,##__VA_ARGS__); APPLY1_BIN(OP,##__VA_ARGS__)
#define APPLY3_BIN(OP, ...) OP(result + 2, vec1 + 2, vec2 + 2,##__VA_ARGS__); APPLY2_BIN(OP,##__VA_ARGS__)
#define APPLY4_BIN(OP, ...) OP(result + 3, vec1 + 3, vec2 + 3,##__VA_ARGS__); APPLY3_BIN(OP,##__VA_ARGS__)

#define IMPL_VECN_OP_UN(N, OP) void vec##N##_##OP(float result[N], const float vec[N]){APPLY##N##_UN(OP);}
#define IMPL_VECN_OP_UN_EXTRA(N, OP, TYPE, NAME) void vec##N##_##OP(float result[N], const float vec[N], TYPE NAME){APPLY##N##_UN(OP, NAME);}

#define IMPL_VECN_OP_BIN(N, OP) void vec##N##_##OP(float result[N], const float vec1[N], const float vec2[N]){APPLY##N##_BIN(OP);}
#define IMPL_VECN_OP_BIN_EXTRA(N, OP, TYPE, NAME) void vec##N##_##OP(float result[N], const float vec1[N], const float vec2[N], TYPE NAME){APPLY##N##_BIN(OP, NAME);}

#define IMPL_VEC_DIMENTION(N)\
    IMPL_VECN_OP_UN_EXTRA(N, scale, float, value)\
    IMPL_VECN_OP_UN_EXTRA(N, offset, float, value)\
    IMPL_VECN_OP_BIN(N, add)\
    IMPL_VECN_OP_BIN(N, sub)\
    IMPL_VECN_OP_BIN(N, mul)\
    IMPL_VECN_OP_BIN(N, div)\
    IMPL_VECN_OP_BIN(N, max)\
    IMPL_VECN_OP_BIN(N, min)\
    IMPL_VECN_OP_BIN_EXTRA(N, lerp, float, progress)\

static void scale(float *result, const float *src, float scale){*result = *src * scale;}
static void offset(float *result, const float *src, float offset){*result = *src + offset;}

static void add(float *result, const float *src1, const float *src2){*result = *src1 + *src2;}
static void sub(float *result, const float *src1, const float *src2){*result = *src1 - *src2;}
static void mul(float *result, const float *src1, const float *src2){*result = *src1 * *src2;}
static void div(float *result, const float *src1, const float *src2){*result = *src1 / *src2;}
static void max(float *result, const float *src1, const float *src2){*result = *src1 > *src2 ? *src1 : *src2;}
static void min(float *result, const float *src1, const float *src2){*result = *src1 < *src2 ? *src1 : *src2;}

static void lerp(float *result, const float *src1, const float *src2, float progress){*result = *src1 + (*src2 - *src1) * progress;}

float lerpf(float from, float to, float progress){
    return from + (to - from) / progress;
}

IMPL_VEC_DIMENTION(2)
IMPL_VEC_DIMENTION(3)
IMPL_VEC_DIMENTION(4)


