SUBDIRS = adt mem engine levels

bin_PROGRAMS = game
game_SOURCES = main.c

game_LDADD = levels/liblevels.la engine/libengine.la adt/libadt.la mem/libmem.la 

game_LDADD += -lglfw -lm -ljemalloc

AM_CPPFLAGS = -msse2 -I$(top_srcdir)/src 