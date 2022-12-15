#ifndef __GLV_COORDS_H
#define __GLV_COORDS_H

#define coords_rel __glv_coords_rel
#define coords_abs __glv_coords_abs
#define coords_rel_sz __glv_coords_rel_sz
#define coords_abs_sz __glv_coords_abs_sz

//relative is coords where 0;0 are viewport center, 1;1, -1;-1, 1;-1; -1;1 are viewport virtices and y axis is inverted 
void coords_rel(float relative[2], const int absolute[2], const int viewport[4]);

//relative is coords where 0;0 are viewport center, 1;1, -1;-1, 1;-1; -1;1 are viewport virtices and y axis is inverted 
void coords_abs(int absolute[2], const float relative[2], const int viewport[4]);

//relative is coords where 0;0 are viewport center, 1;1, -1;-1, 1;-1; -1;1 are viewport virtices and y axis is inverted 
void coords_rel_sz(float relative[2], const int absolute[2], const int viewport_size[2]);

//relative is coords where 0;0 are viewport center, 1;1, -1;-1, 1;-1; -1;1 are viewport virtices and y axis is inverted 
void coords_abs_sz(int absolute[2], const float relative[2], const int viewport_size[2]);

#endif //__GLV_COORDS_H
