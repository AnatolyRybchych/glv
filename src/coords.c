#include <glv/math/coords.h>

static float rel_x(int absolute, int viewport);
static float rel_y(int absolute, int viewport);
static int abs_x(float relative, int viewport);
static int abs_y(float relative, int viewport);

void coords_rel(float relative[2], const int absolute[2], const int viewport[4]){
    relative[0] = rel_x(absolute[0], viewport[2]);
    relative[1] = rel_y(absolute[1], viewport[3]);
}

void coords_abs(int absolute[2], const float relative[2], const int viewport[4]){
    absolute[0] = abs_x(relative[0], viewport[2]);
    absolute[1] = abs_y(relative[1], viewport[3]);
}

void coords_rel_sz(float relative[2], const int absolute[2], const int viewport_size[2]){
    relative[0] = rel_x(absolute[0], viewport_size[0]);
    relative[1] = rel_y(absolute[1], viewport_size[1]);
}

void coords_abs_sz(int absolute[2], const float relative[2], const int viewport_size[2]){
    absolute[0] = abs_x(relative[0], viewport_size[0]);
    absolute[1] = abs_y(relative[1], viewport_size[1]);
}

static float rel_x(int absolute, int viewport){
    if(viewport == 0){
        return 0;
    }
    else{
        return absolute / (float)(viewport / 2) - 1.0;
    }
}

static float rel_y(int absolute, int viewport){
    if(viewport == 0){
        return 0;
    }
    else{
        return 1.0 - absolute / (float)(viewport / 2);
    }
}

static int abs_x(float relative, int viewport){
    return (relative / 2.0 + 0.5) * (float) viewport;
}

static int abs_y(float relative, int viewport){
    return (0.5 - relative / 2.0) * (float) viewport;
}