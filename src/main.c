#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>
#include <glv/stack_panel.h>
#include <glv/canvas.h>
#include <glv/margin.h>
#include <glv/menu_panel.h>
#include <glv/popup_panel.h>
#include <glv/text_input.h>

// void manage_root(View *view, ViewMsg msg, void *args, void *root_context){
//     view = view;
//     msg = msg;
//     args = args;
//     root_context = root_context;
//     switch (msg){
//     case VM_MOUSE_DOWN:{
//         GlvMouseDown *e = args;
//         if(e->button == 1){
//             glv_popup_panel_show_popup(view);
//         }
//         else{
//             glv_popup_panel_hide_popup(view);
//         }
//     }break;
//     }
// }

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused
    view = view;
    
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "Sarai.ttf", 0);

    glv_set_background(view, glv_gen_texture_solid_color(60, 50, 35, 255));
    glv_set_foreground(view, glv_gen_texture_solid_color(160, 100, 80, 255));

    View *popup = glv_create_popup(mgr, glv_text_view_proc, NULL, NULL);
    
    glv_text_view_set_text(popup, L"text");

    glv_set_foreground(popup, glv_gen_texture_solid_color(255, 0, 0, 255));

    glv_delete(popup);
}

int main(void){
    return glv_run(glv_text_input_proc, NULL, NULL, init_spa);
}
