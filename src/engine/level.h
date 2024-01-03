#ifndef cgame_LEVEL_H
#define cgame_LEVEL_H

typedef struct
{
    void *context;
    void *(*create)(void *);
    void *(*update)(void *);
    void *(*destroy)(void *);
} Level;

void level_init(int n);

void level_add(Level level);

void level_activate(int i);

void level_update();

void level_destroy();

void level_prev();

void level_next();

#endif