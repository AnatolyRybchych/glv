#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text.h>

void init_spa(View *view){
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "./Sarai.ttf", 0);
    glv_text_set_text(view, "g");

    View *child = glv_create(view, glv_text_view_proc, NULL, NULL);
    glv_text_set_text(child, "q");
}

int main(void){
    return glv_run(glv_text_view_proc, NULL, NULL, init_spa);
}
