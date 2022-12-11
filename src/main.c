#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>
#include <glv/stack_panel.h>

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused

    glv_stack_panel_set_vertical(view);

    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "./Sarai.ttf", 0);

    View *text1 = glv_create(view, glv_text_view_proc, NULL, NULL);
    View *text2 = glv_create(view, glv_text_view_proc, NULL, NULL);

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
    return glv_run(glv_stack_panel_proc, NULL, NULL, init_spa);
}
