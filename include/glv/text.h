#ifndef __GLV_TEXT_H_
#define __GLV_TEXT_H_

#include <glv.h>

#define VM_TEXT_SET_TEXT VM_USER_FIRST

extern ViewProc glv_text_view_proc;

void glv_text_set_text(View *text_view, const char *text);



#endif //__GLV_TEXT_H_
