#ifndef cgame_GAME_H
#define cgame_GAME_H

#include <stdbool.h>
#include "math/vec2.h"

typedef struct
{
    void *window;
    Vec2 size;
    float ratio;
    int fps;
    bool full_screen;
    Vec2 screen;
} Game;

typedef struct
{
    float elapsed;
    float delta;
} Time;

extern Game *game;
extern Time *gtime;

void game_init();
void game_begin();
bool game_end();
void game_exit();
void game_resist();
void game_terminate();

#endif