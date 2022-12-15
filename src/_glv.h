#ifndef _GLV_H
#define _GLV_H

#include <glv.h>
#include "builtin_shaders.h"

typedef struct SingletonData SingletonData; 
typedef struct _MBEvEnumArgs _MBEvEnumArgs;
typedef struct _KeyEvEnumArgs _KeyEvEnumArgs;
typedef struct GlvSdlEvent GlvSdlEvent;
typedef struct DrawTextureProgram DrawTextureProgram;
typedef struct DrawCircleProgram DrawCircleProgram;
typedef struct DrawTriangleProgram DrawTriangleProgram;
typedef struct DrawTextProgram DrawTextProgram;

void log_printf(GlvMgr *mgr, const char *log);
bool should_redraw(GlvMgr *mgr);
void default_on_sdl_event(View *root, const SDL_Event *event, void *root_context);

DrawTextureProgram init_draw_texture(GlvMgr *mgr);
void free_draw_texture(DrawTextureProgram *prog);

DrawCircleProgram init_draw_circle(GlvMgr *mgr);
void free_draw_circle(DrawCircleProgram *prog);

DrawTriangleProgram init_draw_triangle(GlvMgr *mgr);
void free_draw_triangle(DrawTriangleProgram *prog);

DrawTextProgram init_draw_text(GlvMgr *mgr);
void free_draw_text(DrawTextProgram *prog);

struct SingletonData{
    ViewProc proc;
};

struct _MBEvEnumArgs{
    struct GlvEventMouseButton ev;
    ViewMsg message;
};

struct _KeyEvEnumArgs{
    struct GlvEventKey ev;
    ViewMsg message;
};

struct GlvSdlEvent{
    Uint32 type;        /**< ::SDL_USEREVENT through ::SDL_LASTEVENT-1 */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The associated window if any */
    ViewMsg message;        /**< User defined event code */
    View *view;        /**< User defined data pointer */
    void *data;        /**< User defined data pointer */
};

struct DrawTextureProgram{
    GLuint prog;
    GLuint vbo;
    GLuint coords_bo;

    GLuint vertex_p;
    GLuint tex_coords_p;
    
    GLuint tex_p;
    GLuint mvp_p;
    GLuint tex_mvp_p;
};

struct DrawCircleProgram{
    GLuint program;
    GLuint vbo;

    GLuint vbo_pos;
    GLuint color_pos;
    GLuint offset_pos;
    GLuint scale_pos;
    GLuint px_radius_pos;
};

struct DrawTriangleProgram{
    GLuint program;

    GLuint vbo_pos;
    GLuint vertex_color_pos;
};

struct DrawTextProgram{
    GLuint glyph_texture;

    GLuint prog_render_glyph;
    GLuint loc_vertex_p;
    GLuint loc_tex_coords;
    GLuint loc_glyph_coords;
    GLint loc_tex;
    GLint loc_glyph_tex;
};

struct GlvMgr{
    bool is_running;
    int return_code;

    SDL_GLContext *gl_rc;
    SDL_Window *window;
    Uint32 wind_id;

    FT_Library ft_lib;

    Uint32 faces_cnt;
    FT_Face *faces;
    
    Uint32 view_singleton_data_cnt;
    SingletonData **view_singleton_data;

    void (*logger_proc)(GlvMgr *mgr, const char *err);

    void (*on_sdl_event)(View *root, const SDL_Event *event, void *root_context);

    bool required_redraw;
    View *root_view;

    Uint32 min_frametime_ms;
    Uint32 last_frame_drawed_time_ms;

    DrawTextureProgram draw_texture_program;
    DrawCircleProgram draw_circle_program;
    DrawTriangleProgram draw_triangle_program;
    DrawTextProgram draw_text_program;
};

struct View{
    struct GlvMgr *mgr;

    GLuint framebuffer;
    GLuint texture;
    GLuint bg_tex;
    GLuint fg_tex;

    void *singleton_data;

    int x;
    int y;
    unsigned int w;
    unsigned int h;

    bool is_drawable;
    bool is_redraw_queue;
    bool is_visible;
    bool is_focused;
    bool is_mouse_over;
    bool is_text_input;

    View *parent;

    unsigned int childs_cnt;
    View **childs;

    ViewProc view_proc;
    void *view_data;
    unsigned int view_data_size;

    ViewManage view_manage;
    void *user_context;
};


#endif //_GLV_H

