#include <glv/math/line.h>

float line_point_dst(float line_p1[2], float line_p2[2], float p[2]){
    float lvec[2] = {
        line_p2[0] - line_p1[0],
        line_p2[1] - line_p1[1],
    };

    float v = lvec[0] * (line_p1[1] - p[1]) - (line_p1[0] - p[0]) * lvec[1];
    v = v < 0 ? -v : v;

    return v / sqrtf(lvec[0] * lvec[0] + lvec[1] * lvec[1]);
}