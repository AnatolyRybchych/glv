#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>

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

void init_spa(View *view){
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "./Sarai.ttf", 0);

    View *text = glv_create(view, glv_text_view_proc, NULL, NULL);
    glv_text_view_set_text(text, L"Some text");
    glv_set_pos(text, 0, 0);
    glv_set_size(text, 800, 800);

    glv_text_view_set_font_height(text, 200);
    glv_text_view_set_solid_foreground(text, (SDL_Color){190, 120, 110, 255});
    glv_text_view_set_alignment(text, 0, -1);
    glv_text_view_normalize(text, true);
}

int main(void){
    return glv_run(view_proc, NULL, NULL, init_spa);
}
