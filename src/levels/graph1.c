#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/mathf.h"
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/noise.h"
#include "mem/alloc.h"
#include "mem/std.h"

typedef struct
{
} Graph1Context;

static void create(Graph1Context *self)
{
}

static void update(Graph1Context *self)
{
}

static void destroy(Graph1Context *self)
{
}

Level make_graph1()
{
    return (Level){
        context : arena_alloc(alloc->global, sizeof(Graph1Context), sizeof(size_t)),
        create : &create,
        update : &update,
        destroy : &destroy,
    };
}