#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>

void manage_proc(View *view, uint32_t msg, void *in, void *user_data){
    in = in;//unused
    user_data = user_data;//unused
    switch (msg){
    case VM_CREATE:
        glv_set_pos(view, 100, 100);
        glv_set_size(view, 200, 200);
        break;
    }
}

void view_proc2(View *view, uint32_t msg, void *in, void *out){
    switch (msg){
    case VM_CREATE:{
        glv_draw(view);
        printf("create\n");
    }break;
    case VM_SHOW:{
        printf("show\n");
    } break;
    case VM_HIDE:{
        printf("hide\n");
    }break;
    case VM_DRAW:{
        GLuint texture = glv_get_texture(view);

        glBindTexture(GL_TEXTURE_2D, texture);
        unsigned char pixels[] = {
            50, 0, 0, 255,        0, 50, 0, 255, 
            0, 0, 50, 255,        50, 0, 50, 255
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        printf("draw\n");
    }break;
    case VM_MOUSE_DOWN:{
        const GlvMouseDown *ev = in;
        printf("mouse %i button down \t{%i, %i}\t times %i\n", ev->button, ev->x, ev->y, ev->clicks);
    } break;
    case VM_MOUSE_UP:{
        const GlvMouseDown *ev = in;
        printf("mouse %i button up \t{%i, %i}\t times %i\n", ev->button, ev->x, ev->y, ev->clicks);
    } break;
    case VM_MOUSE_MOVE:{
        const GlvMouseMove *ev = in;
        printf("mouse move \t{%i, %i}\n", ev->x, ev->y);
    } break;
    case VM_FOCUS:{
        printf("focus\n");
    }break;
    case VM_UNFOCUS:{
        printf("unfocus\n");
    }break;
    case VM_DELETE:{
        printf("delete\n");
    } break;
    case VM_CHILD_RESIZE:{
        printf("childs resize\n");
    } break;
    case VM_CHILD_MOVE:{
        printf("child move\n");
    } break;
    case VM_CHILD_CREATE:{
        printf("child create\n");
    } break;
    case VM_CHILD_DELETE:{
        printf("child delete\n");
    } break;
    case VM_MOUSE_HOVER:{
        printf("mouse hover\n");
    } break;
    case VM_MOUSE_LEAVE:{
        printf("mouse leave\n");
    } break;
    case VM_KEY_DOWN:{
        const GlvKeyDown *ev = in;
        printf("key\t%i down repeat %hhi\n", ev->sym.sym, ev->repeat);
    }break;
    case VM_TEXT:{
        printf("text_input: \"%s\"\n", (const char *)in);
    }break;
    case VM_TEXT_EDITING:{
        GlvTextEditing *te = in;
        printf("[%i, %i]\ttext edit: \"%s\"\n", te->cursor, te->selection_len, te->composition);
    }break;
    }
    glv_proc_default(view, msg, in, out);
}


void view_proc(View *view, uint32_t msg, void *in, void *out){
    switch (msg){
    case VM_CREATE:{
        glv_draw(view);
    }break;
    case VM_DRAW:{
        GLuint texture = glv_get_texture(view);

        glBindTexture(GL_TEXTURE_2D, texture);
        unsigned char pixels[] = {
            255, 0, 0, 255,        0, 255, 0, 255, 
            0, 0, 255, 255,        255, 0, 255, 255
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
    } break;
    }
    glv_proc_default(view, msg, in, out);
}

void init_spa(View *view){
    View *child = glv_create(view, view_proc2, NULL, NULL);

    glv_set_pos(child, 100, 100);
    glv_set_size(child, 200, 200);
}

int main(void){

    return glv_run(view_proc, manage_proc, NULL, init_spa);
}
