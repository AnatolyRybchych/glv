#include "_glv.h"

static GLuint init_normal_vbo(void);
static GLuint init_normal_uv_bo(void);

static void glv_draw_polygon_sector_rel_(GlvMgr *mgr, float solid_p[2], float border_p1[2], float border_p2[2], float color[3], GLint viewport[4]);

DrawTriangleProgram init_draw_triangle(GlvMgr *mgr){
    DrawTriangleProgram result;

    if(glv_build_program_or_quit_err(
        mgr, draw_triangle_vert, draw_triangle_frag, &result.program
    ) == false){
        glv_log_err(mgr, "cannot initialize shader program for triangle drawing");
        glv_quit(mgr);
    }

    result.vbo_pos = glGetAttribLocation(result.program, "vertex_p");
    result.vertex_color_pos = glGetAttribLocation(result.program, "vertex_color");

    return result;
}   

void free_draw_triangle(DrawTriangleProgram *prog){
    glDeleteProgram(prog->program);
}

DrawCircleProgram init_draw_circle(GlvMgr *mgr){

    DrawCircleProgram result;
    if(glv_build_program_or_quit_err(
        mgr, draw_circle_vert, draw_circle_frag, &result.program
    ) == false){
        glv_log_err(mgr, "cannot initialize shader program for circle drawing");
        glv_quit(mgr);
    }


    result.vbo = init_normal_vbo();

    result.vbo_pos = glGetAttribLocation(result.program, "vbo");
    result.color_pos = glGetUniformLocation(result.program, "color");
    result.scale_pos = glGetUniformLocation(result.program, "scale");
    result.offset_pos = glGetUniformLocation(result.program, "offset");
    result.px_radius_pos = glGetUniformLocation(result.program, "px_radius");

    return result;
}

void free_draw_circle(DrawCircleProgram *prog){
    SDL_assert(prog != NULL);

    glDeleteBuffers(1, &prog->vbo);
    glDeleteProgram(prog->program);
}

DrawTextureProgram init_draw_texture(GlvMgr *mgr){
    DrawTextureProgram result;

    if(glv_build_program_or_quit_err(
        mgr, draw_texture_vert, draw_texture_frag, &result.prog
    ) == false){
        glv_log_err(mgr, "cannot initialize shader program for texture drawing");
        glv_quit(mgr);
    }

    result.vbo = init_normal_vbo();
    result.coords_bo = init_normal_uv_bo();

    result.vertex_p = glGetAttribLocation(result.prog, "vertex_p");
    result.tex_coords_p = glGetAttribLocation(result.prog, "tex_coords");

    result.tex_p = glGetUniformLocation(result.prog, "tex");
    result.mvp_p = glGetUniformLocation(result.prog, "mvp");
    result.tex_mvp_p = glGetUniformLocation(result.prog, "tex_mvp");
    return result;
}

void free_draw_texture(DrawTextureProgram *prog){
    
    if(prog->prog) glDeleteProgram(prog->prog);
    if(prog->vbo) glDeleteBuffers(1, &prog->vbo);
    if(prog->tex_coords_p) glDeleteBuffers(1, &prog->tex_coords_p);
}

static GLuint init_normal_vbo(void){
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float buf_data[] = {
        -1,1, 1,1, -1,-1,
        1,-1, 1,1, -1,-1
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

static GLuint init_normal_uv_bo(void){
    GLuint uv_bo;
    glGenBuffers(1, &uv_bo);

    glBindBuffer(GL_ARRAY_BUFFER, uv_bo);
    float buf_data[] = {
        0,1, 1,1, 0,0,
        1,0, 1,1, 0,0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return uv_bo;
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
    SDL_assert(mgr != NULL);

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
    SDL_assert(mgr != NULL);

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
    SDL_assert(mgr != NULL);

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
    SDL_assert(mgr != NULL);

    float tex_mvp[4*4];
    mvp_identity(tex_mvp);
    glv_draw_texture_mat2(mgr, texture, mvp, tex_mvp);
}

void glv_draw_texture_st(GlvMgr *mgr, GLuint texture, float scale[3], float translate[3], float angle){
    SDL_assert(mgr != NULL);

    float mvp[4*4];
    mvp_identity(mvp);
    mvp_translate(mvp, translate);
    mvp_rotate_x(mvp, angle);
    mvp_scale(mvp, scale);

    glv_draw_texture_mat(mgr, texture, mvp);
}

void glv_draw_texture_absolute(GlvMgr *mgr, GLuint texture, const SDL_Rect *src, const SDL_Rect *dst){
    SDL_assert(mgr != NULL);

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

void glv_draw_circle_rel(GlvMgr *mgr, Uint32 radius, float rel_x, float rel_y, float r, float g, float b, float a){
    SDL_assert(mgr != NULL);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glUseProgram(mgr->draw_circle_program.program);
    glEnableVertexAttribArray(mgr->draw_circle_program.vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, mgr->draw_circle_program.vbo);
    glVertexAttribPointer(mgr->draw_circle_program.vbo_pos, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glUniform4f(mgr->draw_circle_program.color_pos, r, g, b, a);
    glUniform1f(mgr->draw_circle_program.px_radius_pos, ((vp[2] + vp[3]) / (float)radius * 8.0) / (sqrt(vp[2] * vp[2] + vp[3] * vp[3])) );
    glUniform2f(mgr->draw_circle_program.scale_pos, radius / (float)vp[2], radius / (float)vp[3]);
    glUniform2f(mgr->draw_circle_program.offset_pos, rel_x, rel_y);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(mgr->draw_circle_program.vbo_pos);
    glUseProgram(0);
}

void glv_draw_circle(GlvMgr *mgr, Uint32 radius, int x, int y, float r, float g, float b, float a){
    SDL_assert(mgr != NULL);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glv_draw_circle_rel(mgr, radius, (x * 2) / (float)vp[2] - 1.0, (-y * 2) / (float)vp[3] + 1.0, r, g, b, a);
}

void glv_draw_triangles_rel(GlvMgr *mgr, Uint32 vertices_cnt, float *vertices, Uint32 per_vertex, float *colors, Uint32 per_color){
    SDL_assert(mgr != NULL);

    glUseProgram(mgr->draw_triangle_program.program);
    glEnableVertexAttribArray(mgr->draw_triangle_program.vbo_pos);
    glEnableVertexAttribArray(mgr->draw_triangle_program.vertex_color_pos);

    glVertexAttribPointer(mgr->draw_triangle_program.vbo_pos, per_vertex, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(mgr->draw_triangle_program.vertex_color_pos, per_color, GL_FLOAT, GL_FALSE, 0, colors);
    
    glDrawArrays(GL_TRIANGLES, 0, vertices_cnt);

    glDisableVertexAttribArray(mgr->draw_triangle_program.vbo_pos);
    glDisableVertexAttribArray(mgr->draw_triangle_program.vertex_color_pos);
    glUseProgram(0);
}

void glv_draw_triangle_rel(GlvMgr *mgr, float p1[2], float p2[2], float p3[2], float color[3]){
    float center[2];

    vec2_lerp(center, p1, p2, 0.5);
    vec2_eq_lerp(center, p3, 0.5);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glv_draw_polygon_sector_rel_(mgr, center, p1, p2, color, vp);
    glv_draw_polygon_sector_rel_(mgr, center, p1, p3, color, vp);
    glv_draw_polygon_sector_rel_(mgr, center, p2, p3, color, vp);
}

static void glv_draw_polygon_sector_rel_(GlvMgr *mgr, float solid_p[2], float border_p1[2], float border_p2[2], float color[3], GLint viewport[4]){
    float vertices[] = {
        solid_p[0], solid_p[1],
        border_p1[0], border_p1[1],
        border_p2[0], border_p2[1],
    };

    float p1_px[2] = {viewport[2] *border_p1[0], viewport[3] *border_p1[1],};
    float p2_px[2] = {viewport[2] *border_p2[0], viewport[3] *border_p2[1],};
    float solid_px[2] = {viewport[2] *solid_p[0], viewport[3] *solid_p[1],};

    float colors[] = {
        color[0], color[1], color[2], line_point_dst(p1_px, p2_px, solid_px) / 2.0f,
        color[0], color[1], color[2], 0.0,
        color[0], color[1], color[2], 0.0
    };

    glv_draw_triangles_rel(mgr, 3, vertices, 2, colors, 4);
}

void glv_draw_polygon_sector_rel(GlvMgr *mgr, float solid_p[2], float border_p1[2], float border_p2[2], float color[3]){
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glv_draw_polygon_sector_rel_(mgr, solid_p, border_p1, border_p2, color, vp);
}

static int __cmp_by_x_axis(const void *first, const void *second){
    return **(float**)first > **(float**)second ? 1 : -1;
}

static int __cmp_by_y_axis(const void *first, const void *second){
    return (*(float**)first)[1] > (*(float**)second)[1] ? 1 : -1;
}
    
void glv_draw_quadrangle_rel(GlvMgr *mgr, float p1[2], float p2[2], float p3[2], float p4[2], float color[3]){
    float *v[] = {p1, p2, p3, p4};
    qsort(v, 4, sizeof(float*), __cmp_by_x_axis); //[0,1] are left sides, [2, 3] are right
    qsort(v + 0, 2, sizeof(float*), __cmp_by_y_axis);//[1] are left top, [0] are left bottom
    qsort(v + 2, 2, sizeof(float*), __cmp_by_y_axis);//[3] are right top, [2] are right bottom

    float center[2];

    vec2_lerp(center, v[0], v[3], 0.5);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glv_draw_polygon_sector_rel(mgr, center, v[0], v[1], color);// triangle {center, left top, letf bottom}
    glv_draw_polygon_sector_rel(mgr, center, v[2], v[3], color);// triangle {center, right top, right bottom}
    glv_draw_polygon_sector_rel(mgr, center, v[1], v[3], color);// triangle {center, left top, right top}
    glv_draw_polygon_sector_rel(mgr, center, v[0], v[2], color);// triangle {center, left bottom, right bottom}
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

