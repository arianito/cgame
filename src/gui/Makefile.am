noinst_LTLIBRARIES = libgui.la

libgui_la_SOURCES = \
    cimgui.cpp \
    imgui_draw.cpp \
    imgui_demo.cpp \
    imgui_impl_glfw.cpp \
    imgui_impl_opengl3.cpp \
    imgui_tables.cpp \
    imgui_widgets.cpp \
    imgui_freetype.cpp \
    libgui.cpp \
    imgui.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src -I/usr/include/freetype2 -I/usr/include/libpng16 -DIMGUI_FREETYPE
AM_CFLAGS = -Wall -Wextra -static 