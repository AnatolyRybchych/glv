#include "_glv.h"

bool should_redraw(GlvMgr *mgr){
    if(mgr->required_redraw == false) return false;

    Uint32 curr_ms = SDL_GetTicks();
    if(mgr->last_frame_drawed_time_ms < mgr->min_frametime_ms + curr_ms){
        mgr->required_redraw = false;
        mgr->last_frame_drawed_time_ms = curr_ms;
        return true;
    }
    else{
        return false;
    }
}

void default_on_sdl_event(View *root, const SDL_Event *event, void *root_context){
    root = root;//unused
    event = event;//unused
    root_context = root_context;//unused
}

void log_printf(GlvMgr *mgr, const char *log){
    mgr = mgr;
    fprintf(stderr, "\033[0;31mERROR:\"%s\"\033[0m\n", log);
}

void glv_set_error_logger(GlvMgr *mgr, void (*logger_proc)(GlvMgr *mgr, const char *err)){
    SDL_assert(mgr != NULL);

    if(logger_proc != NULL)mgr->logger_proc = logger_proc;
    else mgr->logger_proc = log_printf;
}

void glv_quit(GlvMgr *mgr){
    SDL_assert(mgr != NULL);

    mgr->is_running = false;
}

void glv_redraw_window(GlvMgr *mgr){
    SDL_Event ev;
    ev.type = SDL_WINDOWEVENT;

    SDL_WindowEvent *wev = &ev.window;

    wev->event = SDL_WINDOWEVENT_EXPOSED;
    wev->windowID = mgr->wind_id;
    wev->timestamp = SDL_GetTicks();
    wev->data1 = wev->data2 = 0;
    
    SDL_PushEvent(&ev);

    mgr->required_redraw = true;
}

void glv_draw_texture_mat2(GlvMgr *mgr, GLuint texture, float mvp[4*4], float tex_mvp[4*4]){

    DrawTextureProgram *p = &mgr->draw_texture_program;

    glUseProgram(p->prog);
    glEnableVertexAttribArray(p->vertex_p);
    glEnableVertexAttribArray(p->tex_coords_p);

    glBindBuffer(GL_ARRAY_BUFFER, p->vbo);
    glVertexAttribPointer(p->vertex_p, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, p->coords_bo);
    glVertexAttribPointer(p->tex_coords_p, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(p->tex_p, 0);

    glUniformMatrix4fv(p->mvp_p, 1, 0, mvp);
    glUniformMatrix4fv(p->tex_mvp_p, 1, 0, tex_mvp);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableVertexAttribArray(p->vertex_p);
    glDisableVertexAttribArray(p->tex_coords_p);
    glUseProgram(0);
}

void glv_draw_texture_mat(GlvMgr *mgr, GLuint texture, float mvp[4*4]){
    float tex_mvp[4*4];
    mvp_identity(tex_mvp);
    glv_draw_texture_mat2(mgr, texture, mvp, tex_mvp);
}

void glv_draw_texture_st(GlvMgr *mgr, GLuint texture, float scale[3], float translate[3], float angle){
    float mvp[4*4];
    mvp_identity(mvp);
    mvp_translate(mvp, translate);
    mvp_rotate_x(mvp, angle);
    mvp_scale(mvp, scale);

    glv_draw_texture_mat(mgr, texture, mvp);
}

void glv_draw_texture_absolute(GlvMgr *mgr, GLuint texture, const SDL_Rect *src, const SDL_Rect *dst){

    float mvp[4*4];
    float tex_mvp[4*4];

    float translation[3];
    float scale[3];

    SDL_Rect vp;

    glGetIntegerv(GL_VIEWPORT, (GLint*)&vp);

    mvp_identity(mvp);
    mvp_identity(tex_mvp);

    translation[0] = (dst->x*2 + dst->w - vp.w) / (float)vp.w;
    translation[1] = (vp.h -( dst->y*2 + dst->h)) / (float)vp.h;
    translation[2] = 0;

    scale[0] = dst->w / (float)vp.w;
    scale[1] = dst->h / (float)vp.h;
    scale[2] = 0;

    mvp_translate(mvp, translation); 
    mvp_scale(mvp, scale);

    translation[1] = - dst->h / (float)src->h;
    translation[0] = translation[2] = 0;
    

    scale[0] = dst->w / (float)src->w;
    scale[1] = dst->h / (float)src->h;
    scale[2] = 1;

    mvp_translate(tex_mvp, translation); 
    mvp_scale(tex_mvp, scale);

    glv_draw_texture_mat2(mgr, texture, mvp, tex_mvp);
}

SDL_Window *glv_get_window(GlvMgr *mgr){
    return mgr->window;
}

void glv_set_sdl_event_handler(GlvMgr *mgr, void(*on_sdl_event)(View *root, const SDL_Event *event, void *root_context)){
    SDL_assert(mgr != NULL);
    if(on_sdl_event == NULL){
        mgr->on_sdl_event = default_on_sdl_event;
    }
    else{
        mgr->on_sdl_event = on_sdl_event;
    }
}

void glv_log_err(GlvMgr *mgr, const char *err){
    if(err == NULL){
        err = "log(NULL)";
    }
    mgr->logger_proc(mgr, err);
}

