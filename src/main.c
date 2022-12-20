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

void manage_canvas(View *view, ViewMsg msg, void *args, void *root_context){
    view = view;
    msg = msg;
    args = args;
    root_context = root_context;
    switch (msg){
    case VM_DRAW:{
        GlvMgr *mgr = glv_get_mgr(view);

        mgr = mgr;
    }break;
    }
}

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused
    view = view;
    
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "Sarai.ttf", 0);

    glv_stack_panel_set_alignment(view, 0, 0);
    glv_set_background(view, glv_gen_texture_solid_color(0x00, 0x80, 0xcc, 255));

    View *text_input = glv_create(view, glv_text_input_proc, NULL, NULL);
    glv_set_background(text_input, glv_gen_texture_solid_color(0x00, 0x57, 0xb7, 255));
    glv_set_foreground(text_input, glv_gen_texture_solid_color(0xff, 0xd7, 00, 255));
    glv_text_input_set_carete_color(text_input, 0xff /(float) 0xff, 0xd7 /(float) 0xff, 0x00 /(float) 0xff);
    glv_text_input_set_selection_color(text_input, 0x00 / 350.0, 0x57 / 350.0, 0xb7 / 350.0);
    glv_set_font_height(text_input, 48);
    glv_set_size(text_input, 200, 48);

    View *canvas = glv_create_weak(mgr, glv_canvas_proc, manage_canvas, NULL);
    canvas = canvas;
}

int main(void){
    return glv_run(glv_stack_panel_proc, NULL, NULL, init_spa);
}
