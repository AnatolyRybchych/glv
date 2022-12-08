#include <glv/text.h>

static void view_proc(View *view, ViewMsg msg, void *in, void *out);

ViewProc glv_text_view_proc = view_proc;

typedef struct TextViewData{
    char *text;
    Uint32 face_width;
    Uint32 face_height;
    GlvFaceId face;
} TextViewData;

static void view_proc(View *view, ViewMsg msg, void *in, void *out){
    static Uint32 data_offset;

    switch (msg){
    case VM_CREATE:{
        TextViewData *data = glv_get_view_data(view, data_offset);
        data->text = NULL;
        data->face_height = 800;
        data->face_width = 0;
        glv_draw(view);
    }break;
    case VM_GET_VIEW_DATA_SIZE:{
        glv_proc_default(view, msg, in, out);
        data_offset = *(Uint32*)out;
        *(Uint32*)out += sizeof(TextViewData);
    }break;
    case VM_DRAW:{
        
        TextViewData *data = glv_get_view_data(view, data_offset);
        FT_Face face = glv_get_freetype_face(glv_get_mgr(view), data->face);

        if(data->text == NULL) break;

        GLuint texture = glv_get_texture(view);

        FT_Set_Pixel_Sizes(face, data->face_width, data->face_height);
        FT_Load_Char(face, *data->text, FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;

        SDL_Color *glyph_data = malloc(glyph->bitmap.width * glyph->bitmap.rows * sizeof(SDL_Color));

        for (unsigned int x = 0; x < glyph->bitmap.width; x++){
            for (unsigned int y = 0; y < glyph->bitmap.rows; y++){
                glyph_data[(glyph->bitmap.rows - y - 1) * glyph->bitmap.width + x] = (SDL_Color){
                    .r = 255,
                    .g = 0,
                    .b = 0,
                    .a = glyph->bitmap.buffer[y * glyph->bitmap.width + x]
                };
            }
        }
        

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glyph->bitmap.width, glyph->bitmap.rows, 
                                        0, GL_RGBA, GL_UNSIGNED_BYTE, glyph_data);
        glBindTexture(GL_TEXTURE_2D, 0);

        free(glyph_data);
    }break;
    case VM_TEXT_SET_TEXT:{
        TextViewData *data = glv_get_view_data(view, data_offset);
        free(data->text);
        data->text = malloc(strlen(in) + 1);
        strcpy(data->text, in);
        glv_draw(view);
    } break;;
    default:
        glv_proc_default(view, msg, in, out);
        break;
    }
}

void glv_text_set_text(View *text_view, const char *text){
    SDL_assert(text != NULL);
    glv_call_event(text_view, VM_TEXT_SET_TEXT, (void*)text, NULL);
    glv_call_manage(text_view, VM_TEXT_SET_TEXT, (void*)text);
}



