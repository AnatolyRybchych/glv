#ifndef __GLV_STACK_PANEL_H
#define __GLV_STACK_PANEL_H

#include <glv.h>

#define VM_STACK_PANEL_SET_VERTICAL VM_USER_FIRST + 10
#define VM_STACK_PANEL_SET_HORISONTAL VM_USER_FIRST + 11
#define VM_STACK_PANEL_SET_ALIGNMENT VM_USER_FIRST + 12

extern ViewProc glv_stack_panel_proc;

void glv_stack_panel_set_vertical(View *stack_panel);
void glv_stack_panel_set_horisontal(View *stack_panel);

//if alignment is < 0 -> alignment begin
//if alignment is == 0 -> alignment center
//if alignment is > 0 -> alignment end
void glv_stack_panel_set_alignment(View *stack_panel, int x, int y);

#endif //__GLV_STACK_PANEL_H