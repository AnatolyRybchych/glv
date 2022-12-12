#include <glv/flex_panel.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void apply_vsizing(View *flex_panel);
static void apply_hsizing(View *flex_panel);
static void apply_vmoving(View *flex_panel);
static void apply_hmoving(View *flex_panel);

static void align_childs(View *flex_panel);

ViewProc glv_flex_panel_proc = proc;

static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_CHILD_CREATE:
    case VM_CHILD_DELETE:
    case VM_CHILD_HIDE:
    case VM_CHILD_SHOW:
    case VM_CHILD_RESIZE:
    case VM_CHILD_MOVE:
    case VM_RESIZE:
    case VM_CREATE:
        align_childs(view);
    break;
    default:
        parent_proc(view, msg, in, out);
    }
}
