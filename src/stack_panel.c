#include <glv/stack_panel.h>

#define parent_proc(view, msg, in, out) glv_background_proc(view, msg, in, out)

typedef struct Data{
    bool vertical;
    SDL_Point alignment;
    bool stretch[2];
}Data;

typedef struct ChildsLocationArgs{
    SDL_Point childs_size;
    SDL_Point panel_size;
    SDL_Point alignment;
    SDL_Point curr;
    bool vertical;
} ChildsLocationArgs;

static void proc(View *view, ViewMsg msg, void *in, void *out);
static void init_data_size(View *stack_panel, Uint32 *size);

static void locate_childs(View *stack_panel);
static void stretch_childs(View *stack_panel);
static void enum_locate_childs(View *child, void *args);
static void enum_resize_w(View *child, void *args);
static void enum_resize_h(View *child, void *args);

static SDL_Point measure_childs(View *stack_panel);
static void enum_measure_childs(View *child, void *p);

static bool set_vertical(View *stack_panel);
static bool set_horisontal(View *stack_panel);
static void set_alignment(View *stack_panel, const SDL_Point *alignment);
static void set_stretch(View *stack_panel, const bool stretch[2]);
static void write_docs(View *stack_panel, ViewMsg msg, GlvMsgDocs *docs);

ViewProc glv_stack_panel_proc = proc;

static Uint32 data_offset ;

void glv_stack_panel_set_vertical(View *stack_panel){
    SDL_assert(stack_panel != NULL);

    glv_push_event(stack_panel, VM_STACK_PANEL_SET_VERTICAL, NULL, 0);
}

void glv_stack_panel_set_horisontal(View *stack_panel){
    SDL_assert(stack_panel != NULL);

    glv_push_event(stack_panel, VM_STACK_PANEL_SET_HORISONTAL, NULL, 0);
}

void glv_stack_panel_set_alignment(View *stack_panel, int x, int y){
    SDL_assert(stack_panel != NULL);
    SDL_Point point = {
        .x = x,
        .y = y,
    };
    glv_push_event(stack_panel, VM_STACK_PANEL_SET_ALIGNMENT, &point, sizeof(point));
}

void glv_stack_panel_set_stretching(View *stack_panel, bool x, bool y){
    SDL_assert(stack_panel != NULL);

    bool stretch[2] = {
        [0] = x,
        [1] = y,
    };
    glv_push_event(stack_panel, VM_STACK_PANEL_SET_STRETCH, stretch, sizeof(stretch));
}

static void proc(View *view, ViewMsg msg, void *in, void *out){
    in = in;
    out = out;
    view = view;

    switch (msg){
    case VM_GET_VIEW_DATA_SIZE:
        init_data_size(view, out); 
        break;
    case VM_STACK_PANEL_SET_VERTICAL:
        if(set_vertical(view)) {
            stretch_childs(view);
            locate_childs(view);
        }
        break;
    case VM_STACK_PANEL_SET_HORISONTAL:
        if(set_horisontal(view)) {
            stretch_childs(view);
            locate_childs(view);
        }
        break;
    case VM_STACK_PANEL_SET_ALIGNMENT:
        set_alignment(view, in);
        stretch_childs(view);
        locate_childs(view);
        break;
    case VM_STACK_PANEL_SET_STRETCH:
        set_stretch(view, in);
        stretch_childs(view);
        locate_childs(view);
        break;
    case VM_CHILD_DELETE:
    case VM_CHILD_RESIZE:
    case VM_CHILD_MOVE:
    case VM_CHILD_CREATE:
    case VM_CHILD_HIDE:
    case VM_CHILD_SHOW:
        if(((const GlvChildChanged *)in)->sender != view){
            stretch_childs(view);
            locate_childs(view);
        }
    break;
    case VM_RESIZE:
        stretch_childs(view);
        locate_childs(view);
        break;
    case VM_GET_DOCS:
        write_docs(view, *(ViewMsg*)in, out);
        break;
    default:
        parent_proc(view, msg, in, out);
        break;
    }
}

static void init_data_size(View *stack_panel, Uint32 *size){
    parent_proc(stack_panel, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void locate_childs(View *stack_panel){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    SDL_Point size = glv_get_size(stack_panel);
    SDL_Point chils_size = measure_childs(stack_panel);
    SDL_Point curr = {0, 0};

    if(data->vertical){
        if(data->alignment.y < 0) curr.y = 0;
        else if(data->alignment.y == 0) curr.y = (size.y - chils_size.y) / 2;
        else curr.y = size.y - chils_size.y;
    }
    else{
        if(data->alignment.x < 0) curr.x = 0;
        else if(data->alignment.x == 0) curr.x = (size.x - chils_size.x) / 2;
        else curr.x = size.x - chils_size.x;
    }


    ChildsLocationArgs location_args = {
        .panel_size = size,
        .childs_size = chils_size,
        .alignment = data->alignment,
        .curr = curr,
        .vertical = data->vertical,
    };

    glv_enum_childs(stack_panel, enum_locate_childs, &location_args);
}

static void stretch_childs(View *stack_panel){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    SDL_Point panel_size = glv_get_size(stack_panel);
    Uint32 panel_childs_cnt = glv_get_childs_cnt(stack_panel);

    if(data->stretch[0]){
        Uint32 childs_width;
        if(data->vertical){
            childs_width = panel_size.x;
        }
        else{
            childs_width = panel_size.x / panel_childs_cnt;
        }

        void *args[] = {
            [0] = &childs_width,
            [1] = stack_panel,
        };
        glv_enum_visible_childs(stack_panel, enum_resize_w, args);
    }

    if(data->stretch[1]){
        Uint32 childs_height;
        if(data->vertical){
            childs_height = panel_size.y / panel_childs_cnt;
        }
        else{
            childs_height = panel_size.y;
        }

        void *args[] = {
            [0] = &childs_height,
            [1] = stack_panel,
        };
        glv_enum_visible_childs(stack_panel, enum_resize_h, args);
    }
}

static void enum_locate_childs(View *child, void *args){
    ChildsLocationArgs *location_params = args;

    SDL_Point child_size = glv_get_size(child);

    if(location_params->vertical){
        int x = 0;
        if(location_params->alignment.x == 0) x = (location_params->panel_size.x - child_size.x) / 2;
        else if(location_params->alignment.x > 0) x = (location_params->panel_size.x - child_size.x);

        glv_set_pos_by(glv_get_Parent(child), child, x, location_params->curr.y);

        location_params->curr.y += child_size.y;
    }
    else{
        int y = 0;
        if(location_params->alignment.y == 0) y = (location_params->panel_size.y - child_size.y) / 2;
        else if(location_params->alignment.y > 0) y = (location_params->panel_size.y - child_size.y);

        glv_set_pos_by(glv_get_Parent(child), child, location_params->curr.x, y);

        location_params->curr.x += child_size.x;
    }
}

static void enum_resize_w(View *child, void *args){
    void **a = args;
    Uint32 *width = a[0];
    View *panel = a[1];

    SDL_Point size = glv_get_size(child);

    glv_set_size_by(panel, child, *width, size.y);
}

static void enum_resize_h(View *child, void *args){
    void **a = args;
    Uint32 *height = a[0];
    View *panel = a[1];

    SDL_Point size = glv_get_size(child);

    glv_set_size_by(panel, child, size.x, *height);
}

static SDL_Point measure_childs(View *stack_panel){
    SDL_Point result = {0, 0};

    glv_enum_visible_childs(stack_panel, enum_measure_childs, &result);
    return result;
}

static void enum_measure_childs(View *child, void *p){
    SDL_Point *curr_size = p;
    SDL_Point child_size = glv_get_size(child);

    curr_size->x += child_size.x;  
    curr_size->y += child_size.y;  
}

static bool set_vertical(View *stack_panel){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    if(data->vertical){
        return false;
    }
    else{
        return data->vertical = true;
    }
}

static bool set_horisontal(View *stack_panel){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    if(data->vertical){
        data->vertical = false;
        return true;
    }
    else{
        return false;
    }
}

static void set_alignment(View *stack_panel, const SDL_Point *alignment){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    data->alignment = *alignment;
}

static void set_stretch(View *stack_panel, const bool stretch[2]){
    Data *data = glv_get_view_data(stack_panel, data_offset);

    data->stretch[0] = stretch[0];
    data->stretch[1] = stretch[1];
}

static void write_docs(View *stack_panel, ViewMsg msg, GlvMsgDocs *docs){
    switch (msg){
    case VM_STACK_PANEL_SET_VERTICAL:
        glv_write_docs(docs, msg, SDL_STRINGIFY_ARG(VM_STACK_PANEL_SET_VERTICAL),
            "NULL", "NULL", "set stack panel as vertical");
        break;
    case VM_STACK_PANEL_SET_HORISONTAL:
        glv_write_docs(docs, msg, SDL_STRINGIFY_ARG(VM_STACK_PANEL_SET_HORISONTAL),
            "NULL", "NULL", "set stack panel as horisontal");
        break;
    case VM_STACK_PANEL_SET_ALIGNMENT:
        glv_write_docs(docs, msg, SDL_STRINGIFY_ARG(VM_STACK_PANEL_SET_HORISONTAL),
            "const SDL_Point *alignment", "NULL", "set stack panel elements alignment: x -> horisontal,"
            " y -> vertical; if < 0 -> alignment begin, == 0 -> alignment center, > 0 -> alignment end");
        break;
    case VM_STACK_PANEL_SET_STRETCH:
        glv_write_docs(docs, msg, SDL_STRINGIFY_ARG(VM_STACK_PANEL_SET_STRETCH),
            "const bool stretch[2]", "NULL", "set stack panel elements stretch: [0] -> x, [1] -> y"
            "if true, elements will be stretched to fill all space in chosen dirrection");
        break;
    default:
        parent_proc(stack_panel, VM_GET_DOCS, &msg, docs);
    }
}