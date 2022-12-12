#include "_glv.h"

static void init_draw_texture_program(GlvMgr *mgr);
static void init_draw_texture_vbo(GlvMgr *mgr);
static void init_draw_texture_coord_bo(GlvMgr *mgr);
static void init_draw_texture_var_locations(GlvMgr *mgr);

void init_draw_texture_ifninit(GlvMgr *mgr){
    if(mgr->draw_texture_program.prog == 0){
        init_draw_texture_program(mgr);
        init_draw_texture_vbo(mgr);
        init_draw_texture_coord_bo(mgr);
        init_draw_texture_var_locations(mgr);
    }
}

static void init_draw_texture_program(GlvMgr *mgr){
    if(glv_build_program_or_quit_err(
        mgr, draw_texture_vert, draw_texture_frag, &mgr->draw_texture_program.prog
    ) == false){
        glv_quit(mgr);
    }
}

static void init_draw_texture_vbo(GlvMgr *mgr){
    glGenBuffers(1, &mgr->draw_texture_program.vbo);
    GLuint vbo = mgr->draw_texture_program.vbo;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float buf_data[] = {
        -1,1, 1,1, -1,-1,
        1,-1, 1,1, -1,-1
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_draw_texture_coord_bo(GlvMgr *mgr){
    glGenBuffers(1, &mgr->draw_texture_program.coords_bo);
    GLuint coords_bo = mgr->draw_texture_program.coords_bo;

    glBindBuffer(GL_ARRAY_BUFFER, coords_bo);
    float buf_data[] = {
        0,1, 1,1, 0,0,
        1,0, 1,1, 0,0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_draw_texture_var_locations(GlvMgr *mgr){
    DrawTextureProgram *p = &mgr->draw_texture_program;

    p->vertex_p = glGetAttribLocation(p->prog, "vertex_p");
    p->tex_coords_p = glGetAttribLocation(p->prog, "tex_coords");

    p->tex_p = glGetUniformLocation(p->prog, "tex");
    p->mvp_p = glGetUniformLocation(p->prog, "mvp");
    p->tex_mvp_p = glGetUniformLocation(p->prog, "tex_mvp");
}

void free_draw_texture(GlvMgr *mgr){
    DrawTextureProgram *p = &mgr->draw_texture_program;
    
    if(p->prog) glDeleteProgram(p->prog);
    if(p->vbo) glDeleteBuffers(1, &p->vbo);
    if(p->tex_coords_p) glDeleteBuffers(1, &p->tex_coords_p);

    p->prog = 0;
    p->vbo = 0;
    p->tex_coords_p = 0;
}

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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniformMatrix4fv(p->mvp_p, 1, 0, mvp);
    glUniformMatrix4fv(p->tex_mvp_p, 1, 0, tex_mvp);

    glUniform1i(p->tex_p, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableVertexAttribArray(p->vertex_p);
    glDisableVertexAttribArray(p->tex_coords_p);
    glUseProgram(0);
}

void glv_dump_texture(GlvMgr *mgr, const char *file, GLuint texture, Uint32 bmp_width, Uint32 bmp_height){
    GLuint curr_fb;
    GLuint curr_texture;
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int*)&curr_fb);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (int*)&curr_texture);

    GLuint fb_texture;
    GLuint fb;

    glGenFramebuffers(1, &fb);
    glGenTextures(1, &fb);

    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp_width, bmp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture, 0);

    glViewport(0, 0, bmp_width, bmp_height);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        glv_log_err(mgr, "glv_dump_texture(): framebuffer inclompleate");
    }

    float mat[16];
    mvp_identity(mat);

    glv_draw_texture_mat(mgr, texture, mat);

    SDL_Color *pixels = malloc(sizeof(SDL_Color) * bmp_width * bmp_height);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels, bmp_width, bmp_height, 32, bmp_width * sizeof(SDL_Color), 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    
    glReadPixels(0, 0, bmp_width, bmp_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    SDL_SaveBMP(surface, file);

    SDL_FreeSurface(surface);
    free(pixels);

    glDeleteFramebuffers(1, &fb);
    glDeleteTextures(1, &fb_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, curr_fb);
    glBindTexture(GL_TEXTURE_2D, curr_texture);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
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

GlvFontFaceId glv_new_freetype_face(GlvMgr *mgr, const char *filepath, FT_Long face_index){
    SDL_assert(mgr != NULL);
    SDL_assert(filepath != NULL);

    FT_Face face;

    FT_Error err = FT_New_Face(mgr->ft_lib, filepath, face_index, &face);
    if(err != FT_Err_Ok){
        char err_buffer[256 + 1];
        snprintf(err_buffer, 256, "glv_new_freetype_face(): %s filepath : \"%s\"", "freetype2 cannot open face", filepath);
        glv_log_err(mgr, err_buffer);
        glv_log_err(mgr, FT_Error_String(err));
        return -1;
    }

    mgr->faces_cnt++;
    mgr->faces = realloc(mgr->faces, mgr->faces_cnt * sizeof(FT_Face));
    mgr->faces[mgr->faces_cnt - 1] = face;
    return mgr->faces_cnt - 1;
}

GlvFontFaceId glv_new_freetype_face_mem(GlvMgr *mgr, const void *data, FT_Long data_size, FT_Long face_index){
    SDL_assert(mgr != NULL);
    SDL_assert(data != NULL);

    FT_Face face;

    FT_Error err = FT_New_Memory_Face(mgr->ft_lib, data, data_size, face_index, &face);
    if(err != FT_Err_Ok){
        glv_log_err(mgr, "glv_new_freetype_face_mem(): freetype2 cannot open memmory face");
        glv_log_err(mgr, FT_Error_String(err));
        return -1;
    }

    mgr->faces_cnt++;
    mgr->faces = realloc(mgr->faces, mgr->faces_cnt * sizeof(FT_Face));
    mgr->faces[mgr->faces_cnt - 1] = face;
    return mgr->faces_cnt - 1;
}

FT_Face glv_get_freetype_face(GlvMgr *mgr, GlvFontFaceId id){
    SDL_assert(mgr != NULL);

    if(mgr->faces_cnt == 0){
        glv_log_err(mgr, "glv_get_freetype_face(): faces count is 0, returned NULL");
        return NULL;
    }

    if(id < 0){
        char err_buffer[256 + 1];
        snprintf(err_buffer, 256, "glv_get_freetype_face(): id is out of range, id=%i, returned NULL", id);
        glv_log_err(mgr, err_buffer);
        return NULL;
    }
    
    GlvFontFaceId id_to_return = id;

    if((Uint32)id >= mgr->faces_cnt){
        id_to_return = mgr->faces_cnt - 1;
        char err_buffer[256 + 1];
        snprintf(err_buffer, 256, "glv_get_freetype_face(): id is out of range, id=%i, returned font for id=%i", id, id_to_return);
        glv_log_err(mgr, err_buffer);
    }

    return mgr->faces[id_to_return];
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

