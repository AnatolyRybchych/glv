#include <glv/canvas.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

static void proc(View *view, ViewMsg msg, void *in, void *out);

ViewProc glv_canvas_proc = proc;

static Uint32 data_offset;

typedef struct Data{
    SDL_Point last_sz;
    bool required_resize;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out){
    out = out;//unused
    switch (msg){
    case VM_CREATE:{
        Data *data = glv_get_view_data(view, data_offset);
        data->required_resize = true;
        
    }break;
    case VM_GET_VIEW_DATA_SIZE:{
        parent_proc(view, msg, in, out);
        Uint32 *sz = out;
        data_offset = *sz;
        *sz = data_offset + sizeof(Data);
    } break;
    case VM_RESIZE:{
        Data *data = glv_get_view_data(view, data_offset);
        data->required_resize = true;
        data->last_sz = *(const SDL_Point*)in;

        glv_draw(view);
    } break;
    case VM_DRAW:{
        Data *data = glv_get_view_data(view, data_offset);
        GLuint tex = glv_get_texture(view);

        if(data->required_resize){
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data->last_sz.x, data->last_sz.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);

            data->required_resize = false;
        }
        glViewport(0, 0, data->last_sz.x, data->last_sz.y);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

    } break;
    default:
        parent_proc(view, msg, in, out);
    }
}