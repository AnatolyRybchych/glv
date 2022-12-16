#include <stdio.h>
#include <glv.h>
#include <SDL2/SDL.h>
#include <glv/text_view.h>
#include <glv/stack_panel.h>
#include <glv/canvas.h>
#include <glv/margin.h>
#include <glv/menu_panel.h>

void manage_root(View *view, ViewMsg msg, void *args, void *root_context){
    view = view;
    msg = msg;
    args = args;
    root_context = root_context;

    switch (msg){
    case VM_MOUSE_MOVE:{
        const GlvMouseMove *mmove = args;
        SDL_Point p = {.x = mmove->x, .y = mmove->y};
        p = glv_view_to_parent(view, p);

        printf("parent:{\t%i,\t%i\t}\n", p.x, p.y);
    }break;
    }
}

void init_spa(View *view, void *root_context){
    root_context = root_context;//unused
    view = view;
    
    GlvMgr *mgr = glv_get_mgr(view);
    glv_new_freetype_face(mgr, "Sarai.ttf", 0);

    View *stack_panel = glv_create(view, glv_stack_panel_proc, NULL, NULL);

    glv_stack_panel_set_horisontal(stack_panel);
    glv_stack_panel_set_alignment(stack_panel, -1, 0);

    View *text1 = glv_create(stack_panel, glv_text_view_proc, NULL, NULL);
    View *text2 = glv_create(stack_panel, glv_text_view_proc, NULL, NULL);
    View *text3 = glv_create(stack_panel, glv_text_view_proc, NULL, NULL);

    glv_text_view_set_text(text1, L"text 1 ");
    glv_text_view_set_text(text2, L"text 2 ");
    glv_text_view_set_text(text3, L"text 3 ");

    glv_text_view_normalize(text1, false);
    glv_text_view_normalize(text2, false);
    glv_text_view_normalize(text3, false);

    glv_set_foreground(text1, glv_gen_texture_solid_color(120, 110, 100, 255));
    glv_set_foreground(text2, glv_gen_texture_solid_color(120, 110, 100, 255));
    glv_set_foreground(text3, glv_gen_texture_solid_color(120, 110, 100, 255));

    View *text4 = glv_create(view, glv_text_view_proc, NULL, NULL);
    glv_text_view_set_text(text4, L"text 4 ");
    glv_text_view_normalize(text4, false);
    glv_set_foreground(text4, glv_gen_texture_solid_color(140, 120, 100, 255));

    glv_menu_panel_set_menu(view, stack_panel);
    glv_menu_panel_set_top(view);
    glv_menu_panel_set_size(view, 50);

    glv_set_background(view, glv_gen_texture_solid_color(20, 15, 10, 255));
    // glv_set_background(text2, glv_gen_texture_solid_color(120, 110, 100, 255));
    // glv_set_background(text3, glv_gen_texture_solid_color(100, 110, 90, 255));

    // glv_set_background(text1_margin, glv_gen_texture_solid_color(255, 0, 0, 255));

    // glv_set_background(view, glv_gen_texture_solid_color(20, 20, 20, 255));
}

int main(void){
    return glv_run(glv_menu_panel_proc, manage_root, NULL, init_spa);
}
