#ifndef GLV_H
#define GLV_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_opengl.h>
#include <stdbool.h>
#include <mvp.h>

typedef struct GlvMgr GlvMgr;
typedef struct View View;

typedef unsigned int ViewMsg;
typedef void (*ViewProc)(View *view, ViewMsg msg, void *in, void *out);
typedef void (*ViewManage)(View *view, ViewMsg msg, void *event_args, void *user_context);

typedef struct GlvMsgDocs GlvMsgDocs;

typedef struct GlvEventMouseButton GlvMouseDown;
typedef struct GlvEventMouseButton GlvMouseUp;
typedef struct GlvEventMouseMove GlvMouseMove;
typedef struct GlvEventKey GlvKeyUp;
typedef struct GlvEventKey GlvKeyDown;
typedef struct GlvEventTextEditing GlvTextEditing;

GlvMgr *glv_get_mgr(View *view);
    void glv_set_error_logger(GlvMgr *mgr, void (*logger_proc)(GlvMgr *mgr, const char *err));
    void glv_quit(GlvMgr *mgr);
    void glv_redraw_window(GlvMgr *mgr);
    void glv_draw_texture_mat2(GlvMgr *mgr, GLuint texture, float mvp[4*4], float tex_mvp[4*4]);
    void glv_draw_texture_mat(GlvMgr *mgr, GLuint texture, float mvp[4*4]);
    void glv_draw_texture_st(GlvMgr *mgr, GLuint texture, float scale[3], float translate[3], float angle);
    void glv_draw_texture_absolute(GlvMgr *mgr, GLuint texture, const SDL_Rect *src, const SDL_Rect *dst);
    void glv_log_err(GlvMgr *mgr, const char *err);
    SDL_Window *glv_get_window(GlvMgr *mgr);

    //SDL_USEREVENT is taken
    void glv_set_sdl_event_handler(GlvMgr *mgr, void(*on_sdl_event)(View *root, const SDL_Event *event, void *root_context));

void glv_enum_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);
void glv_enum_visible_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);
void glv_enum_focused_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);
void glv_enum_parents(View *view, void(*enum_proc)(View *parent, void *data), void *data);

View *glv_create(View *parent, ViewProc view_proc, ViewManage manage_proc, void *user_context);
void glv_delete(View *view);
void glv_proc_default(View *view, ViewMsg msg, void *in, void *out);
int glv_run(ViewProc root_view_proc, ViewManage root_view_manage, void *root_user_data, void (*init_spa)(View *root_view));
void *get_view_data(View *view, unsigned int offset);

//handles in message queue
//copies args 
//handles by manage proc
void glv_push_event(View *view, ViewMsg message, void *args, uint32_t args_size);

//handles instantly
//doesnt handles by manage proc
void glv_call_event(View *view, ViewMsg message, void *in, void *out);

//handles only by manage proc
void glv_call_manage(View *view, ViewMsg message, void *event_args);

void glv_bind_view_framebuffer(View *view);
GLuint glv_get_texture(View *view);

GlvMsgDocs glv_get_docs(View *view, ViewMsg message);
void glv_print_docs(View *view, ViewMsg message);

void glv_write_docs(GlvMsgDocs *docs, ViewMsg message, 
    const char *name, const char *input_description, 
    const char *output_description, const char *general_description);

void glv_set_minimal_frametime(GlvMgr *mgr, Uint32 ms_min_frametime);

void glv_set_pos(View *view, int x, int y);
void glv_set_size(View *view, unsigned int width, unsigned int height);
void glv_draw(View *view);
void glv_show(View *view);
void glv_hide(View *view);
void glv_set_focus(View *view);
SDL_Point glv_get_pos(View *view);
SDL_Point glv_get_size(View *view);
//return NULL if view is root
View *glv_get_Parent(View *view);
bool glv_is_focused(View *view);
bool glv_is_visible(View *view);
bool glv_is_mouse_over(View *view);

//takes focus witout unfocusing others
void glv_set_secondary_focus(View *view);

//removes only view focus and recursive childs
void glv_unset_secondary_focus(View *view);

enum ViewMsg{
    VM_NULL,
    //calls last but not sent to view_proc or manage_proc
    VM_VIEW_FREE__,
    
    VM_CREATE,
    VM_DELETE,
    VM_RESIZE,
    VM_MOVE,
    VM_MOUSE_DOWN,
    VM_MOUSE_UP,
    VM_MOUSE_MOVE,
    VM_DRAW,
    VM_SHOW,
    VM_HIDE,
    VM_FOCUS,
    VM_UNFOCUS,
    VM_KEY_DOWN,
    VM_KEY_UP,

    VM_CHILD_RESIZE,
    VM_CHILD_MOVE,
    VM_CHILD_CREATE,
    VM_CHILD_DELETE,

    VM_MOUSE_HOVER,
    VM_MOUSE_LEAVE,

    VM_TEXT,
    VM_TEXT_EDITING,

    VM_GET_DOCS,
    VM_GET_VIEW_DATA_SIZE,

    //user defined events should be in range [VM_USER_FIRST; VM_USER_LAST], this count is reserved by SDL_RegisterEvents
    VM_USER_FIRST = 100,
    //user defined events should be in range [VM_USER_FIRST; VM_USER_LAST], this count is reserved by SDL_RegisterEvents
    VM_USER_LAST = 999,
};

struct GlvMsgDocs {
    ViewMsg msg;
    //all strings is static
    const char *name;
    const char *description;
    const char *input_description;
    const char *output_description;
};

struct GlvEventMouseButton{
    //the mouse instance id, or SDL_TOUCH_MOUSEID
    Uint32 which;
    Uint8 button;
    Uint8 clicks;
    int x;
    int y;
};

struct GlvEventMouseMove{
    //the mouse instance id, or SDL_TOUCH_MOUSEID
    Uint32 which;
    int x;
    int y;
};

struct GlvEventKey{
    SDL_Keysym sym;
    Uint8 repeat;
};

struct GlvEventTextEditing{
    char composition[32];
    int cursor;
    int selection_len;
};

#endif //GLV_H
