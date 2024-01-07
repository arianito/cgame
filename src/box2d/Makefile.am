noinst_LTLIBRARIES = libbox2d.la

libbox2d_la_SOURCES = \
    aabb.c \
    arena_allocator.c \
    array.c \
    bitset.c \
    block_allocator.c \
    body.c \
    broad_phase.c \
    contact.c \
    contact_solver.c \
    distance.c \
    distance_joint.c \
    dynamic_tree.c \
    geometry.c \
    graph.c \
    hull.c \
    island.c \
    joint.c \
    manifold.c \
    motor_joint.c \
    mouse_joint.c \
    pool.c \
    prismatic_joint.c \
    revolute_joint.c \
    shape.c \
    table.c \
    timer.c \
    types.c \
    weld_joint.c \
    wheel_joint.c \
    world.c

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Wextra -static -msse2 -mavx