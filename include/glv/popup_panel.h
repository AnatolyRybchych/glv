#ifndef __GLV_POPUP_PANEL_H
#define __GLV_POPUP_PANEL_H

#include <glv/background.h>

#define VM_POPUP_PANEL_SHOW_POPUP VM_USER_FIRST + 40
#define VM_POPUP_PANEL_HIDE_POPUP VM_USER_FIRST + 41
#define VM_POPUP_PANEL_SET_POPUP VM_USER_FIRST + 42
#define VM_POPUP_PANEL_SET_CONTENT VM_USER_FIRST + 43

/*
                        popup panel
    popup panel consists of two parts: content and popup
    when popup is show, content is hidden
    popup is hidden content is show
    
    others child are allways hidden 

    created for custom message boxes without creating a separate window

    TODO: blured content as background if background texture is 0
*/

extern ViewProc glv_popup_panel_proc;

void glv_popup_panel_show_popup(View *popup_panel);
void glv_popup_panel_hide_popup(View *popup_panel);
void glv_popup_panel_set_popup(View *popup_panel, View *popup);
void glv_popup_panel_set_content(View *popup_panel, View *content);

#endif //__GLV_POPUP_PANEL_H
