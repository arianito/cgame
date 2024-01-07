noinst_LTLIBRARIES = libmath.la

libmath_la_SOURCES = \
    color.c \
    edge.c \
    mat4.c \
    quad.c \
    quat.c \
    rand.c \
    ray.c \
    rot.c \
    scalar.c \
    tetrahedron.c \
    triangle.c \
    noise.c

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Wextra -static -msse2 -mavx