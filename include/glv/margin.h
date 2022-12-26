#ifndef __GLV_MARGIN_H
#define __GLV_MARGIN_H

#include <glv/background.h>

#define VM_MARGIN_SET_ABSOLUTE VM_USER_FIRST + 20
#define VM_MARGIN_SET_RELATIVE VM_USER_FIRST + 21
#define VM_MARGIN_SET_ABS_REL_OPERATION VM_USER_FIRST + 22

extern ViewProc glv_margin_proc;

typedef Uint32 GlvMarginOp;
enum GlvMarginOp{
    MARGIN_OP_ADD,
    MARGIN_OP_SUB,
    MARGIN_OP_MIN,
    MARGIN_OP_MAX,
    MARGIN_OP_TAKE_RELATIVE,
    MARGIN_OP_TAKE_ABSOLUTE,

    MARGIN_OP_MAX_ADD_ZERO,
    MARGIN_OP_MAX_SUB_ZERO,
};

void glv_margin_set_absolute(View *margin_panel, int left, int top, int right, int bottom);
void glv_margin_set_relative(View *margin_panel, float left, float top, float right, float bottom);
void glv_margin_set_rel_abs_operation(View *margin_panel, GlvMarginOp left, GlvMarginOp top, GlvMarginOp right, GlvMarginOp bottom);

#endif //__GLV_MARGIN_H
