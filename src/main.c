#include <glv.h>
#include <glv/text_view.h>
#include <glv/margin.h>

#include <stdio.h>

typedef struct{
    int n;
    View *output;
} AppContext;

void app_manage(View *view, ViewMsg msg, void *ev, void *app_context){
    (void)ev;
    (void)view;
    AppContext *ctx = app_context;

    wchar_t buf[128];

    switch (msg){
    case VM_MOUSE_DOWN:
        ctx->n++;
        swprintf(buf, sizeof buf / sizeof *buf, L"%i", ctx->n);
        glv_text_view_set_text(ctx->output, buf);
        break;
    }
}

void app_init(View *root_view, void *app_context){
    AppContext *ctx = app_context;
    GlvMgr *mgr = glv_get_mgr(root_view);
    glv_new_freetype_face(mgr, "NotoMono-Regular.ttf", 0);
    
    View *text = glv_create(root_view, glv_text_view_proc, NULL, NULL);
    glv_set_background(text, 0);
    glv_set_font(text, 0);
    glv_set_font_height(text, 128);
    ctx->output = text;

    glv_margin_set_absolute(root_view, 0, 0, 0, 0);

    glv_set_background(root_view, glv_gen_texture_solid_color(80, 160, 200, 255));
}

int main(void){
    AppContext ctx = {
        .n = 0,
    };

    glv_run(glv_margin_proc, app_manage, &ctx, app_init);
    return 0;
}
