#ifndef __GLV_TEXT_INPUT_H
#define __GLV_TEXT_INPUT_H

#include <glv.h>

#define VM_TEXT_INPUT_SET_CARETE_POS VM_USER_FIRST + 50
#define VM_TEXT_INPUT_SET_SELECTION VM_USER_FIRST + 51
#define VM_TEXT_INPUT_TEXT_CHANGED VM_USER_FIRST + 52
#define VM_TEXT_INPUT_GET_TEXT VM_USER_FIRST + 53
#define VM_TEXT_INPUT_GET_SELECTION VM_USER_FIRST + 54
#define VM_TEXT_INPUT_GET_CARETE VM_USER_FIRST + 55
#define VM_TEXT_INPUT_SET_CARETE_COLOR VM_USER_FIRST + 56
#define VM_TEXT_INPUT_SET_SELECTION_COLOR VM_USER_FIRST + 57

extern ViewProc glv_text_input_proc; 

void glv_text_input_set_carete_pos(View *text_input, Uint32 carete_pos);
void glv_text_input_set_selection(View *text_input, Uint32 first, Uint32 count);
const wchar_t *glv_text_input_get_text(View *text_input);
void glv_text_input_get_selection(View *text_input, Uint32 selection[2]);
Uint32 glv_text_input_get_carete(View *text_input);
void glv_text_input_set_carete_color(View *text_input, float r, float g, float b);
void glv_text_input_set_selection_color(View *text_input, float r, float g, float b);


#endif //__GLV_TEXT_INPUT_H
