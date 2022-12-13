#include <glv/flex_panel.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct JustifyParams JustifyParams;

typedef struct Data{
    bool vertical;

    ViewMsg Justify;
    int justify_stratch_pad;
    int justify_stratch_min;
    int justify_stratch_max;
    ViewMsg Justify_stratch;

    ViewMsg align;
    int aligh_stratch_pad;
    int aligh_stratch_min;
    int aligh_stratch_max;
    ViewMsg align_stratch;
} Data;

struct JustifyParams{
    View *flex_panel;
    void (*set_pos)(View *flex_panel, View *child, int pos);
    void (*set_size)(View *flex_panel, View *child, Uint32 size);
    int (*get_pos)(View *child);
    Uint32 (*get_size)(View *child);
    Uint32 (*calc_target_size)(View *child, JustifyParams *params);

    Uint32 panel_size;
    Uint32 panel_contents_size;

    int justify_stratch_pad;
    int justify_stratch_min;
    int justify_stratch_max;

    Uint32 childs_cnt;

    ViewMsg justify;
};

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void get_data_size(View *view, Uint32 *size);

static void justify_childs(View *flex_panel);

static void set_pos_x(View *flex_panel, View *child, int pos);
static void set_pos_y(View *flex_panel, View *child, int pos);
static void set_size_x(View *flex_panel, View *child, Uint32 size);
static void set_size_y(View *flex_panel, View *child, Uint32 size);

static int get_pos_x(View *child);
static int get_pos_y(View *child);
static Uint32 get_size_x(View *child);
static Uint32 get_size_y(View *child);

static void enum_get_contents_size_x(View *child, void *result);
static Uint32 get_contents_size_x(View *flex_panel);
static void enum_get_contents_size_y(View *flex_panel, void *result);
static Uint32 get_contents_size_y(View *flex_panel);

Uint32 calc_target_size_flex_no_stratch(View *child, JustifyParams *params);
Uint32 calc_target_size_flex_stratch_min_max(View *child, JustifyParams *params);
Uint32 calc_target_size_flex_stratch_pad_min(View *child, JustifyParams *params);

JustifyParams get_vertical_justify_params(View *flex_panel, const Data *data);
JustifyParams get_horisontal_justify_params(View *flex_panel, const Data *data);

void enum_resize_justify(View *child, void *args);
void resize_justify(JustifyParams *params);
void move_justify(JustifyParams *justify_params);

void move_justify_via_pad(JustifyParams *params, int pad_before, Uint32 pad_between);
void enum_move_justify_via_pad(View *child, void *args);

ViewProc glv_flex_panel_proc = proc;
static Uint32 data_offset;

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_CHILD_CREATE:
    case VM_CHILD_DELETE:
    case VM_CHILD_HIDE:
    case VM_CHILD_SHOW:
    case VM_CHILD_RESIZE:
    case VM_CHILD_MOVE:
        if(((const GlvChildChanged *)in)->sender != view){
            justify_childs(view);
        }
        break;
    case VM_RESIZE:
        justify_childs(view);
        break;
    case VM_GET_VIEW_DATA_SIZE:
        get_data_size(view, out);
        break;
    default:
        parent_proc(view, msg, in, out);
    }
}

static void justify_childs(View *flex_panel){
    Data *data = glv_get_view_data(flex_panel, data_offset);

    JustifyParams justify_params;
    if(data->vertical){
        justify_params = get_vertical_justify_params(flex_panel, data);
        resize_justify(&justify_params);
        justify_params.panel_contents_size =  get_contents_size_y(flex_panel);
    }
    else{
        justify_params = get_horisontal_justify_params(flex_panel, data);
        resize_justify(&justify_params);
        justify_params.panel_contents_size =  get_contents_size_x(flex_panel);
    }
    move_justify(&justify_params);
}

static void set_pos_x(View *flex_panel, View *child, int pos){
    SDL_Point prev_pos = glv_get_pos(child);
    glv_set_pos_by(flex_panel, child, pos, prev_pos.y);
}

static void set_pos_y(View *flex_panel, View *child, int pos){
    SDL_Point prev_pos = glv_get_pos(child);
    glv_set_pos_by(flex_panel, child, prev_pos.x, pos);
}

static void set_size_x(View *flex_panel, View *child, Uint32 pos){
    SDL_Point prev_size = glv_get_size(child);
    glv_set_size_by(flex_panel, child, pos, prev_size.y);
}

static void set_size_y(View *flex_panel, View *child, Uint32 pos){
    SDL_Point prev_size = glv_get_size(child);
    glv_set_size_by(flex_panel, child, prev_size.x, pos);
}

static int get_pos_x(View *child){
    SDL_Point pos = glv_get_pos(child);
    return pos.x;
}

static int get_pos_y(View *child){
    SDL_Point pos = glv_get_pos(child);
    return pos.y;
}

static Uint32 get_size_x(View *child){
    SDL_Point size = glv_get_size(child);
    return size.x;
}

static Uint32 get_size_y(View *child){
    SDL_Point size = glv_get_size(child);
    return size.y;
}

static void enum_get_contents_size_x(View *child, void *result){
    SDL_Point child_size = glv_get_size(child);
    Uint32 *res_size = result;
    res_size[0] += child_size.x;
}

static Uint32 get_contents_size_x(View *flex_panel){
    Uint32 result = 0;
    glv_enum_visible_childs(flex_panel, enum_get_contents_size_x, &result);
    return result;
}

static void enum_get_contents_size_y(View *flex_panel, void *result){
    SDL_Point child_size = glv_get_size(flex_panel);
    Uint32 *res_size = result;
    res_size[0] += child_size.y;
}

static Uint32 get_contents_size_y(View *flex_panel){
    Uint32 result = 0;
    glv_enum_visible_childs(flex_panel, enum_get_contents_size_y, &result);
    return result;
}

Uint32 calc_target_size_flex_no_stratch(View *child, JustifyParams *params){
    return params->get_size(child);
}

Uint32 calc_target_size_flex_stratch_min_max(View *child, JustifyParams *params){
    int max_fit = params->panel_contents_size / params->childs_cnt;

    if(max_fit > params->justify_stratch_max){
        return params->justify_stratch_max;
    }

    int child_size = params->get_size(child);

    if(child_size > params->justify_stratch_min){
        return child_size;
    }
    else{
        return params->justify_stratch_min;
    }
}

Uint32 calc_target_size_flex_stratch_pad_min(View *child, JustifyParams *params){
    child = child;//unused
    int max_fit = params->panel_contents_size / params->childs_cnt;

    if(max_fit - params->justify_stratch_pad < params->justify_stratch_min){
        return params->justify_stratch_min;
    }
    else{
        return max_fit - params->justify_stratch_pad;
    }
}

JustifyParams get_vertical_justify_params(View *flex_panel, const Data *data){
    SDL_Point panel_size = glv_get_size(flex_panel);
    JustifyParams result = {
        .set_pos = set_pos_y,
        .set_size = set_size_y,
        .get_pos = get_pos_y,
        .get_size = get_size_y,
        .justify_stratch_max = data->justify_stratch_max,
        .justify_stratch_min = data->justify_stratch_min,
        .justify_stratch_pad = data->justify_stratch_pad,
        .calc_target_size = NULL,
        .panel_contents_size = get_contents_size_y(flex_panel),
        .panel_size = panel_size.y,
        .flex_panel = flex_panel,
        .justify = data->Justify,
    };

    switch (data->Justify_stratch){
    case VM_FLEX_PANEL_SET_JUSTIFY_STRATCH_MIN_MAX:
        result.calc_target_size = calc_target_size_flex_stratch_min_max;
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_STRATCH_PAD_MIN:
        result.calc_target_size = calc_target_size_flex_stratch_pad_min;
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_NO_STRATCH:
        result.calc_target_size = calc_target_size_flex_no_stratch;
        break;
    default:
        glv_log_err(glv_get_mgr(flex_panel), "flex panel: undefined justify alignment param");
        result.calc_target_size = calc_target_size_flex_no_stratch;
        break;
    }
    return result;
}

JustifyParams get_horisontal_justify_params(View *flex_panel, const Data *data){
    SDL_Point panel_size = glv_get_size(flex_panel);
    JustifyParams result = {
        .set_pos = set_pos_x,
        .set_size = set_size_x,
        .get_pos = get_pos_x,
        .get_size = get_size_x,
        .justify_stratch_max = data->justify_stratch_max,
        .justify_stratch_min = data->justify_stratch_min,
        .justify_stratch_pad = data->justify_stratch_pad,
        .calc_target_size = NULL,
        .panel_contents_size = get_contents_size_x(flex_panel),
        .panel_size = panel_size.x,
        .flex_panel = flex_panel,
        .justify = data->Justify,
    };

    switch (data->Justify_stratch){
    case VM_FLEX_PANEL_SET_JUSTIFY_STRATCH_MIN_MAX:
        result.calc_target_size = calc_target_size_flex_stratch_min_max;
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_STRATCH_PAD_MIN:
        result.calc_target_size = calc_target_size_flex_stratch_pad_min;
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_NO_STRATCH:
        result.calc_target_size = calc_target_size_flex_no_stratch;
        break;
    default:
        glv_log_err(glv_get_mgr(flex_panel), "flex panel: undefined justify stratching param");
        result.calc_target_size = calc_target_size_flex_no_stratch;
        break;
    }
    return result;
}

void resize_justify(JustifyParams *params){
    glv_enum_visible_childs(params->flex_panel, enum_resize_justify, params);
}

void move_justify(JustifyParams *justify_params){
    int pad_space = (int)justify_params->panel_size - (int)justify_params->panel_contents_size;
    if(pad_space < 0) pad_space = 0;

    switch (justify_params->justify){
    case VM_FLEX_PANEL_SET_JUSTIFY_START:
        move_justify_via_pad(justify_params, 0, 0);
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_CENTER:
        move_justify_via_pad(justify_params, pad_space / 2, 0);
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_SPACE_AROUND:
        move_justify_via_pad(justify_params, 
            pad_space / justify_params->childs_cnt / 2, 
            pad_space / justify_params->childs_cnt);
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_SPACE_BETWEEN:
        move_justify_via_pad(justify_params, 0, 
            pad_space / (justify_params->childs_cnt - 1));
        break;
    case VM_FLEX_PANEL_SET_JUSTIFY_SPACE_EVENLY:
        move_justify_via_pad(justify_params,
            pad_space / (justify_params->childs_cnt + 1),
            pad_space / (justify_params->childs_cnt + 1));
        break;
    default:
        glv_log_err(
            glv_get_mgr(justify_params->flex_panel),
            "flex panel: undefined justify alignment param");
        move_justify_via_pad(justify_params, 0, 0);
        break;
    }
}

void move_justify_via_pad(JustifyParams *params, int pad_before, Uint32 pad_between){
    void *p[] = {
        [0] = params,
        [1] = &pad_before,
        [2] = &pad_between,
    };

    glv_enum_visible_childs(params->flex_panel, enum_move_justify_via_pad, p);
}

void enum_move_justify_via_pad(View *child, void *args){
    void **params = args;
    const JustifyParams *justify_params = params[0];
    int *prev_end = params[1];
    int *pad = params[2];

    justify_params->set_pos(justify_params->flex_panel, child, *prev_end);
    prev_end[0] += *pad + justify_params->get_size(child); 

    
}


void enum_resize_justify(View *child, void *args){
    JustifyParams *params = args;
    params->set_size(params->flex_panel, child, params->calc_target_size(child, params));
}

static void get_data_size(View *view, Uint32 *size){
    parent_proc(view, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}
