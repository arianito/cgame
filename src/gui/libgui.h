#pragma once
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "gui/cimgui.h"
#include "math/vec2.h"

CIMGUI_API void gui_init(const char *font);
CIMGUI_API void gui_begin();
CIMGUI_API void gui_end();
CIMGUI_API void gui_destroy();

#define vec2im(v) (*(Vec2 *)(&(v)))
#define imvec2(v) (*(ImVec2 *)(&(v)))
#define imvec2f(x, y) ((ImVec2){x, y})