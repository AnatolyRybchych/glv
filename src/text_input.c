#include <glv/text_input.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    wchar_t text[GLV_TEXT_INPUT_TEXT_LEN_MAX];
    Uint32 text_len;

    //carete char index
    Uint32 carete;

    int text_pos[2];

    //carete postion in pixels reletive to text pos
    int carete_pos[2];

    Uint32 selection[2];
    //selection pos in pixels reletive to text pos
    int selection_pos;
    //selection size in pixels
    int selection_size;

    GlvFontFaceId face_id;
    Uint32 face_size[2];

    float carete_color[3];
    float selection_color[3];

    bool texture_resize_needed;

    bool is_mouse_down;
    Uint32 carete_on_down;
    Uint32 last_mouse_down;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void on_init_data_size(View *view, Uint32 *size);
static void on_create(View *view);
static void on_delete(View *view);
static void on_text(View *view, const char *text);
static void on_resize(View *view, const SDL_Point *new_size);
static void on_draw(View *view);
static void on_docs(View *view, ViewMsg *msg, GlvMsgDocs *docs); 
static void on_set_carete_pos(View *view, const Uint32 *carete_pos); 
static void on_key_down(View *view, const GlvKeyDown *key); 
static void on_set_slection(View *view, const Uint32 selection[2]); 
static void on_get_text(View *view, wchar_t **text); 
static void on_get_selection(View *view, Uint32 selection[2]); 
static void on_get_carete(View *view, Uint32 *cursor); 
static void on_set_carete_color(View  *view, const float color[3]); 
static void on_set_selection_color(View *view, const float color[3]); 
static void on_set_face(View *view, const GlvFontFaceId *face); 
static void on_set_face_height(View *view, const Uint32 *face_height); 
static void on_set_face_width(View *view, const Uint32 *face_width); 
static void on_set_text(View *view, const wchar_t *text); 
static void on_clear(View *view); 
static void on_mouse_down(View *view, const GlvMouseDown *e);
static void on_mouse_up(View *view, const GlvMouseUp *e);
static void on_mouse_move(View *view, const GlvMouseMove *e);
static void on_mouse_leave(View *view);
static void on_sdl(View *view, const SDL_Event *ev);

static void normalize_height(View *view);

static void resize_texture(View *view);
static void align_text(View *view);
static void draw_text(View *view);
static void draw_bg(View *view);
static void draw_carete(View *view);
static void draw_selection(View *view);
static Uint32 calc_text_width(View *view, const wchar_t *text, Uint32 text_len);
static Uint32 calc_index(View *view, int pos_x);
static void paste_text(View *view, const wchar_t *text);
static void backspace(View *view);
static void delete(View *view);
static void ctrl_backspace(View *view);
static void ctrl_delete(View *view);
static void k_left(View *view);
static void k_right(View *view);
static void ctrl_k_left(View *view);
static void ctrl_k_right(View *view);
static void ctrl_v(View *view);
static void ctrl_c(View *view);
static void ctrl_x(View *view);
static void ctrl_a(View *view);
static void esc(View *view);
static void delete_rng(View *view, Uint32 from, Uint32 cnt);
static void erse_selected(View *view);

static void instant_carete_pos(View *text_input, Uint32 carete_pos);
static void instant_selection(View *text_input, const Uint32 selection[2]);

static bool is_word_end(char ch);
static bool is_in_str(const char *str, char ch);

ViewProc glv_text_input_proc = proc; 
static Uint32 data_offset;

void glv_text_input_set_carete_pos(View *text_input, Uint32 carete_pos){
    SDL_assert(text_input != NULL);

    glv_push_event(text_input, VM_TEXT_INPUT_SET_CARETE_POS, &carete_pos, sizeof(carete_pos));
}

void glv_text_input_set_selection(View *text_input, Uint32 first, Uint32 count){
    SDL_assert(text_input != NULL);

    Uint32 args[] = {first, count};

    glv_push_event(text_input, VM_TEXT_INPUT_SET_CARETE_POS, args, sizeof(args));
}

const wchar_t *glv_text_input_get_text(View *text_input){
    SDL_assert(text_input != NULL);

    wchar_t *result = NULL;
    glv_call_event(text_input, VM_TEXT_INPUT_GET_TEXT, NULL, &result);
    return result;
}

void glv_text_input_get_selection(View *text_input, Uint32 selection[2]){
    SDL_assert(text_input != NULL);
    glv_call_event(text_input, VM_TEXT_INPUT_GET_SELECTION, NULL, selection);
}

Uint32 glv_text_input_get_carete(View *text_input){
    SDL_assert(text_input != NULL);
    Uint32 result = 0;
    glv_call_event(text_input, VM_TEXT_INPUT_GET_SELECTION, NULL, &result);
    return result;
}

void glv_text_input_set_carete_color(View *text_input, float r, float g, float b){
    SDL_assert(text_input != NULL);
    float rgb[3] = {r, g, b};
    glv_push_event(text_input, VM_TEXT_INPUT_SET_CARETE_COLOR, rgb, sizeof(rgb));
}

void glv_text_input_set_selection_color(View *text_input, float r, float g, float b){
    SDL_assert(text_input != NULL);
    float rgb[3] = {r, g, b};
    glv_push_event(text_input, VM_TEXT_INPUT_SET_SELECTION_COLOR, rgb, sizeof(rgb));
}

void glv_text_input_clear(View *text_input){
    SDL_assert(text_input != NULL);
    glv_push_event(text_input, VM_TEXT_INPUT_CLEAR, NULL, 0);
}

void glv_text_input_set_text(View *text_input, const wchar_t *text){
    SDL_assert(text_input != NULL);
    glv_push_event(text_input, VM_TEXT_INPUT_SET_TEXT, (void*)text, (wcslen(text) + 1) * sizeof(wchar_t));
}

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_SDL_REDIRECT:
        on_sdl(view, in);
        break;
    case VM_TEXT:
        on_text(view, in);
        break;
    case VM_MOUSE_DOWN:
        on_mouse_down(view, in);
        break;
    case VM_MOUSE_UP:
        on_mouse_up(view, in);
        break;
    case VM_MOUSE_MOVE:
        on_mouse_move(view, in);
        break;
    case VM_MOUSE_LEAVE:
        on_mouse_leave(view);
        break;
    case VM_KEY_DOWN:
        on_key_down(view, in);
        break;
    case VM_RESIZE:
        on_resize(view, in);
        break;
    case VM_DRAW:
        on_draw(view);
        break;
    case VM_CREATE:
        on_create(view);
        break;
    case VM_DELETE:
        on_delete(view);
        break;
    case VM_GET_VIEW_DATA_SIZE:
        on_init_data_size(view, out);
        break;
    case VM_TEXT_INPUT_SET_CARETE_POS:
        on_set_carete_pos(view, in);
        break;
    case VM_TEXT_INPUT_SET_SELECTION:
        on_set_slection(view, in);
        break;
    case VM_TEXT_INPUT_GET_TEXT:
        on_get_text(view, out);
        break;
    case VM_TEXT_INPUT_GET_SELECTION:
        on_get_selection(view, out);
        break;
    case VM_TEXT_INPUT_GET_CARETE:
        on_get_carete(view, out);
        break;
    case VM_TEXT_INPUT_SET_CARETE_COLOR:
        on_set_carete_color(view, in);
        break;
    case VM_TEXT_INPUT_SET_SELECTION_COLOR:
        on_set_selection_color(view, in);
        break;
    case VM_TEXT_INPUT_SET_TEXT:
        on_set_text(view, in);
        break;
    case VM_TEXT_INPUT_CLEAR:
        on_clear(view);
        break;
    case VM_SET_FONT:
        on_set_face(view, in);
        break;
    case VM_SET_FONT_HEIGHT:
        on_set_face_height(view, in);
        break;
    case VM_SET_FONT_WIDTH:
        on_set_face_width(view, in);
        break;
    case VM_GET_DOCS:
        on_docs(view, in, out);
        break;
    default:
        parent_proc(view, msg, in, out);
        break;
    }
}

static void on_init_data_size(View *view, Uint32 *size){
    parent_proc(view, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void on_create(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    data->text[0] = 0;
    data->face_size[1] = 48;
}

static void on_delete(View *view){
    view = view;//unused
}

static void on_text(View *view, const char *text){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[0] != data->selection[1]){
        erse_selected(view);
    }

    wchar_t *wtext = (wchar_t*)SDL_iconv_string("WCHAR_T", "UTF-8", text, strlen(text) + 1);

    paste_text(view, wtext);
    SDL_free(wtext);
}

static void on_resize(View *view, const SDL_Point *new_size){
    new_size = new_size;

    Data *data = glv_get_view_data(view, data_offset);
    data->texture_resize_needed = true;

    normalize_height(view);

    glv_draw(view);
}

static void on_draw(View *view){
    SDL_Point size = glv_get_size(view);
    glViewport(0, 0, size.x, size.y);

    align_text(view);
    resize_texture(view);
    draw_bg(view);
    draw_selection(view);
    draw_text(view);
    draw_carete(view);
}

static void on_docs(View  *view, ViewMsg *msg, GlvMsgDocs *docs){
    switch (*msg){
    case VM_TEXT_INPUT_SET_CARETE_POS:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_SET_CARETE_POS),
            "const Uint32 *carete_pos", "NULL", "set text carete position");
        break;
    case VM_TEXT_INPUT_SET_SELECTION:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_SET_CARETE_POS),
            "const Uint32 selection[2]", "NULL", "set text selection from selection[0], selection[1] elements");
        break;
    case VM_TEXT_INPUT_TEXT_CHANGED:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_TEXT_CHANGED),
            "NULL", "NULL", "invokes on any text changed");
        break;
    case VM_TEXT_INPUT_GET_TEXT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_GET_TEXT),
            "NULL", "wchar_t **text", "resturns current text");
        break;
    case VM_TEXT_INPUT_GET_SELECTION:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_GET_SELECTION),
            "NULL", "Uint32 selection[2]", "returns selection: [0] is start, [1] i count");
        break;
    case VM_TEXT_INPUT_GET_CARETE:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_GET_CARETE),
            "NULL", "Uint32 *carete", "returns carete position");
        break;
    case VM_TEXT_INPUT_SET_CARETE_COLOR:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_SET_CARETE_COLOR),
            "float color[3]", "NULL", "changes carete color");
        break;
    case VM_TEXT_INPUT_SET_SELECTION_COLOR:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_SET_SELECTION_COLOR),
            "float color[3]", "NULL", "changes selection color");
        break;
    case VM_TEXT_INPUT_CLEAR:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_CLEAR),
            "NULL", "NULL", "removes all text");
        break;
    case VM_TEXT_INPUT_SET_TEXT:
        glv_write_docs(docs, *msg, SDL_STRINGIFY_ARG(VM_TEXT_INPUT_CLEAR),
            "const wchar_t *text", "NULL", "removes all text and pase this instead");
        break;
    default:
        parent_proc(view, VM_GET_DOCS, msg, docs);
        break;
    }
}

static void on_set_carete_pos(View  *view, const Uint32 *carete_pos){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->text_len < *carete_pos){
        GlvMgr *mgr = glv_get_mgr(view);
        glv_log_err(mgr, "text input: carete is out of range");
        data->carete = data->text_len;
        data->carete_pos[0] = calc_text_width(view, data->text, data->carete);
        data->carete_pos[1] = 0;
        return;
    }

    data->carete = *carete_pos;
    data->carete_pos[0] = calc_text_width(view, data->text, data->carete);
    data->carete_pos[1] = 0;

    glv_draw(view);
}

static void on_key_down(View  *view, const GlvKeyDown *key){
    switch (key->sym.scancode){
    case SDL_SCANCODE_BACKSPACE:
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_backspace(view);
        }
        else{
            backspace(view);
        }
        break;
    case SDL_SCANCODE_LEFT:
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_k_left(view);
        }
        else{
            k_left(view);
        }
        break;
    case SDL_SCANCODE_RIGHT:
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_k_right(view);
        }
        else{
            k_right(view);
        }
        break;
    case SDL_SCANCODE_DELETE:
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_delete(view);
        }
        else{
            delete(view);
        }
        break;
    case SDL_SCANCODE_V:{
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_v(view);
        }
    } break;
    case SDL_SCANCODE_C:{
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_c(view);
        }
    } break;
    case SDL_SCANCODE_A:{
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_a(view);
        }
    } break;
    case SDL_SCANCODE_X:{
        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]){
            ctrl_x(view);
        }
    } break;
    case SDL_SCANCODE_ESCAPE:
        esc(view);
        break;
    default:
        break;
    }
}

static void on_set_slection(View  *view, const Uint32 selection[2]){
    Data *data = glv_get_view_data(view, data_offset);

    if(selection[1] == 0){
        data->selection[0] = data->selection[1] = 0;
        glv_draw(view);
        return;
    }

    Uint32 s[2] = {selection[0], selection[1]};
    if(s[1] > data->text_len - s[0]) s[1] = data->text_len - s[0];

    data->selection_pos = calc_text_width(view, data->text, s[0]);
    data->selection_size = calc_text_width(view, data->text + s[0], s[1]);

    data->selection[0] = s[0];
    data->selection[1] = s[1];

    glv_draw(view);
}

static void on_get_text(View  *view, wchar_t **text){
    Data *data = glv_get_view_data(view, data_offset);
    *text = data->text;
}

static void on_get_selection(View  *view, Uint32 selection[2]){
    Data *data = glv_get_view_data(view, data_offset);
    selection[0] = data->selection[0];
    selection[1] = data->selection[1];
}

static void on_get_carete(View  *view, Uint32 *cursor){
    Data *data = glv_get_view_data(view, data_offset);

    *cursor = data->carete;
}

static void on_set_carete_color(View  *view, const float color[3]){
    Data *data = glv_get_view_data(view, data_offset);
    data->carete_color[0] = color[0];
    data->carete_color[1] = color[1];
    data->carete_color[2] = color[2];

    glv_draw(view);
}

static void on_set_selection_color(View  *view, const float color[3]){
    Data *data = glv_get_view_data(view, data_offset);

    data->selection_color[0] = color[0];
    data->selection_color[1] = color[1];
    data->selection_color[2] = color[2];

    glv_draw(view);
}

static void on_set_face(View *view, const GlvFontFaceId *face){
    Data *data = glv_get_view_data(view, data_offset);
    data->face_id = *face;    

    normalize_height(view);

    glv_draw(view);
}

static void on_set_face_height(View *view, const Uint32 *face_height){
    Data *data = glv_get_view_data(view, data_offset);
    data->face_size[1] = *face_height;
    
    normalize_height(view);

    glv_draw(view);
}

static void on_set_face_width(View *view, const Uint32 *face_width){
    Data *data = glv_get_view_data(view, data_offset);

    data->face_size[0] = *face_width;
    glv_draw(view);
}

static void on_set_text(View *view, const wchar_t *text){
    ctrl_a(view);
    delete(view);
    paste_text(view, text);
} 

static void on_clear(View *view){
    ctrl_a(view);
    delete(view);
}

static void on_mouse_down(View *view, const GlvMouseDown *e){
    Data *data = glv_get_view_data(view, data_offset);

    data->is_mouse_down = true;

    Uint32 time = SDL_GetTicks();
    Uint32 index = calc_index(view, e->x);

    
    data->carete_on_down = index;

    if(data->last_mouse_down + 500 > time && data->selection[1] == 0){

        Uint32 selection[2] = {index, 0};
        while (selection[0] > 0){
            if(!is_word_end(data->text[selection[0]])){
                selection[0]--;
            }
            else{
                selection[0]++;
                break;
            }
        }
        selection[1] += index - selection[0];

        while (selection[0] + selection[1] < data->text_len 
        && !is_word_end(data->text[selection[0] + selection[1]])){
            selection[1]++;
        }
        
        instant_selection(view, selection);
    }
    else{
        instant_selection(view, (Uint32[2]){0, 0});
        instant_carete_pos(view, index);
    }

    data->last_mouse_down = time;
}

static void on_mouse_up(View *view, const GlvMouseUp *e){
    Data *data = glv_get_view_data(view, data_offset);
    data->is_mouse_down = false;

    e = e;
}

static void on_mouse_move(View *view, const GlvMouseMove *e){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->is_mouse_down){
        data->carete_on_down = SDL_min(data->carete_on_down, data->text_len);

        Uint32 index = calc_index(view, e->x);

        Uint32 selection[2];
        selection[0] = SDL_min(data->carete_on_down, index);
        selection[1] = SDL_max(data->carete_on_down, index) - selection[0];
        
        instant_selection(view, selection);
        
        instant_carete_pos(view, index);
    }
}

static void on_mouse_leave(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    data->is_mouse_down = false;
}

static void on_sdl(View *view, const SDL_Event *ev){
    if(ev->type == SDL_DROPTEXT
    || ev->type == SDL_DROPFILE){
        const SDL_DropEvent *drop = &ev->drop;

        SDL_Point size = glv_get_size(view);
        SDL_Point mouse_pos;
        SDL_GetGlobalMouseState(&mouse_pos.x, &mouse_pos.y);
        mouse_pos = glv_desktop_to_window(glv_get_mgr(view), glv_window_to_view(view, mouse_pos));

        if(mouse_pos.x >= 0
        && mouse_pos.y >= 0
        && mouse_pos.x < size.x
        && mouse_pos.y < size.y){
            Uint32 text_len = strlen(drop->file);
            wchar_t *wtext = malloc((text_len + 1) * sizeof(wchar_t));
            for (Uint32 i = 0; i < text_len; i++){
                wtext[i] = drop->file[i];
            }
            wtext[text_len] = 0;
            on_set_text(view, wtext);

            free(wtext);
        }
    }
}

static void normalize_height(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    FT_Face face = glv_get_freetype_face(glv_get_mgr(view), data->face_id);
    FT_Set_Pixel_Sizes(face, data->face_size[0], data->face_size[1]);
    FT_Load_Char(face, L'\0', FT_LOAD_RENDER);

    SDL_Point size = glv_get_size(view);
    glv_set_size(view, size.x, face->glyph->metrics.vertAdvance / 64);
}

static void resize_texture(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->texture_resize_needed){
        GLuint texture = glv_get_texture(view);
        SDL_Point size = glv_get_size(view);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);

        data->texture_resize_needed = false;
    }
}

static void align_text(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    Uint32 text_width = calc_text_width(view, data->text, data->carete);
    Uint32 view_width = glv_get_size(view).x;

    if(text_width + data->face_size[1] > view_width){
        data->text_pos[0] = view_width - text_width - data->face_size[1];
    }
    else{
        data->text_pos[0] = 0;
    }
}

static void draw_text(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    FT_Face face = glv_get_freetype_face(mgr, data->face_id);
    FT_Set_Pixel_Sizes(face, data->face_size[0], data->face_size[1]);

    glv_draw_text(mgr, face, data->text, data->text_pos, glv_get_fg_texture(view));
}

static void draw_bg(View *view){
    GlvMgr *mgr = glv_get_mgr(view);
    GLuint bg = glv_get_bg_texture(view);
    GLuint fg = glv_get_fg_texture(view);

    float mat[4*4];
        mvp_identity(mat);
    if(bg == 0){
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
        glv_draw_texture_mat(mgr, fg, mat);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else{
        glv_draw_texture_mat(mgr, bg, mat);
    }
}

static void draw_carete(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    SDL_Point size = glv_get_size(view);
    int *_size = (int*)&size;

    int carete_lt_px[2] = {
        [0] = data->text_pos[0] + data->carete_pos[0] - (size.y / 32 + 1),
        [1] = data->text_pos[1] + data->carete_pos[1],
    };

    int carete_rb_px[2] = {
        [0] = carete_lt_px[0] + (size.y / 16 + 1),
        [1] = carete_lt_px[1] + size.y,
    };

    float carete_lt[2];
    float carete_rb[2];

    coords_rel_sz(carete_lt, carete_lt_px, _size);
    coords_rel_sz(carete_rb, carete_rb_px, _size);

    float vertices[] = {
        carete_lt[0], carete_lt[1],
        carete_rb[0], carete_lt[1],
        carete_rb[0], carete_rb[1],

        carete_lt[0], carete_lt[1],
        carete_lt[0], carete_rb[1],
        carete_rb[0], carete_rb[1],
    };

    float colors[] = {
        data->carete_color[0], data->carete_color[1], data->carete_color[2],
        data->carete_color[0], data->carete_color[1], data->carete_color[2],
        data->carete_color[0], data->carete_color[1], data->carete_color[2],

        data->carete_color[0], data->carete_color[1], data->carete_color[2],
        data->carete_color[0], data->carete_color[1], data->carete_color[2],
        data->carete_color[0], data->carete_color[1], data->carete_color[2],
    };

    glv_draw_triangles_rel(mgr, 6, vertices, 2, colors, 3);
}

static void draw_selection(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1] == 0) return;
    GlvMgr *mgr = glv_get_mgr(view);

    SDL_Point size = glv_get_size(view);
    int *_size = (int*)&size;

    int selection_lt_px[2] = {
        [0] = data->text_pos[0] + data->selection_pos,
        [1] = data->text_pos[1],
    };

    int selection_rb_px[2] = {
        [0] = selection_lt_px[0] + data->selection_size,
        [1] = selection_lt_px[1] + size.y,
    };

    float selection_lt[2];
    float selection_rb[2];

    coords_rel_sz(selection_lt, selection_lt_px, _size);
    coords_rel_sz(selection_rb, selection_rb_px, _size);

    float vertices[] = {
        selection_lt[0], selection_lt[1],
        selection_rb[0], selection_lt[1],
        selection_rb[0], selection_rb[1],

        selection_lt[0], selection_lt[1],
        selection_lt[0], selection_rb[1],
        selection_rb[0], selection_rb[1],
    };

    float colors[] = {
        data->selection_color[0], data->selection_color[1], data->selection_color[2],
        data->selection_color[0], data->selection_color[1], data->selection_color[2],
        data->selection_color[0], data->selection_color[1], data->selection_color[2],

        data->selection_color[0], data->selection_color[1], data->selection_color[2],
        data->selection_color[0], data->selection_color[1], data->selection_color[2],
        data->selection_color[0], data->selection_color[1], data->selection_color[2],
    };

    glv_draw_triangles_rel(mgr, 6, vertices, 2, colors, 3);
}

static Uint32 calc_text_width(View *view, const wchar_t *text, Uint32 text_len){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    FT_Face face = glv_get_freetype_face(mgr, data->face_id);
    FT_Set_Pixel_Sizes(face, data->face_size[0], data->face_size[1]);

    return glv_calc_text_width_n(face, text, text_len);
}

static Uint32 calc_index(View *view, int pos_x){
    if(pos_x <= 0) return 0;

    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    FT_Face face = glv_get_freetype_face(mgr, data->face_id);
    FT_Set_Pixel_Sizes(face, data->face_size[0], data->face_size[1]);

    pos_x -= data->text_pos[0];
    wchar_t *text_ptr = data->text;
    while (*text_ptr){
        FT_Load_Char(face, *text_ptr, FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;
        pos_x -= glyph->metrics.horiAdvance / 64;
        if(pos_x <= 0){
            if(pos_x + glyph->metrics.horiAdvance / 128 <= 0){
                return text_ptr - data->text;
            }
            else{
                return text_ptr - data->text + 1;
            }
        }
        text_ptr++;
    }
    return data->text_len;
}

static void paste_text(View *view, const wchar_t *text){
    Data *data = glv_get_view_data(view, data_offset);

    Uint32 len_paste = wcslen(text);
    if(data->text_len + len_paste >= GLV_TEXT_INPUT_TEXT_LEN_MAX ){
        glv_log_err(glv_get_mgr(view), "text input: text are bigger than " SDL_STRINGIFY_ARG(GLV_TEXT_INPUT_TEXT_LEN_MAX));
        len_paste = GLV_TEXT_INPUT_TEXT_LEN_MAX -  data->text_len - 1;
    }

    memmove(data->text + data->carete + len_paste, 
        data->text + data->carete,
        (data->text_len - data->carete) * sizeof(wchar_t));
    
    memcpy(data->text + data->carete, 
        text, 
        len_paste * sizeof(wchar_t));

    data->text_len += len_paste;
    data->text[data->text_len] = 0;

    instant_carete_pos(view, data->carete + len_paste);
    glv_push_event(view, VM_TEXT_INPUT_TEXT_CHANGED, NULL, 0);
}

static void backspace(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        erse_selected(view);
    }
    else if(data->carete == 0){
        return;
    }
    else{
        delete_rng(view, data->carete - 1, 1);
        instant_carete_pos(view, data->carete - 1);
    }
}

static void delete(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        erse_selected(view);
    }
    else{
        delete_rng(view, data->carete, 1);
    }
}

static void ctrl_backspace(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        erse_selected(view);
        return;
    }
    if(data->carete == 0){
        return;
    }
    
    
    int remove_cnt = 1;
    while (data->carete - remove_cnt > 0 && !is_word_end(data->text[data->carete - remove_cnt - 1])){
        remove_cnt++;
    }
    delete_rng(view, data->carete - remove_cnt, remove_cnt);
    instant_carete_pos(view, data->carete - remove_cnt);
}

static void ctrl_delete(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        erse_selected(view);
        return;
    }

    delete_rng(view, data->carete, 1);
    while (data->carete < data->text_len && !is_word_end(data->text[data->carete])){
        delete_rng(view, data->carete, 1);
    }

    int remove_cnt = 1;
    while (data->carete + remove_cnt < data->text_len && !is_word_end(data->text[data->carete + remove_cnt])){
        remove_cnt++;
    }
    delete_rng(view, data->carete, remove_cnt);
}

static void k_left(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->carete == 0){
        if(!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT]){
            Uint32 selection[2] = {0, 0};
            instant_selection(view, selection);
        }
        return;
    }

    if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT]){
        Uint32 selection[2] = {data->selection[0], data->selection[1]};

        if(selection[1] == 0) selection[0] = data->carete;

        if(selection[0] == data->carete){
            selection[0]--;
            selection[1]++;
        }
        else if(selection[0] + selection[1] == data->carete){
            selection[1]--;
        }
        else{
            selection[1] = 0;
        }
        instant_selection(view, selection);
        instant_carete_pos(view, data->carete - 1);
    }
    else{
        if(data->selection[1]){
            instant_carete_pos(view, data->selection[0]);
        }
        else{
            instant_carete_pos(view, data->carete - 1);
        }

        Uint32 selection[2] = {0, 0};
        instant_selection(view, selection);
    }
}

static void k_right(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->carete + 1 > data->text_len){
        if(!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT]){
            Uint32 selection[2] = {0, 0};
            instant_selection(view, selection);
        }
        return;
    }

    if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT]){
        Uint32 selection[2] = {data->selection[0], data->selection[1]};

        if(selection[1] == 0) selection[0] = data->carete;
        

        if(selection[0] + selection[1] == data->carete){
            selection[1]++;
        }
        else if(selection[0] == data->carete){
            selection[0]++;
            selection[1]--;
        }
        else{
            selection[1] = 0;
        }
        instant_selection(view, selection);
        instant_carete_pos(view, data->carete + 1);
    }
    else{
        if(data->selection[1]){
            instant_carete_pos(view, data->selection[0] + data->selection[1]);
        }
        else{
            instant_carete_pos(view, data->carete + 1);
        }
        Uint32 selection[2] = {0, 0};
        instant_selection(view, selection);
    }
}

static void ctrl_k_left(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    k_left(view);
    while (data->carete > 0 && !is_word_end(data->text[data->carete - 1])){
        k_left(view);
    }
}

static void ctrl_k_right(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    k_right(view);
    while (data->carete < data->text_len && !is_word_end(data->text[data->carete])){
        k_right(view);
    }
}

static void ctrl_v(View *view){
    if(SDL_HasClipboardText()){
        Data *data = glv_get_view_data(view, data_offset);

        if(data->selection[1]){
            erse_selected(view);
        }

        char *text = SDL_GetClipboardText();
        wchar_t *wtext = (wchar_t*)SDL_iconv_string("WCHAR_T", "UTF-8", text, strlen(text) + 1);

        paste_text(view, wtext);

        SDL_free(text);
        SDL_free(wtext);
    }
}

static void ctrl_c(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        char *text = SDL_iconv_string("UTF-8", "WCHAR_T", 
            (char*)(data->text + data->selection[0]), 
            (data->selection[1] + 1) * sizeof(wchar_t));

        SDL_SetClipboardText(text);

        SDL_free(text);
    }
    else{
        ctrl_a(view);
        ctrl_c(view);
    }
}

static void ctrl_x(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        ctrl_c(view);
        delete(view);
    }
}

static void ctrl_a(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    Uint32 selection[2] = {0, data->text_len};
    instant_selection(view, selection);
}

static void esc(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        Uint32 selection[2] = {0, 0};
        instant_selection(view, selection);
    }
}

static void delete_rng(View *view, Uint32 from, Uint32 cnt){
    Data *data = glv_get_view_data(view, data_offset);
    
    if(from >= data->text_len) return;
    if(from + cnt > data->text_len) cnt = data->text_len - from;

    Uint32 new_len = data->text_len - cnt;

    memmove(data->text + from, 
        data->text + from + cnt,
        (data->text_len - from - cnt) * sizeof(wchar_t));
    data->text[new_len] = 0;
    data->text_len = new_len;

    glv_push_event(view, VM_TEXT_INPUT_TEXT_CHANGED, NULL, 0);
    align_text(view);
    glv_draw(view);
}

static void erse_selected(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    if(data->selection[1] == 0){
        return;
    }
    else if(data->selection[0] >= data->text_len){
        glv_log_err(mgr, "text input: erse text out of range");

        Uint32 selection[2] = {0, 0};
        instant_selection(view, selection);
        return;
    }

    delete_rng(view, data->selection[0], data->selection[1]);
    instant_carete_pos(view, data->selection[0]);
    Uint32 selection[2] = {0, 0};
    instant_selection(view, selection);
}

static void instant_carete_pos(View *text_input, Uint32 carete_pos){
    glv_call_event(text_input, VM_TEXT_INPUT_SET_CARETE_POS, &carete_pos, NULL);
    glv_call_manage(text_input, VM_TEXT_INPUT_SET_CARETE_POS, &carete_pos);
}

static void instant_selection(View *text_input, const Uint32 selection[2]){
    glv_call_event(text_input, VM_TEXT_INPUT_SET_SELECTION, (void*)selection, NULL);
    glv_call_manage(text_input, VM_TEXT_INPUT_SET_SELECTION, (void*)selection);
}

static bool is_word_end(char ch){
    return is_in_str(
        " \t\n\r\v\f"
        "(){}[]<>"
        ".,:;!?"
        "-+=*/"
        "\"\'"
        "@#$%^&", ch); 
}

static bool is_in_str(const char *str, char ch){
    while (*str){
        if(*str == ch){
            return true;
        }
        str++;
    }
    return false;
}