#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>
#include <glv/stack_panel.h>

void view_proc(View *view, ViewMsg msg, void *in, void *out){
    in = in; //unused
    out = out; //unused
    switch (msg){
    case VM_CREATE:
        glv_draw(view);
        break;
    case VM_DRAW:{
        GLuint texture = glv_get_texture(view);

        glBindTexture(GL_TEXTURE_2D, texture);
        SDL_Color bg_color = {
            .r = 30,
            .g = 15,
            .b = 15,
            .a = 255
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &bg_color);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    }
}

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused

    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "./Sarai.ttf", 0);

    View *stack_panel = glv_create(view, glv_stack_panel_proc, NULL, NULL);
    glv_set_size(stack_panel, 800, 800);

    View *text1 = glv_create(stack_panel, glv_text_view_proc, NULL, NULL);
    View *text2 = glv_create(stack_panel, glv_text_view_proc, NULL, NULL);

    glv_set_pos(text2, 0, 100);
    text2 = text2;

    glv_text_view_set_text(text1, L"text 1");
    glv_text_view_set_text(text2, L"text 2");

    glv_text_view_normalize(text1, false);
    glv_text_view_normalize(text2, false);


    glv_text_view_set_solid_foreground(text1, (SDL_Color){200, 100, 50, 255});
    glv_text_view_set_solid_foreground(text2, (SDL_Color){120, 110, 100, 255});
}

int main(void){
    return glv_run(view_proc, NULL, NULL, init_spa);
}
