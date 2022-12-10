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
            .a = 15
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
    glv_text_view_set_text(text, "Some text");
    glv_set_pos(text, 0, 0);
    glv_set_size(text, 800, 800);

    GlvSetStyle style = {
        .self_size = sizeof(style),
        .apply_fg = true,
        .apply_font_face = true,
        .apply_font_height = true,
        .foreground = {
            .solid_color = {
                .type = GLV_COLORSTYLE_SOLID,
                .a = 255,
                .r = 255,
                .g = 140,
                .b = 120,
            },
        },
        .font_face_id = 0,
        .font_height = 72,
    };

    glv_set_style(text, &style);
}

int main(void){
    return glv_run(view_proc, NULL, NULL, init_spa);
}
