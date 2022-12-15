#include <glv/menu_panel.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    ViewMsg menu_side;
    Uint32 menu_size;

    View *menu_view;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void init_data_size(View *menu_panel, Uint32 *size);
static void init_data(View *menu_panel);

static void apply_set_side(View *menu_panel, ViewMsg side);
static void apply_set_size(View *menu_panel, const Uint32 *size);
static void apply_set_menu(View *menu_panel, View **menu);
static void apply_docs(View *menu_panel, ViewMsg *msg, GlvMsgDocs *docs);

static void locate_childs(View *menu_panel);
static void enum_locate_childs(View *child, void *args);
static void locate_menu(View *panel, View *menu, Data *data, const SDL_Point *panel_size, ViewMsg menu_side);
static void locate_non_menu(View *panel, View *menu, Data *data, const SDL_Point *panel_size, ViewMsg menu_side);

static void enum_check_contains(View *child, void *args);

ViewProc glv_menu_panel_proc = proc;
static Uint32 data_offset;

void glv_menu_panel_set_left(View *menu_panel){
    SDL_assert(menu_panel != NULL);

    glv_push_event(menu_panel, VM_MENU_PANEL_SET_LEFT, NULL, 0);
}

void glv_menu_panel_set_top(View *menu_panel){
    SDL_assert(menu_panel != NULL);
    
    glv_push_event(menu_panel, VM_MENU_PANEL_SET_TOP, NULL, 0);
}

void glv_menu_panel_set_right(View *menu_panel){
    SDL_assert(menu_panel != NULL);
    
    glv_push_event(menu_panel, VM_MENU_PANEL_SET_RIGHT, NULL, 0);
}

void glv_menu_panel_set_bottom(View *menu_panel){
    SDL_assert(menu_panel != NULL);
    
    glv_push_event(menu_panel, VM_MENU_PANEL_SET_BOTTOM, NULL, 0);
}

void glv_menu_panel_set_size(View *menu_panel, Uint32 size){
    SDL_assert(menu_panel != NULL);
    
    glv_push_event(menu_panel, VM_MENU_PANEL_SET_MENU_SIZE, &size, sizeof(size));
}

void glv_menu_panel_set_menu(View *menu_panel, View *menu){
    SDL_assert(menu_panel != NULL);
    
    glv_push_event(menu_panel, VM_MENU_PANEL_SET_MENU, &menu, sizeof(menu));
}

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_GET_VIEW_DATA_SIZE:
        init_data_size(view, out);
        break;
    case VM_MENU_PANEL_SET_LEFT:
    case VM_MENU_PANEL_SET_TOP:
    case VM_MENU_PANEL_SET_RIGHT:
    case VM_MENU_PANEL_SET_BOTTOM:
        apply_set_side(view, msg);
        locate_childs(view);
        break;
    case VM_MENU_PANEL_SET_MENU_SIZE:
        apply_set_size(view, in);
        locate_childs(view);
        break;
    case VM_MENU_PANEL_SET_MENU:
        apply_set_menu(view, in);
        locate_childs(view);
        break;
    case VM_RESIZE:
        locate_childs(view);
        break;
    case VM_CHILD_HIDE:
    case VM_CHILD_SHOW:
    case VM_CHILD_MOVE:
    case VM_CHILD_RESIZE:
    case VM_CHILD_CREATE:
    case VM_CHILD_DELETE:
        if(((GlvChildChanged*)in)->sender != view){
            locate_childs(view);
        }
        break;
    case VM_SET_BG:
        glv_swap_texture_with_bg(view);
        break;
    case VM_CREATE:
        init_data(view);
        break;
    case VM_GET_DOCS:
        apply_docs(view, in, out);
        break;
    default:
        parent_proc(view, msg, in, out);
    }
}

static void init_data_size(View *menu_panel, Uint32 *size){
    parent_proc(menu_panel, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void init_data(View *menu_panel){
    Data *data = glv_get_view_data(menu_panel, data_offset);

    data->menu_side = VM_MENU_PANEL_SET_LEFT;
    data->menu_size = 100;
}

static void apply_set_side(View *menu_panel, ViewMsg side){
    Data *data = glv_get_view_data(menu_panel, data_offset);
    data->menu_side = side;
}

static void apply_set_size(View *menu_panel, const Uint32 *size){
    Data *data = glv_get_view_data(menu_panel, data_offset);
    data->menu_size = *size;
}

static void apply_set_menu(View *menu_panel, View **menu){
    Data *data = glv_get_view_data(menu_panel, data_offset);
    data->menu_view = *menu;

    bool is_contains = false;
    void *args[] = {
        [0] = *menu,
        [1] = &is_contains
    };

    glv_enum_childs(menu_panel, enum_check_contains, args);
    
    if(is_contains == false){
        glv_log_err(glv_get_mgr(menu_panel), "menu panel: error while set menu, menu view is not child of menu panel");
    }
}

static void apply_docs(View *menu_panel, ViewMsg *msg, GlvMsgDocs *docs){
    switch (*msg){
    case VM_MENU_PANEL_SET_LEFT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_LEFT),
            "NULL", "NULL", "changes menu location to left");
        break;
    case VM_MENU_PANEL_SET_RIGHT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_RIGHT),
            "NULL", "NULL", "changes menu location to right");
        break;
    case VM_MENU_PANEL_SET_BOTTOM:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_BOTTOM),
            "NULL", "NULL", "changes menu location to bottom");
        break;
    case VM_MENU_PANEL_SET_TOP:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_TOP),
            "NULL", "NULL", "changes menu location to top");
        break;
    case VM_MENU_PANEL_SET_MENU:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_MENU),
            "View *menu", "NULL", "changes target view to locate as menu");
        break;
    case VM_MENU_PANEL_SET_MENU_SIZE:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_MENU_PANEL_SET_MENU_SIZE),
            "Uint32 *size", "NULL", "changes menu size in main direction another dirrection is 100%");
        break;
    default:
        parent_proc(menu_panel, VM_GET_DOCS, msg, docs);
        break;
    }
}

static void locate_childs(View *menu_panel){
    Data *data = glv_get_view_data(menu_panel, data_offset);

    SDL_Point panel_size = glv_get_size(menu_panel);
    Uint32 count_resized = 0;
    void *args[] = {
        [0] = &panel_size,
        [1] = data,
        [2] = &count_resized,
        [3] = menu_panel,
    };

    glv_enum_visible_childs(menu_panel, enum_locate_childs, args);

    if(count_resized > 1){
        glv_log_err(glv_get_mgr(menu_panel), "menu panel: there is more than 1 visible non menu element, resizing is invalid");
    }
}

static void enum_locate_childs(View *child, void *args){
    void **a = args;
    SDL_Point *panel_size = a[0];
    Data *data = a[1];
    Uint32 *count_resized = a[2];
    View *menu_panel = a[3];

    if(child == data->menu_view){
        locate_menu(menu_panel, child, data, panel_size, data->menu_side);
    }
    else{
        locate_non_menu(menu_panel, child, data, panel_size, data->menu_side);
        count_resized++;
    }
}

static void locate_menu(View *panel, View *menu, Data *data, const SDL_Point *panel_size, ViewMsg menu_side){
    switch (menu_side){
    case VM_MENU_PANEL_SET_LEFT:
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, data->menu_size, panel_size->y);
        break;
    case VM_MENU_PANEL_SET_TOP:
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, panel_size->x, data->menu_size);
        break;
    case VM_MENU_PANEL_SET_RIGHT:
        glv_set_pos_by(panel, menu, panel_size->x - data->menu_size, 0);
        glv_set_size_by(panel, menu, data->menu_size, panel_size->y);
        break;
    case VM_MENU_PANEL_SET_BOTTOM:
        glv_set_pos_by(panel, menu, 0, panel_size->y - data->menu_size);
        glv_set_size_by(panel, menu, panel_size->x, data->menu_size);
        break;
    default:
        glv_log_err(glv_get_mgr(menu), "menu panel: unknown menu side, used left");
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, data->menu_size, panel_size->y);
        break;
    }
}

static void locate_non_menu(View *panel, View *menu, Data *data, const SDL_Point *panel_size, ViewMsg menu_side){
    switch (menu_side){
    case VM_MENU_PANEL_SET_LEFT:
        glv_set_pos_by(panel, menu, data->menu_size, 0);
        glv_set_size_by(panel, menu, panel_size->x - data->menu_size, panel_size->y);
        break;
    case VM_MENU_PANEL_SET_TOP:
        glv_set_pos_by(panel, menu, 0, data->menu_size);
        glv_set_size_by(panel, menu, panel_size->x, panel_size->y - data->menu_size);
        break;
    case VM_MENU_PANEL_SET_RIGHT:
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, panel_size->x - data->menu_size, panel_size->y);
        break;
    case VM_MENU_PANEL_SET_BOTTOM:
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, panel_size->x, panel_size->y - data->menu_size);
        break;
    default:
        glv_log_err(glv_get_mgr(menu), "menu panel: unknown menu side, used left");
        glv_set_pos_by(panel, menu, 0, 0);
        glv_set_size_by(panel, menu, data->menu_size, panel_size->y);
        break;
    }
}

static void enum_check_contains(View *child, void *args){
    void **a = args;
    View *view = a[0];
    bool *is_contains = a[1];
    
    if(child == view){
        *is_contains = true;
    }
}