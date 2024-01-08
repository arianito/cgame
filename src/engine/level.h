#ifndef cgame_LEVEL_H
#define cgame_LEVEL_H

typedef void VoidFunc(void*);

typedef struct
{
    void *context;
    VoidFunc* create;
    VoidFunc* render_before;
    VoidFunc* render;
    VoidFunc* render_after;
    VoidFunc* destroy;
} Level;

void level_init(int n);

void level_add(Level level);

void level_activate(int i);

void level_render_before();

void level_render();

void level_render_after();

void level_destroy();

void level_prev();

void level_next();

#endif