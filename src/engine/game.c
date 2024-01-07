
#include "game.h"

#include "mem/alloc.h"
#include "math/scalar.h"

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
    game->size.x = (float)width * 0.5f;
    game->size.y = (float)height * 0.5f;
#else
    game->size.x = (float)width;
    game->size.y = (float)height;
#endif
    game->ratio = game->size.x / game->size.y;
    glViewport(0, 0, width, height);
}

void game_init()
{
    gtime = (Time *)xxarena(sizeof(Time));
    gtime->delta = 1 / 60.0f;
    gtime->elapsed = 0;

    game = (Game *)xxarena(sizeof(Game));
    game->fps = 60;
    game->size.x = 1200;
    game->size.y = 780;
    game->ratio = game->size.x / game->size.y;
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
    game->screen.x = (float)mode->width;
    game->screen.y = (float)mode->height;

    if (game->full_screen)
    {
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
        game->size.x = game->screen.x;
        game->size.y = game->screen.y;
    }

    game->window = glfwCreateWindow((int)game->size.x, (int)game->size.y, "Game",
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
    int f = (int)(floof(gtime->elapsed));
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