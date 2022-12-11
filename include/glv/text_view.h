#ifndef __GLV_TEXT_VIEW_H_
#define __GLV_TEXT_VIEW_H_

#include <glv.h>

#define VM_TEXT_VIEW_SET_TEXT VM_USER_FIRST
#define VM_TEXT_VIEW_APPEND_TEXT VM_USER_FIRST + 1
#define VM_TEXT_VIEW_GET_TEXT VM_USER_FIRST + 2
#define VM_TEXT_VIEW_GET_TEXT_PARAMS VM_USER_FIRST + 3

extern ViewProc glv_text_view_proc;

typedef struct GlvTextViewTextParams GlvTextViewTextParams;

void glv_text_view_set_text(View *text_view, const wchar_t *text);
void glv_text_view_append_text(View *text_view, const wchar_t *text);
const wchar_t *glv_text_view_get_text(View *text_view);
GlvTextViewTextParams glv_text_view_get_text_params(View *text_view);
void glv_text_view_set_font_faceid(View *text_view, GlvFontFaceId face);
void glv_text_view_set_font_width(View *text_view, Uint32 width);
void glv_text_view_set_font_height(View *text_view, Uint32 height);
void glv_text_view_set_solid_foreground(View *text_view, SDL_Color color);

struct GlvTextViewTextParams{
    GlvFontFaceId font_face_id;

    Uint32 face_width;
    Uint32 face_height;

    GlvColorStyle text_color;

    Uint32 text_len;
    Uint32 text_width;
};

#endif //__GLV_TEXT_VIEW_H_
