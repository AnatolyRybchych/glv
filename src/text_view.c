#include <glv/text_view.h>
#include "builtin_shaders.h"

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    Uint32 text_len;
    Uint32 text_width;
    wchar_t *text;

    Uint32 face_width;
    Uint32 face_height;
    GlvFontFaceId face;

    SDL_Point align;
} Data;

typedef struct Singleton{
    bool is_defined;

    bool is_definition_error;
    char *definition_error;

    GLuint glyph_texture;

    GLuint prog_render_glyph;
    GLuint loc_vertex_p;
    GLuint loc_tex_coords;
    GLuint loc_glyph_coords;
    GLint loc_tex;
    GLint loc_glyph_tex;
} Singleton;

static void view_proc(View *text_view, ViewMsg msg, void *in, void *out);
static void init_singleton(View *text_view);
static void finalize_singleton(Singleton *singleton);
static void init_data(View *text_view);
static void finalize_data(View *text_view);

static void init_singleton_size(Uint32 *size);
static void init_data_size(View *view, Uint32 *size);

static void set_text(View *text_view, const wchar_t *text);
static void append_text(View *text_view, const wchar_t *text);
static void get_text(View *text_view, const wchar_t **text);
static void get_text_params(View *text_view, GlvTextViewTextParams *params);
static void get_docs(View *text_view, const ViewMsg *msg, GlvMsgDocs *docs);
static void log_definition_error_if_exists(View *text_view);
static void apply_align(View *text_view, SDL_Point align);
static void resize(View *text_view, const SDL_Point *new_size);
static void normalize(View *text_view, bool move);
static void render(View *text_view);
static void render_text(FT_Face face, const wchar_t *text, SDL_Point pos, SDL_Point viewport_size, Singleton *singleton, GLuint fg);
static void init_glyph_vbo(float vbo[12], const FT_GlyphSlot glyph, const SDL_Point *glyph_pos, const SDL_Point *viewport_size);
static Uint32 calc_text_width(View *text_view, const wchar_t *text);
static SDL_Point get_text_pos(View *text_view);

static void apply_face(View *text_view, const GlvFontFaceId *faceid);
static void apply_font_width(View *text_view, const Uint32 *width);
static void apply_font_height(View *text_view, const Uint32 *height);

ViewProc glv_text_view_proc = view_proc;
static Uint32 data_offset;

static void view_proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_CREATE:
        init_singleton(view);
        log_definition_error_if_exists(view);
        init_data(view);
        glv_draw(view);
        break;
    case VM_SET_FG:{
        glv_draw(view);
    }break;
    case VM_SET_BG:{
        glv_draw(view);
    }break;
    case VM_SET_FONT:{
        apply_face(view, in);
        glv_draw(view);
    }break;
    case VM_SET_FONT_WIDTH:{
        apply_font_width(view, in);
        glv_draw(view);
    }break;
    case VM_SET_FONT_HEIGHT:{
        apply_font_height(view, in);
        glv_draw(view);
    }break;
    case VM_GET_VIEW_DATA_SIZE: 
        init_data_size(view, out);
        break;
    case VM_GET_SINGLETON_DATA_SIZE: 
        init_singleton_size(out);
        break;
    case VM_DELETE: 
        finalize_data(view);
        break;
    case VM_SINGLETON_DATA_DELETE: 
        finalize_singleton(in);
        break;
    case VM_RESIZE:{
        resize(view, in);
        glv_draw(view);
    } break;
    case VM_DRAW: 
        render(view);
        break;
    case VM_TEXT_VIEW_SET_TEXT:
        set_text(view, in);
        glv_draw(view);
        break;
    case VM_TEXT_VIEW_APPEND_TEXT:
        append_text(view, in);
        glv_draw(view);
        break;
    case VM_TEXT_VIEW_SET_TEXTALIGN:{
        apply_align(view, *(SDL_Point*)in);
        glv_draw(view);
    } break;
    case VM_TEXT_VIEW_NORMALIZE:{
        normalize(view, *(bool*)in);
    } break;
    case VM_TEXT_VIEW_GET_TEXT:
        get_text(view, out);
    break;
    case VM_TEXT_VIEW_GET_TEXT_PARAMS:
        get_text_params(view, out);
        break;
    case VM_GET_DOCS:
        get_docs(view, in, out);
        break;
    default:
        parent_proc(view, msg, in, out);
        break;
    }
}

void glv_text_view_set_text(View *text_view, const wchar_t *text){
    SDL_assert(text_view != NULL);
    SDL_assert(text != NULL);

    glv_push_event(text_view, VM_TEXT_VIEW_SET_TEXT, (void*)text, sizeof(wchar_t) * (wcslen(text) + 1));
}

void glv_text_view_append_text(View *text_view, const wchar_t *text){
    SDL_assert(text_view != NULL);
    SDL_assert(text != NULL);

    glv_push_event(text_view, VM_TEXT_VIEW_APPEND_TEXT, (void*)text, sizeof(wchar_t) * (wcslen(text) + 1));
}

const wchar_t *glv_text_view_get_text(View *text_view){
    SDL_assert(text_view != NULL);

    wchar_t **text = NULL;
    glv_call_event(text_view, VM_TEXT_VIEW_GET_TEXT, NULL, text);
    return *text;
}

GlvTextViewTextParams glv_text_view_get_text_params(View *text_view){
    SDL_assert(text_view != NULL);

    GlvTextViewTextParams result;
    glv_call_event(text_view, VM_TEXT_VIEW_GET_TEXT_PARAMS, NULL, &result);
    return result;
}

void glv_text_view_set_alignment(View *text_view, int x, int y){
    SDL_assert(text_view != NULL);

    SDL_Point p = {
        .x = x,
        .y = y,
    };
    glv_push_event(text_view, VM_TEXT_VIEW_SET_TEXTALIGN, &p, sizeof(p));
}

void glv_text_view_normalize(View *text_view, bool move){
    SDL_assert(text_view != NULL);

    glv_push_event(text_view, VM_TEXT_VIEW_NORMALIZE, &move, sizeof(move));
}

static void init_singleton(View *text_view){
    Singleton *singleton = glv_get_view_singleton_data(text_view);

    GlvMgr *mgr = glv_get_mgr(text_view);

    if(singleton->is_defined == false){

        glGenTextures(1, &singleton->glyph_texture);
        glBindTexture(GL_TEXTURE_2D, singleton->glyph_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glv_build_program_or_quit_err(
            mgr, draw_glyph_vert, draw_glyph_frag, &singleton->prog_render_glyph);

        singleton->loc_vertex_p = glGetAttribLocation(singleton->prog_render_glyph, "vertex_p");
        singleton->loc_tex_coords = glGetAttribLocation(singleton->prog_render_glyph, "tex_coords");
        singleton->loc_glyph_coords = glGetAttribLocation(singleton->prog_render_glyph, "glyph_coords");

        singleton->loc_tex = glGetUniformLocation(singleton->prog_render_glyph, "tex");
        singleton->loc_glyph_tex = glGetUniformLocation(singleton->prog_render_glyph, "glyph_tex");

        singleton->is_defined = true;
    }
}

static void finalize_singleton(Singleton *singleton){
    singleton->is_defined = false;

    if(singleton->is_definition_error){
        free(singleton->definition_error);
    }

    glDeleteTextures(1, &singleton->glyph_texture);
}

static void init_data(View *text_view){
    Data *data = glv_get_view_data(text_view, data_offset);

    data->text = malloc(2);
    data->text[0] = 0;

    data->face = 0;
    data->face_width = 0;
    data->face_height = 36;
}

static void finalize_data(View *text_view){
    Data *data = glv_get_view_data(text_view, data_offset);
    free(data->text);
}

static void init_singleton_size(Uint32 *size){
    *size = sizeof(Singleton);
}

static void init_data_size(View *view, Uint32 *size){
    parent_proc(view, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size += sizeof(Data);
}

static void set_text(View *text_view, const wchar_t *text){
    Data *data = glv_get_view_data(text_view, data_offset);

    if(text == NULL) text = L"";
    data->text_len = wcslen(text);
    Uint32 text_size = (data->text_len + 1) * sizeof(wchar_t);

    free(data->text);
    data->text = malloc(text_size);
    memcpy(data->text, text, text_size);

    data->text_width = calc_text_width(text_view, text);
}

static void append_text(View *text_view, const wchar_t *text){
    Data *data = glv_get_view_data(text_view, data_offset);

    if(text == NULL) text = L"";
    Uint32 append_len = wcslen(text);
    Uint32 append_size = (append_len + 1) * sizeof(wchar_t);

    data->text = realloc(
        data->text,
        data->text_len + append_len
    );
    memcpy(data->text + data->text_len, text, append_size);

    data->text_width = calc_text_width(text_view, text);
}

static void get_text(View *text_view, const wchar_t **text){
    Data *data = glv_get_view_data(text_view, data_offset);
    *text = data->text;
}

static void get_text_params(View *text_view, GlvTextViewTextParams *params){
    Data *data = glv_get_view_data(text_view, data_offset);

    *params = (GlvTextViewTextParams){
        .font_face_id = data->face,
        .text_len = data->text_len,
        .face_height = data->face_height,
        .face_width = data->face_width,
        .text_width = data->text_width
    };
}

static void get_docs(View *text_view, const ViewMsg *msg, GlvMsgDocs *docs){
    switch (*msg){
    case VM_TEXT_VIEW_SET_TEXT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_SET_TEXT),
            "const wchar_t *text", "NULL", "sets the text");
        break;
    case VM_TEXT_VIEW_APPEND_TEXT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_APPEND_TEXT),
            "const wchar_t *text", "NULL", "appends the text");
        break;
    case VM_TEXT_VIEW_GET_TEXT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_GET_TEXT),
            "NULL", "const wchar_t **text", "retuns text but doenst losts ownership");
        break;
    case VM_TEXT_VIEW_GET_TEXT_PARAMS:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_GET_TEXT_PARAMS),
            "NULL", "GlvTextViewTextParams *params", "retuns text params");
        break;
    case VM_TEXT_VIEW_SET_TEXTALIGN:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_SET_TEXTALIGN),
            "const SDL_Point *alignment", "NULL", "set text alignment: x -> horisonta,"
            " y -> vertical; if < 0 -> alignment begin, == 0 -> alignment center, > 0 -> alignment end");
        break;
    case VM_TEXT_VIEW_NORMALIZE:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_VIEW_NORMALIZE),
            "const bool *move", "NULL", "set view size equals text size if *move != false moves this view to save text absolute location");
        break;
    default:
        parent_proc(text_view, VM_GET_DOCS, (void*)msg, docs);
        break;
    }
}

static void log_definition_error_if_exists(View *text_view){
    Singleton *singleton = glv_get_view_singleton_data(text_view);
    GlvMgr *mgr = glv_get_mgr(text_view);

    if(singleton->is_definition_error){
        glv_log_err(mgr, "text_view: definitilion error");
        glv_log_err(mgr, singleton->definition_error);
    }
}

static void apply_align(View *text_view, SDL_Point align){
    Data *data = glv_get_view_data(text_view, data_offset);

    data->align = align;
}

static void resize(View *text_view, const SDL_Point *new_size){
    new_size = new_size;
    GLuint tex = glv_get_texture(text_view);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_size->x, new_size->y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void normalize(View *text_view, bool move){
    SDL_Point curr_pos = glv_get_pos(text_view);

    Data *data = glv_get_view_data(text_view, data_offset);

    SDL_Point text_pos = get_text_pos(text_view);

    glv_set_size(text_view, data->text_width, data->face_height);
    if(move == false) return;

    glv_set_pos(text_view, curr_pos.x + text_pos.x, curr_pos.y + text_pos.y);
}

static void render(View *text_view){
    GlvMgr *mgr = glv_get_mgr(text_view);

    SDL_Point size = glv_get_size(text_view);
    Singleton *singleton = glv_get_view_singleton_data(text_view);
    Data *data = glv_get_view_data(text_view, data_offset);

    GLuint fg_texture = glv_get_fg_texture(text_view);
    GLuint bg_texture = glv_get_bg_texture(text_view);

    FT_Face face = glv_get_freetype_face(mgr, 0);
    FT_Set_Pixel_Sizes(face, data->face_width, data->face_height);

    GLint curr_blend[4];
    glGetIntegerv(GL_BLEND_SRC_RGB, curr_blend);
    glGetIntegerv(GL_BLEND_DST_RGB, curr_blend + 1);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, curr_blend + 2);
    glGetIntegerv(GL_BLEND_DST_ALPHA, curr_blend + 3);
    
    glViewport(0, 0, size.x, size.y);

    if(bg_texture != 0){
        float mat[16];
        mvp_identity(mat);
        
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_SRC_ALPHA, GL_ZERO);  
        glv_draw_texture_mat(mgr, bg_texture, mat);
        
    }
    else{
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_DST_ALPHA);  
    

    SDL_Point text_pos = get_text_pos(text_view);

    render_text(face, data->text, text_pos, size, singleton, fg_texture);

    glBlendFuncSeparate(curr_blend[0], curr_blend[1], curr_blend[2], curr_blend[3]);  
}

static void render_text(FT_Face face, const wchar_t *text, SDL_Point pos, SDL_Point viewport_size, Singleton *singleton, GLuint fg_texture){
    static float glyph_coords[] = {
        0,0, 1,0, 0, 1,
        1,1, 1,0, 0, 1,};

    wchar_t curr;
    wchar_t prev = 0;
    FT_GlyphSlot glyph;
    float vertex_p[12];

    glUseProgram(singleton->prog_render_glyph);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fg_texture);
    glUniform1i(singleton->loc_tex, 0);

    while (*text){
        curr = *text;
        if(curr != prev){
            FT_Load_Char(face, curr, FT_LOAD_RENDER);
            glyph = face->glyph;
            init_glyph_vbo(vertex_p, glyph, &pos, &viewport_size);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, singleton->glyph_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, glyph->bitmap.width, glyph->bitmap.rows, 
                                        0, GL_ALPHA, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

            glUniform1i(singleton->loc_glyph_tex, 1);
        }

        glEnableVertexAttribArray(singleton->loc_glyph_coords);
        //glEnableVertexAttribArray(singleton->loc_tex_coords);
        glEnableVertexAttribArray(singleton->loc_vertex_p);

        glVertexAttribPointer(singleton->loc_glyph_coords, 2, GL_FLOAT, GL_FALSE, 0, glyph_coords);
        glVertexAttribPointer(singleton->loc_vertex_p, 2, GL_FLOAT, GL_FALSE, 0, vertex_p);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(singleton->loc_glyph_coords);
        //glDisableVertexAttribArray(singleton->loc_tex_coords);
        glDisableVertexAttribArray(singleton->loc_vertex_p);

        pos.x += glyph->metrics.horiAdvance / 64;
        if(pos.x > viewport_size.x) break;

        prev = curr;
        text++;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

static void init_glyph_vbo(float vbo[12], const FT_GlyphSlot glyph, const SDL_Point *glyph_pos, const SDL_Point *viewport_size){
    SDL_FRect g_vp = {
        .x = (glyph_pos->x + glyph->metrics.horiBearingX / 64) 
                / (float)viewport_size->x * 2.0 - 1.0, 
        
        .y = (viewport_size->y - (glyph_pos->y + (glyph->metrics.vertAdvance / 2 + glyph->metrics.height - glyph->metrics.horiBearingY) / 64)) 
                / (float)viewport_size->y * 2.0 - 1.0,
        
        .w = glyph->bitmap.width * 2
            / (float)viewport_size->x,
        
        .h = glyph->bitmap.rows * 2
            / (float)viewport_size->y
    };

    float result[] = {
        g_vp.x, g_vp.y + g_vp.h, g_vp.x + g_vp.w, g_vp.y + g_vp.h, g_vp.x, g_vp.y,
        g_vp.x + g_vp.w, g_vp.y, g_vp.x + g_vp.w, g_vp.y + g_vp.h, g_vp.x, g_vp.y,
    };

    memcpy(vbo, result, sizeof(result));
}

static Uint32 calc_text_width(View *text_view, const wchar_t *text){
    Data *data = glv_get_view_data(text_view, data_offset);
    FT_Face face = glv_get_freetype_face(glv_get_mgr(text_view), data->face);

    const wchar_t *text_ptr = text;
    wchar_t curr;

    int curr_x = 0;
    while ((curr = *text_ptr++) != 0){
        FT_Load_Char(face, curr, FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;
        curr_x += glyph->metrics.horiAdvance / 64;
    }
    
    return curr_x;
}

static SDL_Point get_text_pos(View *text_view){
    Data *data = glv_get_view_data(text_view, data_offset);
    SDL_Point size = glv_get_size(text_view);
    SDL_Point text_pos = {0, 0};

    if(data->align.x == 0) text_pos.x = (size.x - (int)data->text_width) / 2;
    else if(data->align.x > 0) text_pos.x = size.x - (int)data->text_width;

    if(data->align.y == 0) text_pos.y = (size.y - (int)data->face_height) / 2;
    else if(data->align.y > 0) text_pos.y = size.y - (int)data->face_height;

    return text_pos;
}

static void apply_face(View *text_view, const GlvFontFaceId *faceid){
    Data *data = glv_get_view_data(text_view, data_offset);

    data->face = *faceid;
}

static void apply_font_width(View *text_view, const Uint32 *width){
    Data *data = glv_get_view_data(text_view, data_offset);

    data->face_width = *width;
}

static void apply_font_height(View *text_view, const Uint32 *height){
    Data *data = glv_get_view_data(text_view, data_offset);

    data->face_height = *height;
}