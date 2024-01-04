#include "level.h"

#include "pthread.h"

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
    pthread_mutex_t mutex;
} LevelManager;

static LevelManager *manager;

void level_init(int n)
{
    manager = (LevelManager *)arena_alloc(alloc->global, sizeof(LevelManager), sizeof(size_t));
    manager->n = 0;
    manager->locked = 0;
    manager->current = -1;
    manager->prev = -1;
    manager->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    manager->levels = (Level *)arena_alloc(alloc->global, sizeof(Level) * n, sizeof(size_t));
}

void level_add(Level level)
{
    manager->levels[manager->n++] = level;
}

void level_activate(int i)
{
    if (manager->locked)
        return;
    pthread_mutex_lock(&manager->mutex);
    if (i >= manager->n || i < 0)
        return;
    manager->current = i;
    pthread_mutex_unlock(&manager->mutex);
}

void level_prev()
{
    if (manager->locked)
        return;
    pthread_mutex_lock(&manager->mutex);
    manager->current--;
    if (manager->current < 0)
        manager->current = manager->n - 1;

    pthread_mutex_unlock(&manager->mutex);
}
void level_next()
{
    if (manager->locked)
        return;
    manager->current++;
    if (manager->current >= manager->n)
        manager->current = 0;
}

static void *proceed_destroy_and_create(void*)
{
    pthread_mutex_lock(&manager->mutex);
    Level *level;
    if (manager->prev != -1)
    {
        level = &manager->levels[manager->prev];
        level->destroy(level->context);
    }
    if (manager->current != -1)
    {
        level = &manager->levels[manager->current];
        level->create(level->context);
    }
    manager->prev = manager->current;
    manager->locked = 0;
    pthread_mutex_unlock(&manager->mutex);
}

void level_render()
{
    if (manager->locked)
    {
        debug_scale(2);
        debug_origin(vec2(0.5, 0.5));
        debug_string(vec2(game->width / 2, game->height / 2), "Loading...", 11);
        debug_scale(1);
        return;
    }
    if (manager->current != manager->prev)
    {
        manager->locked = 1;
        pthread_t pt;
        pthread_create(&pt, NULL, &proceed_destroy_and_create, NULL);
        // proceed_destroy_and_create(NULL);
        return;
    }
    if (manager->current != -1)
    {
        Level *level = &manager->levels[manager->current];
        level->render(level->context);
    }
}

void level_destroy()
{
    if (manager->prev != -1)
    {
        Level *level = &manager->levels[manager->prev];
        level->destroy(level->context);
    }
}