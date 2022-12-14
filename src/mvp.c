#include <glv/math/mvp.h>
#include <stdio.h>
 
#define EL(TARGET, N, X, Y) ((TARGET)[(Y) * (N) + (X)]) 
#define SIZEOF_MAT (sizeof(float) * 4 * 4)
void mvp_identity(float mat[4*4]){
    memset(mat, 0, SIZEOF_MAT);
    mat[0] = mat[5] = mat[10] = mat[15] = 1;
}

void mvp_translate(float mat[4*4], float translation[3]){
    float t_mat[4*4] = {
        1, 0, 0, translation[0],
        0, 1, 0, translation[1],
        0, 0, 1, translation[2],
        0, 0, 0, 1,
    };

    

    float mat_cp[4*4];
    memcpy(mat_cp, mat, SIZEOF_MAT);

    mat_mul(mat, mat_cp, t_mat, 4);
}

void mvp_scale(float mat[4*4], float scale[3]){
    float s_mat[4*4] = {
        scale[0], 0, 0, 0,
        0, scale[1], 0, 0,
        0, 0, scale[2], 0,
        0, 0, 0, 1,
    };

    float mat_cp[4*4];
    memcpy(mat_cp, mat, SIZEOF_MAT);

    mat_mul(mat, mat_cp, s_mat, 4);
}

void mvp_rotate_x(float mat[4*4], float angle){
    float r_mat[4*4] = {
        1, 0, 0, 0,
        0, cosf(angle), -sinf(angle), 0,
        0, 0, sinf(angle), cosf(angle),
        0, 0, 0, 1,
    };

    float mat_cp[4*4];
    memcpy(mat_cp, mat, SIZEOF_MAT);

    mat_mul(mat, mat_cp, r_mat, 4);
}

void mvp_rotate_y(float mat[4*4], float angle){
    float r_mat[4*4] = {
        cosf(angle), 0, sinf(angle), 0,
        0, 1, 0, 0,
        -sinf(angle), 0, cosf(angle), 0,
        0, 0, 0, 1,
    };

    float mat_cp[4*4];
    memcpy(mat_cp, mat, SIZEOF_MAT);

    mat_mul(mat, mat_cp, r_mat, 4);
}

void mvp_rotate_z(float mat[4*4], float angle){
    float r_mat[4*4] = {
        cosf(angle), -sinf(angle), 0, 0,
        sinf(angle), cosf(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    float mat_cp[4*4];
    memcpy(mat_cp, mat, SIZEOF_MAT);

    mat_mul(mat, mat_cp, r_mat, 4);
}



void mat_mul(float *result, const float *mat1, const float *mat2, unsigned int n){
    for (unsigned int i = 0; i < n; i++){
        for (unsigned int j = 0; j < n; j++){
            float curr = 0;
            for (unsigned int k = 0; k < n; k++){
                 curr += EL(mat1, n, i, k) * EL(mat2, n, k, j);   
            }
            EL(result, n, j, i) = curr;
        }
    }
}