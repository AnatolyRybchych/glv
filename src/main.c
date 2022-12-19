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

    glv_stack_panel_set_alignment(view, 0, 0);
    glv_set_background(view, glv_gen_texture_solid_color(20, 16, 7, 255));

    View *text_input = glv_create(view, glv_text_input_proc, NULL, NULL);
    glv_set_background(text_input, glv_gen_texture_solid_color(10, 8, 3, 255));
    glv_set_foreground(text_input, glv_gen_texture_solid_color(200, 200, 0, 255));
    glv_text_input_set_carete_color(text_input, 0.8, 0.6, 0.4);
    glv_text_input_set_selection_color(text_input, 0.8, 0.6, 0.4);
    glv_set_font_height(text_input, 48);
    glv_set_size(text_input, 200, 48);
}

int main(void){
    return glv_run(glv_stack_panel_proc, NULL, NULL, init_spa);
}
