#ifndef __GLV_STACK_PANEL_H
#define __GLV_STACK_PANEL_H

#include <glv/background.h>

#define VM_STACK_PANEL_SET_VERTICAL VM_USER_FIRST + 10
#define VM_STACK_PANEL_SET_HORISONTAL VM_USER_FIRST + 11
#define VM_STACK_PANEL_SET_ALIGNMENT VM_USER_FIRST + 12
#define VM_STACK_PANEL_SET_STRETCH VM_USER_FIRST + 13


//if background is 0, background isnt drawable
extern ViewProc glv_stack_panel_proc;

void glv_stack_panel_set_vertical(View *stack_panel);
void glv_stack_panel_set_horisontal(View *stack_panel);

//if alignment is < 0 -> alignment begin
//if alignment is == 0 -> alignment center
//if alignment is > 0 -> alignment end
void glv_stack_panel_set_alignment(View *stack_panel, int x, int y);

//if true, elements will be stretched to fill all space in chosen dirrection
void glv_stack_panel_set_stretching(View *stack_panel, bool x, bool y);

#endif //__GLV_STACK_PANEL_H