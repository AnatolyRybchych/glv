#ifndef GLV_H
#define GLV_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>
#include <mvp.h>

typedef struct GlvMgr GlvMgr;
typedef struct View View;

typedef unsigned int ViewMsg;
typedef int GlvFontFaceId;
typedef void (*ViewProc)(View *view, ViewMsg msg, void *in, void *out);
typedef void (*ViewManage)(View *view, ViewMsg msg, void *event_args, void *user_context);

typedef struct GlvMsgDocs GlvMsgDocs;

typedef struct GlvEventMouseButton GlvMouseDown;
typedef struct GlvEventMouseButton GlvMouseUp;
typedef struct GlvEventMouseMove GlvMouseMove;
typedef struct GlvEventKey GlvKeyUp;
typedef struct GlvEventKey GlvKeyDown;
typedef struct GlvEventTextEditing GlvTextEditing;
typedef struct GlvEventChildChanged GlvChildChanged;

GlvMgr *glv_get_mgr(View *view);
    void glv_set_error_logger(GlvMgr *mgr, void (*logger_proc)(GlvMgr *mgr, const char *err));
    //exit stop event loop and free mgr and all of its resources (views, fonts)
    void glv_quit(GlvMgr *mgr);

    //redraw window after all events
    void glv_redraw_window(GlvMgr *mgr);

    //draw texture cu current framebuffer
    //mvp -> transformation matrix for texture vertices (identity is normal quad{{-1; -1}, {1; -1}, {-1, 1},{1, 1})
    //tex_mvp -> transformation matrix for texture uv coords (identity is {{0; 0}, {1; 0}, {0, 1},{1, 1})
    void glv_draw_texture_mat2(GlvMgr *mgr, GLuint texture, float mvp[4*4], float tex_mvp[4*4]);

    //glv_draw_texture_mat2 with tex_mvp == identity matrix
    void glv_draw_texture_mat(GlvMgr *mgr, GLuint texture, float mvp[4*4]);

    //glv_draw_texture_mat with mvp builded by scale, translation and angle, angle is rotation around z axis
    void glv_draw_texture_st(GlvMgr *mgr, GLuint texture, float scale[3], float translate[3], float angle);

    //draw src rect subtexture of texture in dst rect
    void glv_draw_texture_absolute(GlvMgr *mgr, GLuint texture, const SDL_Rect *src, const SDL_Rect *dst);

    
    void glv_draw_circle_rel(GlvMgr *mgr, Uint32 radius, float rel_x, float rel_y, float r, float g, float b, float a);

    //draw colored circle coords 0;0 is left topov viewport 
    void glv_draw_circle(GlvMgr *mgr, Uint32 radius, int x, int y, float r, float g, float b, float a);

    //prints error using current error logger
    void glv_log_err(GlvMgr *mgr, const char *err);

    //returns sdl window for this mgr
    SDL_Window *glv_get_window(GlvMgr *mgr);

    //returns -1 if error
    //all font resources are managed by mgr 
    GlvFontFaceId glv_new_freetype_face(GlvMgr *mgr, const char *filepath, FT_Long face_index);

    //returns -1 if error
    //all font resources are managed by mgr 
    GlvFontFaceId glv_new_freetype_face_mem(GlvMgr *mgr, const void *data, FT_Long data_size, FT_Long face_index);

    //returns font for GlvFontFaceId in mgr
    //returns NULL if out of range
    FT_Face glv_get_freetype_face(GlvMgr *mgr, GlvFontFaceId id);

    //SDL_USEREVENT some events are reserved by SDL_RegisterEvents()
    void glv_set_sdl_event_handler(GlvMgr *mgr, void(*on_sdl_event)(View *root, const SDL_Event *event, void *root_context));

//call enum_proc for each first order child first order
void glv_enum_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);

//call enum_proc for each first order visible child
void glv_enum_visible_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);

//call enum_proc for each first order focused child
void glv_enum_focused_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data);

//call enum_proc for parent and his parent... 
void glv_enum_parents(View *view, void(*enum_proc)(View *parent, void *data), void *data);

//create child view
//manage_proc used to handle events
//user_context can be accessed in manage_proc
View *glv_create(View *parent, ViewProc view_proc, ViewManage manage_proc, void *user_context);

//deletes view and all childs
void glv_delete(View *view);

//default view procedure should be called in unhandled by view cases
void glv_proc_default(View *view, ViewMsg msg, void *in, void *out);

//run window with root view
int glv_run(ViewProc root_view_proc, ViewManage root_view_manage, void *root_user_data, void (*init_spa)(View *root_view, void *root_context));

//returns view data that have size, defined in VM_GET_VIEW_DATA_SIZE
void *glv_get_view_data(View *view, unsigned int offset);

//returns view data that have size, defined in VM_GET_VIEW_DATA_SIZE and shared for all same view in this mgr 
void *glv_get_view_singleton_data(View *view);

//saves txture to bitmap file
void glv_dump_texture(GlvMgr *mgr, const char *file, GLuint texture, Uint32 bmp_width, Uint32 bmp_height);

//handles in message queue
//copies args 
//handles by manage proc
void glv_push_event(View *view, ViewMsg message, void *args, uint32_t args_size);

//handles instantly
//doesnt handles by manage proc
void glv_call_event(View *view, ViewMsg message, void *in, void *out);

//handles only by manage proc
void glv_call_manage(View *view, ViewMsg message, void *event_args);

//returns view texture 
GLuint glv_get_texture(View *view);

//returns view framebuffer
//framebuffer is automaticaly bounds in VM_DRAW, but not in the other events
GLuint glv_get_framebuffer(View *view);

//returns documents for target message
//strings are never NULL
GlvMsgDocs glv_get_docs(View *view, ViewMsg message);

//print documents for target message
void glv_print_docs(View *view, ViewMsg message);

//defines document using passed values
void glv_write_docs(GlvMsgDocs *docs, ViewMsg message, 
    const char *name, const char *input_description, 
    const char *output_description, const char *general_description);

//set minimal frametime
//by default minimal frametime is equals to  1 / (monitor refresh rate) 
void glv_set_minimal_frametime(GlvMgr *mgr, Uint32 ms_min_frametime);

//set view position
void glv_set_pos(View *view, int x, int y);

//set view size
void glv_set_size(View *view, unsigned int width, unsigned int height);

//enables view texture drawing
//calls VM_DRAW message end refreshes window
//view should manually resize texture if it needed
void glv_draw(View *view);

//for disable view texture drawing
//childs will still drawing 
void glv_deny_draw(View *view);

//set view as visible
void glv_show(View *view);

//set view as invisible and unfocused and unhovered
void glv_hide(View *view);

//set focus for view and all parents and removes focus of all childs
void glv_set_focus(View *view);

//uses to handle recursion when sended by parent view while handling CHILD_SHOW event
void glv_show_by(View *sender, View *view);

//uses to handle recursion when sended by parent view while handling CHILD_HIDE event
void glv_hide_by(View *sender, View *view);

//uses to handle recursion when sended by parent view while handling CHILD_MOVE event
void glv_set_pos_by(View *sender, View *view, int x, int y);

//uses to handle recursion when sended by parent view while handling CHILD_RESIZE event
void glv_set_size_by(View *sender, View *view, unsigned int width, unsigned int height);

// returns current view position
SDL_Point glv_get_pos(View *view);

//returns current view size
SDL_Point glv_get_size(View *view);

//returns parent view
//return NULL if view is root
View *glv_get_Parent(View *view);

//returns is_focused status
bool glv_is_focused(View *view);

//returns is_visisble status
bool glv_is_visible(View *view);

//returns is_hovered status
bool glv_is_mouse_over(View *view);

//build shader program and log all error
//shader program is unmanaged by mgr
//mgr used only to log error
bool glv_build_program_or_quit_err(GlvMgr *mgr, const char *vertex, const char *fragment, GLuint *result);

//take focus witout unfocusing others
//nice to be used in VM_UNFOCUS message to views that required to capture all keyboard events
void glv_set_secondary_focus(View *view);

//remove only view focus and recursive childs
void glv_unset_secondary_focus(View *view);

//swap view texture with texture
//returns previous view texture
//previous texture is not more manage by mgr, resources should be disposed by glDeleteTextures() 
//makes view as drawable but VM_DRAW message will not sent
GLuint glv_swap_texture(View *view, GLuint texture);

//makes view as drawable but VM_DRAW message will not sent
//good way to handle background if view visualizes only background 
void glv_swap_texture_with_bg(View *view);

//set view backgound unsing texture, view take ownership over texture
//doesnt makes view drawable, view should manage this manually
void glv_set_background(View *view, GLuint texture);

//set view foreground unsing texture, view take ownership over texture
//doesnt makes view drawable, view should manage this manually
void glv_set_foreground(View *view, GLuint texture);

//set view font
//doesnt makes view drawable, view should manage this manually
void glv_set_font(View *view, GlvFontFaceId font_face);

//set view font with
//doesnt makes view drawable, view should manage this manually
void glv_set_font_width(View *view, Uint32 font_width);

//set view font height
//doesnt makes view drawable, view should manage this manually
void glv_set_font_height(View *view, Uint32 font_height);

//returns current bg texture
//doesnt makes view drawable, view should manage this manually
GLuint glv_get_bg_texture(View *view);

//returns current fg texture
//doesnt makes view drawable, view should manage this manually
GLuint glv_get_fg_texture(View *view);

GLuint glv_gen_texture_solid_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

//use glv_get_docs or glv_print_docs to see details
//inherited views should implement docs for own messages and redirect procedure for others  
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
    VM_CHILD_HIDE,
    VM_CHILD_SHOW,

    VM_MOUSE_HOVER,
    VM_MOUSE_LEAVE,

    VM_TEXT,
    VM_TEXT_EDITING,

    VM_SDL_REDIRECT,

    VM_SET_BG,
    VM_SET_FG,
    VM_SET_FONT,
    VM_SET_FONT_WIDTH,
    VM_SET_FONT_HEIGHT,

    VM_GET_DOCS,
    VM_GET_VIEW_DATA_SIZE,

    //has View *view == NULL
    VM_GET_SINGLETON_DATA_SIZE,
    //has View *view == NULL
    VM_SINGLETON_DATA_DELETE,

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

struct GlvEventChildChanged{
    View *child;
    View *sender;//can be NULL uses to handle recursion
};


#endif //GLV_H
