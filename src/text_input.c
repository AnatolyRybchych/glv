#include <glv/text_input.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    TODO: utf-8 to unicode converting
    all characters just mapped as ascii to unicode
*/

typedef struct Data{
    wchar_t *text;
    Uint32 text_len;

    //carete char index
    Uint32 carete;

    //text pos in pixels
    int text_pos[2];

    //carete postion in pixels reletive to text pos
    int carete_pos[2];

    //carete size in pixels
    int carete_size[2];

    Uint32 selection[2];
    //selection pos in pixels reletive to text pos
    int selection_pos[2];
    //selection size in pixels
    int selection_size[2];

    GlvFontFaceId face_id;
    Uint32 face_size[2];

    bool texture_resize_needed;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void on_init_data_size(View *view, Uint32 *size);
static void on_create(View *view);
static void on_delete(View *view);
static void on_text(View *view, const char *text);
static void on_resize(View *view, const SDL_Point *new_size);
static void on_draw(View *view);
static void on_docs(View  *view, ViewMsg *msg, GlvMsgDocs *docs); 
static void on_set_carete_pos(View  *view, const Uint32 *carete_pos); 
static void on_key_down(View  *view, const GlvKeyDown *key); 
static void on_set_slection(View  *view, const Uint32 selection[2]); 

static void resize_texture(View *view);
static void draw_text(View *view);
static void draw_bg(View *view);
static void draw_carete(View *view);
static void draw_selection(View *view);
static Uint32 calc_text_width(View *view, const wchar_t *text, Uint32 text_len);
static void paste_text(View *view, const wchar_t *text);
static void backspace(View *view);
static void delete(View *view);
static void k_left(View *view);
static void k_right(View *view);
static void ctrl_v(View *view);
static void ctrl_c(View *view);
static void ctrl_x(View *view);
static void esc(View *view);
static void delete_rng(View *view, Uint32 from, Uint32 cnt);
static void erse_selected(View *view);

static void instant_carete_pos(View *text_input, Uint32 carete_pos);
static void instant_selection(View *text_input, const Uint32 selection[2]);

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

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_TEXT:
        on_text(view, in);
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

    data->text = malloc(2);
    data->text[0] = 0;

    data->carete_size[0] = 2;
    data->carete_size[1] = 48;

    data->selection_size[1] = 48;
    data->selection_pos[1] = 0;

    data->face_size[1] = 48;
}

static void on_delete(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    
    free(data->text);
}

static void on_text(View *view, const char *text){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[0] != data->selection[1]){
        erse_selected(view);
    }

    wchar_t ch[] = {
        *text,
        0
    };

    paste_text(view, ch);
}

static void on_resize(View *view, const SDL_Point *new_size){
    new_size = new_size;

    Data *data = glv_get_view_data(view, data_offset);
    data->texture_resize_needed = true;

    glv_draw(view);
}

static void on_draw(View *view){
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
        backspace(view);
        break;
    case SDL_SCANCODE_LEFT:
        k_left(view);
        break;
    case SDL_SCANCODE_RIGHT:
        k_right(view);
        break;
    case SDL_SCANCODE_DELETE:
        delete(view);
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
        data->selection_pos[0] = data->selection_pos[1] = 0;
        glv_draw(view);
        return;
    }

    Uint32 s[2] = {selection[0], selection[1]};
    if(s[1] > data->text_len - s[0]) s[1] = data->text_len - s[0];

    data->selection_pos[0] = calc_text_width(view, data->text, s[0]);
    data->selection_size[0] = calc_text_width(view, data->text + s[0], s[1]);

    data->selection[0] = s[0];
    data->selection[1] = s[1];

    glv_draw(view);
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

    float mat[4*4];
    mvp_identity(mat);

    glv_draw_texture_mat(mgr, bg, mat);
}

static void draw_carete(View *view){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    SDL_Point size = glv_get_size(view);
    int *_size = (int*)&size;

    int carete_lt_px[2] = {
        [0] = data->carete_pos[0] + data->text_pos[0],
        [1] = data->carete_pos[1] + data->text_pos[1],
    };

    int carete_rb_px[2] = {
        [0] = carete_lt_px[0] + data->carete_size[0],
        [1] = carete_lt_px[1] + data->carete_size[1],
    };

    float carete_lt[2];
    float carete_rb[2];

    coords_rel_sz(carete_lt, carete_lt_px, _size);
    coords_rel_sz(carete_rb, carete_rb_px, _size);

    glv_draw_quadrangle_rel(mgr, 
        carete_lt,
        carete_rb,
        (float[2]){carete_lt[0], carete_rb[1]},
        (float[2]){carete_rb[0], carete_lt[1]},
        
        (float[3]){0.6, 0.4, 0.3}
    );
}

static void draw_selection(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[0] == data->selection[1]) return;
    GlvMgr *mgr = glv_get_mgr(view);

    SDL_Point size = glv_get_size(view);
    int *_size = (int*)&size;

    int selection_lt_px[2] = {
        [0] = data->selection_pos[0] + data->text_pos[0],
        [1] = data->selection_pos[1] + data->text_pos[1],
    };

    int selection_rb_px[2] = {
        [0] = selection_lt_px[0] + data->selection_size[0],
        [1] = selection_lt_px[1] + data->selection_size[1],
    };

    float selection_lt[2];
    float selection_rb[2];

    coords_rel_sz(selection_lt, selection_lt_px, _size);
    coords_rel_sz(selection_rb, selection_rb_px, _size);

    glv_draw_quadrangle_rel(mgr, 
        selection_lt,
        selection_rb,
        (float[2]){selection_lt[0], selection_rb[1]},
        (float[2]){selection_rb[0], selection_lt[1]},
        
        (float[3]){0.0, 0.0, 0.0}
    );

}

static Uint32 calc_text_width(View *view, const wchar_t *text, Uint32 text_len){
    Data *data = glv_get_view_data(view, data_offset);
    GlvMgr *mgr = glv_get_mgr(view);

    FT_Face face = glv_get_freetype_face(mgr, data->face_id);
    FT_Set_Pixel_Sizes(face, data->face_size[0], data->face_size[1]);

    return glv_calc_text_width_n(face, text, text_len);
}

static void paste_text(View *view, const wchar_t *text){
    Data *data = glv_get_view_data(view, data_offset);

    Uint32 len_paste = wcslen(text);

    wchar_t *new_text = malloc(
        (data->text_len + len_paste + 1) * sizeof(wchar_t));

    wcsncpy(new_text, data->text, data->carete);
    wcsncpy(new_text + data->carete, text, len_paste);
    wcsncpy(new_text + data->carete + len_paste, data->text + data->carete, data->text_len - data->carete + 1);

    free(data->text);
    data->text = new_text;
    data->text_len = data->text_len + len_paste;

    instant_carete_pos(view, data->carete + len_paste);
}

static void backspace(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->carete == 0) return;

    if(data->selection[1]){
        erse_selected(view);
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

static void k_left(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->carete == 0){
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

static void ctrl_v(View *view){
    if(SDL_HasClipboardText()){
        Data *data = glv_get_view_data(view, data_offset);

        if(data->selection[1]){
            erse_selected(view);
        }

        char *text = SDL_GetClipboardText();

        Uint32 text_len = strlen(text);
        wchar_t *wtext = malloc((text_len + 1) * sizeof(wchar_t));

        //TODO: utf8 to unicode converting
        for(Uint32 i = 0; i <= text_len; i++){
            wtext[i] = text[i];
        }

        SDL_free(text);
        paste_text(view, wtext);
        free(wtext);
    }
}

static void ctrl_c(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    if(data->selection[1]){
        //TODO: unicode to utf-8 converting
        char *text = malloc(data->selection[1] + 1);
        for(Uint32 i = 0; i < data->selection[1]; i++){
            text[i] = data->text[data->selection[0] + i];
        }
        text[data->selection[1]] = 0;

        SDL_SetClipboardText(text);
        free(text);
    }
    else{
        Uint32 selection[2] = {0, data->text_len};
        instant_selection(view, selection);
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

    wchar_t *new_text = malloc((new_len + 1) * sizeof(wchar_t));

    wcsncpy(new_text, data->text, from);
    wcsncpy(new_text + from, data->text + from + cnt, data->text_len - from - cnt);
    new_text[new_len] = 0;

    free(data->text);
    data->text = new_text;
    data->text_len = new_len;

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