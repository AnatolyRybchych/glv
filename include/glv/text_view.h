#ifndef __GLV_TEXT_VIEW_H_
#define __GLV_TEXT_VIEW_H_

#include <glv.h>

#define VM_TEXT_VIEW_SET_TEXT VM_USER_FIRST
#define VM_TEXT_VIEW_APPEND_TEXT VM_USER_FIRST + 1
#define VM_TEXT_VIEW_GET_TEXT VM_USER_FIRST + 2
#define VM_TEXT_VIEW_GET_TEXT_PARAMS VM_USER_FIRST + 3
#define VM_TEXT_VIEW_SET_TEXTALIGN VM_USER_FIRST + 4
#define VM_TEXT_VIEW_NORMALIZE VM_USER_FIRST + 5

//if background is 0, background is complitely transparent
extern ViewProc glv_text_view_proc;

typedef struct GlvTextViewTextParams GlvTextViewTextParams;

void glv_text_view_set_text(View *text_view, const wchar_t *text);
void glv_text_view_append_text(View *text_view, const wchar_t *text);
const wchar_t *glv_text_view_get_text(View *text_view);
GlvTextViewTextParams glv_text_view_get_text_params(View *text_view);

//if alignment is < 0 -> alignment begin
//if alignment is == 0 -> alignment center
//if alignment is > 0 -> alignment end
void glv_text_view_set_alignment(View *text_view, int x, int y);

//resize view to make it size equals to text size  
//if move is true, moves this view to save absolute text location 
void glv_text_view_normalize(View *text_view, bool move);


struct GlvTextViewTextParams{
    GlvFontFaceId font_face_id;

    Uint32 face_width;
    Uint32 face_height;

    Uint32 text_len;
    Uint32 text_width;

    int alignment_x;
    int alignment_y;
};

#endif //__GLV_TEXT_VIEW_H_
