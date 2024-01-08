#include "level.h"

#include "mem/alloc.h"
#include "engine/debug.h"
#include "engine/game.h"

typedef struct
{
    Level *levels;
    int n;
    int prev;
    int current;
    int locked;
} LevelManager;

static LevelManager *manager;

void level_init(int n)
{
    manager = (LevelManager *)xxarena(sizeof(LevelManager));
    manager->n = 0;
    manager->locked = 0;
    manager->current = -1;
    manager->prev = -1;
    manager->levels = (Level *)xxarena(sizeof(Level) * n);
}

void level_add(Level level)
{
    manager->levels[manager->n++] = level;
}

void level_activate(int i)
{
    if (manager->locked)
        return;
    if (i >= manager->n || i < 0)
        return;
    manager->current = i;
}

void level_prev()
{
    if (manager->locked)
        return;
    manager->current--;
    if (manager->current < 0)
        manager->current = manager->n - 1;
}
void level_next()
{
    if (manager->locked)
        return;
    manager->current++;
    if (manager->current >= manager->n)
        manager->current = 0;
}

static void *proceed_destroy_and_create(void *)
{
    return NULL;
}

void level_render_before()
{
    if (manager->locked)
        return;

    if (manager->current == -1)
        return;

    Level *level = &manager->levels[manager->current];
    if (level->render_before != NULL)
        level->render_before(level->context);
}

void level_render()
{
    if (manager->locked)
        return;

    if (manager->current == -1)
        return;

    if (manager->current != manager->prev)
    {
        manager->locked = 1;
        return;
    }

    Level *level = &manager->levels[manager->current];
    if (level->render != NULL)
        level->render(level->context);
}

void level_render_after()
{
    if (manager->locked)
    {
        debug_scale(2);
        debug_origin(vec2(0.5, 0.5));
        debug_string(vec2(game->size.x / 2, game->size.y / 2), "Loading...", 11);
        debug_reset();

        if (manager->prev != -1)
        {
            Level *level = &manager->levels[manager->prev];
            level->destroy(level->context);
        }
        if (manager->current != -1)
        {
            Level *level = &manager->levels[manager->current];
            level->create(level->context);
        }
        manager->prev = manager->current;
        manager->locked = 0;
        return;
    }

    if (manager->locked)
        return;

    if (manager->current == -1)
        return;

    Level *level = &manager->levels[manager->current];
    if (level->render_after != NULL)
        level->render_after(level->context);
}
void level_destroy()
{
    if (manager->prev != -1)
    {
        Level *level = &manager->levels[manager->prev];
        level->destroy(level->context);
    }
}