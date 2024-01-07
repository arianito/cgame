#pragma once
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "gui/cimgui.h"

CIMGUI_API void gui_init(void* context);
CIMGUI_API void gui_begin();
CIMGUI_API void gui_end();
CIMGUI_API void gui_destroy();