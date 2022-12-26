#include <glv/background.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

static void proc(View *view, ViewMsg msg, void *in, void *out);

ViewProc glv_background_proc = proc;

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_SET_BG:
        if(*(GLuint*)in == 0) glv_deny_draw(view);
        else glv_swap_texture_with_bg(view);
        break;
    default:
        parent_proc(view, msg, in, out);
    }
}