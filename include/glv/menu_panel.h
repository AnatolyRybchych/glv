#ifndef __GLV_MENU_PANEL_H
#define __GLV_MENU_PANEL_H

#include <glv/background.h>

#define VM_MENU_PANEL_SET_LEFT VM_USER_FIRST + 30
#define VM_MENU_PANEL_SET_TOP VM_USER_FIRST + 31
#define VM_MENU_PANEL_SET_RIGHT VM_USER_FIRST + 32
#define VM_MENU_PANEL_SET_BOTTOM VM_USER_FIRST + 33

#define VM_MENU_PANEL_SET_MENU_SIZE VM_USER_FIRST + 34

#define VM_MENU_PANEL_SET_MENU VM_USER_FIRST + 35

extern ViewProc glv_menu_panel_proc;

void glv_menu_panel_set_left(View *menu_panel);
void glv_menu_panel_set_top(View *menu_panel);
void glv_menu_panel_set_right(View *menu_panel);
void glv_menu_panel_set_bottom(View *menu_panel);

void glv_menu_panel_set_size(View *menu_panel, Uint32 size);
void glv_menu_panel_set_menu(View *menu_panel, View *menu);

#endif //__GLV_MENU_PANEL_H
