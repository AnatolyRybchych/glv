#include <glv/margin.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    int abs[4]; 
    float rel[4]; 
    GlvMarginOp op[4];
} Data;

typedef int (*OpFunc)(int abs, int rel_as_abs);

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void apply_data_size(View *margin, Uint32 *size);
static void apply_set_absolute(View *margin, int m[4]);
static void apply_set_realtive(View *margin, float m[4]);
static void apply_set_abs_rel_operation(View *margin, GlvMarginOp op[4]);

static void margin_rel_to_abs(int absolute[4], const float realtive[4], const SDL_Point *size);

static bool margin_apply_op(int *result, int abs, int rel_as_abs, GlvMarginOp op);
static OpFunc margin_op_as_func(GlvMarginOp op);

static int margin_op_add(int abs, int rel);
static int margin_op_sub(int abs, int rel);
static int margin_op_min(int abs, int rel);
static int margin_op_max(int abs, int rel);
static int margin_op_take_rel(int abs, int rel);
static int margin_op_take_abs(int abs, int rel);
static int margin_op_max_add_zero(int abs, int rel);
static int margin_op_max_sub_zero(int abs, int rel);

static void apply_align_to_childs(View *margin, int m[4]);
static void enum_apply_align_to_childs(View *child, void *args);

static void align(View *margin);

ViewProc glv_margin_proc = proc;
static Uint32 data_offset;

void glv_margin_set_absolute(View *margin, int left, int top, int right, int bottom){
    int args[] = {left, top, right, bottom};

    glv_push_event(margin, VM_MARGIN_SET_ABSOLUTE, args, sizeof(args));
}

void glv_margin_set_relative(View *margin, float left, float top, float right, float bottom){
    float args[] = {left, top, right, bottom};

    glv_push_event(margin, VM_MARGIN_SET_RELATIVE, args, sizeof(args));
}

void glv_margin_set_rel_abs_operation(View *margin, GlvMarginOp left, GlvMarginOp top, GlvMarginOp right, GlvMarginOp bottom){
    GlvMarginOp args[] = {left, top, right, bottom};

    glv_push_event(margin, VM_MARGIN_SET_ABS_REL_OPERATION, args, sizeof(args));
}

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_GET_VIEW_DATA_SIZE:
        apply_data_size(view, out);
        break;
    case VM_CHILD_CREATE:
    case VM_CHILD_DELETE:
    case VM_CHILD_HIDE:
    case VM_CHILD_SHOW:
    case VM_CHILD_RESIZE:
    case VM_CHILD_MOVE:
        if(((GlvChildChanged*)in)->sender != view){
            align(view);
        }
        break;
    case VM_SET_BG:
        glv_swap_texture_with_bg(view);
        break;
    case VM_RESIZE:
        align(view);
        break;
    case VM_MARGIN_SET_RELATIVE:
        apply_set_realtive(view, in);
        align(view);
        break;
    case VM_MARGIN_SET_ABSOLUTE:
        apply_set_absolute(view, in);
        align(view);
        break;
    case VM_MARGIN_SET_ABS_REL_OPERATION:
        apply_set_abs_rel_operation(view, in);
        align(view);
        break;
    default:
        parent_proc(view, msg, in, out);
    }
}

static void apply_data_size(View *margin, Uint32 *size){
    parent_proc(margin, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void apply_set_absolute(View *margin, int m[4]){
    Data *data = glv_get_view_data(margin, data_offset);

    data->abs[0] = m[0];
    data->abs[1] = m[1];
    data->abs[2] = m[2];
    data->abs[3] = m[3];
}

static void apply_set_realtive(View *margin, float m[4]){
    Data *data = glv_get_view_data(margin, data_offset);

    data->rel[0] = m[0];
    data->rel[1] = m[1];
    data->rel[2] = m[2];
    data->rel[3] = m[3];
}

static void apply_set_abs_rel_operation(View *margin, GlvMarginOp op[4]){
    Data *data = glv_get_view_data(margin, data_offset);

    data->op[0] = op[0];
    data->op[1] = op[1];
    data->op[2] = op[2];
    data->op[3] = op[3];
}

static void margin_rel_to_abs(int absolute[4], const float realtive[4], const SDL_Point *size){
    absolute[0] = realtive[0] * size->x;
    absolute[1] = realtive[1] * size->y;
    absolute[2] = realtive[2] * size->x;
    absolute[3] = realtive[3] * size->y;
}

static bool margin_apply_op(int *resut, int abs, int rel_as_abs, GlvMarginOp op){
    OpFunc func = margin_op_as_func(op);
    if(func != NULL){
        *resut = func(abs, rel_as_abs);
        return true;
    }
    else{
        *resut = 0;
        return false;
    }
}

static OpFunc margin_op_as_func(GlvMarginOp op){
    switch (op){
    case MARGIN_OP_ADD: return margin_op_add;
    case MARGIN_OP_SUB: return margin_op_sub;
    case MARGIN_OP_MIN: return margin_op_min;
    case MARGIN_OP_MAX: return margin_op_max;
    case MARGIN_OP_TAKE_RELATIVE: return margin_op_take_rel;
    case MARGIN_OP_TAKE_ABSOLUTE: return margin_op_take_abs;
    case MARGIN_OP_MAX_ADD_ZERO: return margin_op_max_add_zero;
    case MARGIN_OP_MAX_SUB_ZERO: return margin_op_max_sub_zero;
    default: return NULL;
    }
}

static int margin_op_add(int abs, int rel){ return abs + rel;}
static int margin_op_sub(int abs, int rel){ return abs - rel;}
static int margin_op_min(int abs, int rel){ return SDL_min(abs, rel);}
static int margin_op_max(int abs, int rel){ return SDL_max(abs, rel);}
static int margin_op_take_rel(int abs, int rel){abs = abs;/*unused*/ return rel;}
static int margin_op_take_abs(int abs, int rel){rel = rel;/*unused*/ return abs;}
static int margin_op_max_add_zero(int abs, int rel){ return SDL_max(0, margin_op_add(abs, rel));}
static int margin_op_max_sub_zero(int abs, int rel){ return SDL_max(0, margin_op_sub(abs, rel));}

static void apply_align_to_childs(View *margin, int m[4]){
    Uint32 handled = 0;
    SDL_Point size = glv_get_size(margin);

    void *args[] = {
        [0] = m,
        [1] = &size,
        [2] = margin,
        [3] = &handled,
    };

    glv_enum_visible_childs(margin, enum_apply_align_to_childs, args);

    if(handled > 1){
        glv_log_err(glv_get_mgr(margin), "margin: there is more than one visible child");
    }
}

static void enum_apply_align_to_childs(View *child, void *args){
    void **a = args;

    const int *margin = a[0];
    const SDL_Point *panel_size = a[1];
    View *panel = a[2];
    Uint32 *handled = a[3];

    glv_set_pos_by(panel, child, margin[0], margin[1]);
    glv_set_size_by(panel, child, panel_size->x - (margin[0] + margin[2]), panel_size->y - (margin[1] + margin[3]));

    handled[0]++;
}

static void align(View *margin){
    Data *data = glv_get_view_data(margin, data_offset);

    SDL_Point size = glv_get_size(margin);

    int rel_as_abs[4];
    margin_rel_to_abs(rel_as_abs, data->rel, &size);

    int m[4];
    if(!margin_apply_op(m + 0, data->abs[0], rel_as_abs[0], data->op[0])
    || !margin_apply_op(m + 1, data->abs[1], rel_as_abs[1], data->op[1])
    || !margin_apply_op(m + 2, data->abs[2], rel_as_abs[2], data->op[2])
    || !margin_apply_op(m + 3, data->abs[3], rel_as_abs[3], data->op[3])){
        glv_log_err(glv_get_mgr(margin), "margin: undefined operation between relative and absolute values");
    }

    apply_align_to_childs(margin, m);
}
