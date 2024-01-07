SUBDIRS = adt mem engine levels math geometry box2d gui

bin_PROGRAMS = game

game_SOURCES = \
    alloc.c \
    main.c 

game_LDADD = \
    levels/liblevels.la \
    gui/libgui.la \
    box2d/libbox2d.la \
    geometry/libgeometry.la \
    engine/libengine.la \
    adt/libadt.la \
    mem/libmem.la \
    math/libmath.la
 
game_LDADD += -lglfw -lm -ljemalloc

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Wextra -static -Wl,--copy-dt-needed-entries