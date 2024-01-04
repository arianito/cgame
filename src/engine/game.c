
#include "game.h"

#include "mem/alloc.h"
#include "mathf.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include "glad.h"

Game *game = NULL;
Time *gtime = NULL;
static int alive = 1;
static int resist = 0;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
#if __APPLE__
    game->width = (float)width * 0.5f;
    game->height = (float)height * 0.5f;
#else
    game->width = (float)width;
    game->height = (float)height;
#endif
    game->ratio = game->width / game->height;
    glViewport(0, 0, width, height);
}

void game_init()
{
    gtime = (Time *)arena_alloc(alloc->global, sizeof(Time), sizeof(Time));
    gtime->delta = 1 / 60.0f;
    gtime->elapsed = 0;

    game = (Game *)arena_alloc(alloc->global, sizeof(Game), sizeof(Time));
    game->fps = 60;
    game->width = 1200;
    game->height = 780;
    game->ratio = game->width / game->height;
    game->full_screen = 0;

    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    game->screen_width = (float)mode->width;
    game->screen_height = (float)mode->height;

    if (game->full_screen)
    {
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
        game->width = game->screen_width;
        game->height = game->screen_height;
    }

    game->window = glfwCreateWindow((int)game->width, (int)game->height, "Game",
                                    game->full_screen ? monitor : NULL,
                                    NULL);
    if (!game->window)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(game->window);
    glfwSetFramebufferSizeCallback(game->window, framebuffer_size_callback);

    glfwSetInputMode(game->window, GLFW_CURSOR, GL_TRUE);
    glfwSetInputMode(game->window, GLFW_STICKY_KEYS, GL_TRUE);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return;

    glfwSwapInterval(1);
    glfwSetTime(0);

    GLint w, h;
    glfwGetFramebufferSize(game->window, &w, &h);
    framebuffer_size_callback(game->window, w, h);
}

static int frames = 0;
static int lastCheck = 0;

void game_exit()
{
    alive = 0;
}
void game_resist()
{
    resist = 1;
}

void calculate_fps()
{
    gtime->delta = (float)glfwGetTime() - gtime->elapsed;
    gtime->elapsed = (float)glfwGetTime();
    frames++;
    int f = (int)(floorf(gtime->elapsed));
    if (f != lastCheck)
    {
        game->fps = frames;
        frames = 0;
        lastCheck = f;
    }
}

bool game_loop()
{
    calculate_fps();
    glfwSwapBuffers(game->window);
    glfwPollEvents();

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_ADD);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    if (glfwGetKey(game->window, GLFW_KEY_ESCAPE) && !resist)
        return 0;

    resist = 0;
    return (char)(!glfwWindowShouldClose(game->window)) && alive;
}

void game_terminate()
{
    glfwTerminate();
}