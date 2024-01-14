#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "mem/alloc.h"

#include "skel/anim.h"

#include "gui/libgui.h"

typedef struct
{
} Graph1Context;

static void create(Graph1Context *self)
{
    gui_init(game->window, "fonts/roboto.ttf");
}
typedef struct
{
    int i;
    float pt;
    float t0;
    float value;
    bool loop;
} State;

void interpolate(State *s, KeyFrame *keys, int n, float t, float dt)
{
    float ft = t - s->t0;

    if (absf(t - s->pt) > dt)
    {
        printf("shit!\n");
        // JUMP!
        for (int i = 0; i < n; i++)
        {
            if (keys[i].t > t)
            {
                s->i = i;
                break;
            }
        }
    }

    KeyFrame *key = &keys[s->i];
    if (key->t < ft)
        s->i++;

    if (s->i >= n)
    {
        s->i = 1;
        s->t0 = t;
    }

    float out = 0;
    float t0 = keys[s->i - 1].t;
    float v0 = keys[s->i - 1].value;
    float d = key->t - t0;
    s->value = lerp01f(v0, key->value, (ft - t0) / d);
    s->pt = t;
}

static void render(Graph1Context *self)
{
    KeyFrame keys[4] = {
        {0, 4},
        {1, 5},
        {1.5, 2},
    };
}

static float l = 50, c = 22, h = 67;

static void render_after(Graph1Context *self)
{
    gui_begin();



    igBegin("hello", NULL, 0);
    

    igEnd();


    gui_end();
}
static void destroy(Graph1Context *self)
{
    gui_destroy();
}

Level make_graph1()
{
    return (Level){
        context : xxarena(sizeof(Graph1Context)),
        create : &create,
        render : &render,
        render_after : &render_after,
        destroy : &destroy,
    };
}