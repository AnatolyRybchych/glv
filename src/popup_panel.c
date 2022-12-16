#include <glv/popup_panel.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    View *popup;
    View *content;
    bool is_popup_shown;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void on_get_data_size(View *popup_panel, Uint32 *size);
static void on_child_create(View *popup_panel, const GlvChildChanged *created);
static void on_child_delete(View *popup_panel, const GlvChildChanged *deleted);
static void on_get_docs(View *popup_panel, const ViewMsg *msg, GlvMsgDocs *docs);
static void on_show_popup(View *popup_panel);
static void on_hide_popup(View *popup_panel);
static void on_set_popup(View *popup_panel, View **popup);
static void on_set_content(View *popup_panel, View **content);
static void on_resize(View *popup_panel);

static void resize_content(View *popup_panel);
static void resize_popup(View *popup_panel);
static void resize_current(View *popup_panel);

ViewProc glv_popup_panel_proc = proc;
static Uint32 data_offset;

void glv_popup_panel_show_popup(View *popup_panel){
    SDL_assert(popup_panel != NULL);

    glv_push_event(popup_panel, VM_POPUP_PANEL_SHOW_POPUP, NULL, 0);
}

void glv_popup_panel_hide_popup(View *popup_panel){
    SDL_assert(popup_panel != NULL);

    glv_push_event(popup_panel, VM_POPUP_PANEL_HIDE_POPUP, NULL, 0);
}

void glv_popup_panel_set_popup(View *popup_panel, View *popup){
    SDL_assert(popup_panel != NULL);

    glv_push_event(popup_panel, VM_POPUP_PANEL_SET_POPUP, &popup, sizeof(popup));
}

void glv_popup_panel_set_content(View *popup_panel, View *content){
    SDL_assert(popup_panel != NULL);

    glv_push_event(popup_panel, VM_POPUP_PANEL_SET_CONTENT, &content, sizeof(content));
}

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_RESIZE:
        on_resize(view);
        break;
    case VM_CHILD_CREATE:
        on_child_create(view, in);
        break;
    case VM_CHILD_DELETE:
        on_child_delete(view, in);
        break;
    case VM_POPUP_PANEL_SHOW_POPUP:
        on_show_popup(view);
        break;
    case VM_POPUP_PANEL_HIDE_POPUP:
        on_hide_popup(view);
        break;
    case VM_POPUP_PANEL_SET_POPUP:
        on_set_popup(view, in);
        break;
    case VM_POPUP_PANEL_SET_CONTENT:
        on_set_content(view, in);
        break;
    case VM_GET_VIEW_DATA_SIZE:
        on_get_data_size(view, out);
        break;
    case VM_GET_DOCS:
        on_get_docs(view, in, out);
        break;
    default:
        parent_proc(view, msg, in, out);
    }
}

static void on_get_data_size(View *popup_panel, Uint32 *size){
    parent_proc(popup_panel, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void on_child_create(View *popup_panel, const GlvChildChanged *created){
    glv_hide_by(popup_panel, created->child);
}

static void on_child_delete(View *popup_panel, const GlvChildChanged *deleted){
    Data *data = glv_get_view_data(popup_panel, data_offset);

    if(data->popup == deleted->child){
        data->popup = NULL;

        if(data->is_popup_shown){
            GlvMgr *mgr = glv_get_mgr(popup_panel);
            printf("DODO: hide popup\n");
            glv_log_err(mgr, "popup panel: deleted child is popup");
        }
    }
    else if(data->content == deleted->child){
        data->content = NULL;

        if(data->is_popup_shown == false){
            GlvMgr *mgr = glv_get_mgr(popup_panel);
            glv_log_err(mgr, "popup panel: deleted child is content");
        }
    }
}

static void on_get_docs(View *popup_panel, const ViewMsg *msg, GlvMsgDocs *docs){
    switch (*msg){
    case VM_POPUP_PANEL_SHOW_POPUP:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_POPUP_PANEL_SHOW_POPUP),
            "NULL", "NULL", "show popup view and hide content view");
        break;
    case VM_POPUP_PANEL_HIDE_POPUP:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_POPUP_PANEL_HIDE_POPUP),
            "NULL", "NULL", "hide popup view and show content view");
        break;
    case VM_POPUP_PANEL_SET_POPUP:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_POPUP_PANEL_HIDE_POPUP),
            "View **popup", "NULL", "set popup view, popup view should be child");
        break;
    case VM_POPUP_PANEL_SET_CONTENT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_POPUP_PANEL_HIDE_POPUP),
            "View **content", "NULL", "set content view, content view shoud be child");
        break;
    default:
        parent_proc(popup_panel, VM_GET_DOCS, (void*)msg, docs);
        break;
    }
}

static void on_show_popup(View *popup_panel){
    Data *data = glv_get_view_data(popup_panel, data_offset);

    if(data->popup == NULL){
        GlvMgr *mgr = glv_get_mgr(popup_panel);

        glv_log_err(mgr, "popup panel: error while show popup, popup is NULL");
        return;
    }

    if(data->content != NULL){
        glv_hide_by(popup_panel, data->content);
    }
    glv_show_by(popup_panel, data->popup);
    resize_popup(popup_panel);

    data->is_popup_shown = true;
}

static void on_hide_popup(View *popup_panel){
    Data *data = glv_get_view_data(popup_panel, data_offset);

    if(data->popup != NULL) glv_hide_by(popup_panel, data->popup);
    if(data->content != NULL){
        glv_show_by(popup_panel, data->content);
        resize_content(popup_panel);
    }
    data->is_popup_shown = false;
}

static void on_set_popup(View *popup_panel, View **popup){
    Data *data = glv_get_view_data(popup_panel, data_offset);
    GlvMgr *mgr = glv_get_mgr(popup_panel);

    if(data->is_popup_shown){
        glv_log_err(mgr, "popup panel: cannot change popup while popup is shown");
    }
    else if(glv_is_child_of(popup_panel, *popup) == false){
        glv_log_err(mgr, "popup panel: popup is not child of popup panel");
    }
    else if(data->content == *popup){
        glv_log_err(mgr, "popup panel: popup cannot be same as content");
    }
    else{
        data->popup = *popup;
    }
}

static void on_set_content(View *popup_panel, View **content){
    Data *data = glv_get_view_data(popup_panel, data_offset);
    GlvMgr *mgr = glv_get_mgr(popup_panel);

    if(glv_is_child_of(popup_panel, *content) == false){
        glv_log_err(mgr, "popup panel: content is not child of popup panel");
        return;
    }
    if(data->popup == *content){
        glv_log_err(mgr, "popup panel: popup cannot be same as content");
        return;
    }

    if(data->is_popup_shown){
        data->content = *content;
    }
    else{
        if(data->content != NULL) glv_hide_by(popup_panel, data->content);
        data->content = *content;
        glv_show_by(popup_panel, data->content); 
        resize_content(popup_panel);
    }
}

static void on_resize(View *popup_panel){
    resize_current(popup_panel);
}

static void resize_content(View *popup_panel){
    Data *data = glv_get_view_data(popup_panel, data_offset);
    if(data->content == NULL){
        //probably error log
        return;
    }

    SDL_Point size = glv_get_size(popup_panel);
    glv_set_size_by(popup_panel, data->content, size.x, size.y);
}

static void resize_popup(View *popup_panel){
    Data *data = glv_get_view_data(popup_panel, data_offset);
    if(data->popup == NULL){
        //probably error log
        return;
    }

    SDL_Point size = glv_get_size(popup_panel);
    //here is popup size handling
    glv_set_size_by(popup_panel, data->popup, size.x, size.y);
}

static void resize_current(View *popup_panel){
    Data *data = glv_get_view_data(popup_panel, data_offset);

    if(data->is_popup_shown){
        resize_popup(popup_panel);
    }
    else{
        resize_content(popup_panel);
    }
}