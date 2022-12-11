#include <glv/stack_panel.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

static void proc(View *view, ViewMsg msg, void *in, void *out);



ViewProc glv_stack_panel_proc = proc;

//static Uint32 data_offset ;

static void proc(View *view, ViewMsg msg, void *in, void *out){
    in = in;
    out = out;
    view = view;

    glv_print_docs(view, msg);

    switch (msg){
    default:
        parent_proc(view, msg, in, out);
        break;
    }
}