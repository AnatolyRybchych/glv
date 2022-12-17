#include <glv/text_input.h>

#define parent_proc(view, msg, in, out) glv_proc_default(view, msg, in, out)

typedef struct Data{
    int n;
} Data;

static void proc(View *view, ViewMsg msg, void *in, void *out);

static void on_init_data_size(View *view, Uint32 *size);
static void on_create(View *view);

ViewProc glv_text_input_proc = proc; 
static Uint32 data_offset;


static void proc(View *view, ViewMsg msg, void *in, void *out){
    switch (msg){
    case VM_CREATE:
        on_create(view);
        break;
    case VM_GET_VIEW_DATA_SIZE:
        on_init_data_size(view, out);
    break;
    default:
        parent_proc(view, msg, in, out);
    }
}

static void on_init_data_size(View *view, Uint32 *size){
    parent_proc(view, VM_GET_VIEW_DATA_SIZE, NULL, size);
    data_offset = *size;
    *size = data_offset + sizeof(Data);
}

static void on_create(View *view){
    Data *data = glv_get_view_data(view, data_offset);

    data->n = 1;
}
