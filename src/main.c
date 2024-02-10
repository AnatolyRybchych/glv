#include <glv.h>
#include <glv/stack_panel.h>

typedef struct{
    int n;
} AppContext;

void app_manage(View *view, ViewMsg msg, void *ev, void *app_context){
    (void)ev;
    (void)view;
    AppContext *ctx = app_context;

    switch (msg){
    case VM_MOUSE_DOWN:
        printf("click %i\n", ctx->n++);
        break;
    }
}

void app_init(View *root_view, void *app_context){
    AppContext *ctx = app_context;
    printf("ctx:%i\n", ctx->n);

    glv_set_background(root_view, glv_gen_texture_solid_color(80, 160, 200, 255));
}

int main(void){
    AppContext ctx = {
        .n = 0,
    };

    glv_run(glv_stack_panel_proc, app_manage, &ctx, app_init);
    return 0;
}
