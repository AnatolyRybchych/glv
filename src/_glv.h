#ifndef _GLV_H
#define _GLV_H

#include <glv.h>

void log_printf(GlvMgr *mgr, const char *log);
bool should_redraw(GlvMgr *mgr);
void default_on_sdl_event(View *root, const SDL_Event *event, void *root_context);

extern const char *draw_texture_vert;
extern const char *draw_texture_frag;

typedef struct _MBEvEnumArgs{
    struct GlvEventMouseButton ev;
    ViewMsg message;
} _MBEvEnumArgs;

typedef struct _KeyEvEnumArgs{
    struct GlvEventKey ev;
    ViewMsg message;
} _KeyEvEnumArgs;

typedef struct GlvSdlEvent{
    Uint32 type;        /**< ::SDL_USEREVENT through ::SDL_LASTEVENT-1 */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The associated window if any */
    ViewMsg message;        /**< User defined event code */
    View *view;        /**< User defined data pointer */
    void *data;        /**< User defined data pointer */
} GlvSdlEvent;

typedef struct DrawTextureProgram{
    GLuint prog;
    GLuint vbo;
    GLuint coords_bo;

    GLuint vertex_p;
    GLuint tex_coords_p;
    
    GLuint tex_p;
    GLuint mvp_p;
    GLuint tex_mvp_p;
} DrawTextureProgram;

struct GlvMgr{
    bool is_running;
    int return_code;

    SDL_GLContext *gl_rc;
    SDL_Window *window;
    Uint32 wind_id;
    
    void (*logger_proc)(GlvMgr *mgr, const char *err);

    void (*on_sdl_event)(View *root, const SDL_Event *event, void *root_context);

    bool required_redraw;
    View *root_view;

    Uint32 min_frametime_ms;
    Uint32 last_frame_drawed_time_ms;

    DrawTextureProgram draw_texture_program;
};

struct View{
    struct GlvMgr *mgr;

    GLuint framebuffer;
    GLuint texture;

    int x;
    int y;
    unsigned int w;
    unsigned int h;

    bool is_drawable;
    bool is_visible;
    bool is_focused;

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

