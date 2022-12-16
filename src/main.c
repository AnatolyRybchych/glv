#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>
#include <glv/stack_panel.h>
#include <glv/canvas.h>
#include <glv/margin.h>
#include <glv/menu_panel.h>
#include <glv/popup_panel.h>

// void manage_root(View *view, ViewMsg msg, void *args, void *root_context){
//     view = view;
//     msg = msg;
//     args = args;
//     root_context = root_context;
//     switch (msg){
//     case VM_DRAW:{
//         GlvMgr *mgr = glv_get_mgr(view);
        
//         glv_draw_quadrangle_rel_polycolor(mgr, 
//             (float[2]){0, -1}, 
//             (float[2]){0, 1}, 
//             (float[2]){1, 1}, 
//             (float[2]){1, -1.1}, 

//             (float[3]){1.0, 0.0, 0.0},
//             (float[3]){0.0, 1.0, 0.0},
//             (float[3]){0.0, 0.0, 1.0},
//             (float[3]){1.0, 1.0, 0.0}
//         );

//         glv_draw_circle(mgr, 100, 100, 100, 1, 0, 0, 255);
//         glv_draw_circle(mgr, 100, 100, 200, 1, 0, 0, 255);
//         glv_draw_circle(mgr, 100, 200, 100, 1, 0, 0, 255);
//     }break;
//     }
// }

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused
    view = view;
    
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "Sarai.ttf", 0);

    View *popup_text = glv_create(view, glv_text_view_proc, NULL, NULL);
    View *content_text = glv_create(view, glv_text_view_proc, NULL, NULL);

    glv_text_view_set_text(popup_text, L"popup");
    glv_text_view_set_text(content_text, L"content");

    glv_set_foreground(popup_text, glv_gen_texture_solid_color(255, 0, 0, 255));
    glv_set_foreground(content_text, glv_gen_texture_solid_color(0, 255, 0, 255));

    glv_popup_panel_set_content(view, content_text);
    glv_popup_panel_set_popup(view, popup_text);

    glv_popup_panel_show_popup(view);
    glv_popup_panel_hide_popup(view);
}

int main(void){
    return glv_run(glv_popup_panel_proc, NULL, NULL, init_spa);
}
