#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text.h>

void manage_proc(View *view, uint32_t msg, void *in, void *user_data){
    in = in;//unused
    user_data = user_data;//unused
    switch (msg){
    case VM_CREATE:
        glv_text_set_text(view, "text");
        glv_set_pos(view, 100, 100);
        glv_set_size(view, 200, 200);
        break;
    }
}


void init_spa(View *view){
    GlvMgr *mgr = glv_get_mgr(view);
    GlvFaceId sarai_face_id = glv_new_freetype_face(mgr, "./Sarai.ttf", 0);

    printf("%i\n", sarai_face_id);

    View *child = glv_create(view, glv_text_view_proc, manage_proc, NULL);

    glv_set_pos(child, 100, 100);
    glv_set_size(child, 200, 200);
}

int main(void){
    return glv_run(glv_proc_default, manage_proc, NULL, init_spa);
}
