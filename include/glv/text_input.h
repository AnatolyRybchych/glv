#ifndef __GLV_TEXT_INPUT_H
#define __GLV_TEXT_INPUT_H

#include <glv.h>

#define VM_TEXT_INPUT_SET_CARETE_POS VM_USER_FIRST + 50
#define VM_TEXT_INPUT_SET_SELECTION VM_USER_FIRST + 51

extern ViewProc glv_text_input_proc; 

void glv_text_input_set_carete_pos(View *text_input, Uint32 carete_pos);
void glv_text_input_set_selection(View *text_input, Uint32 first, Uint32 count);


#endif //__GLV_TEXT_INPUT_H
