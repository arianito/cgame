

#include "input.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string.h>

#include "mem/alloc.h"
#include "game.h"
#include "math/scalar.h"

#define AXIS_SPEED 8

typedef struct
{
    float value;
    float velocity;
} InputAxis;

typedef struct
{
    int current;
    int prev;
} InputState;

typedef struct
{
    InputState keyState[KEY_COUNT];
    InputState mouseState[MOUSE_COUNT];
    InputAxis axes[AXIS_COUNT];
} InputData;

static InputData *globalInput;
Input *input;

void update_axis(int ax, int low, int high)
{
    float to = 0.0f + (low ? -1.0f : 0.0f) + (high ? 1.0f : 0.0f);
    InputAxis *axis = &(globalInput->axes[ax]);
    axis->value = move_towardsf(axis->value, to, AXIS_SPEED * gtime->delta);
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    input->wheel.x = (float)x;
    input->wheel.y = (float)y;
}

void input_init()
{
    globalInput = (InputData *)xxarena(sizeof(InputData));
    memset(globalInput, 0, sizeof(InputData));

    input = (Input *)xxarena(sizeof(Input));
    memset(input, 0, sizeof(Input));

    glfwSetScrollCallback(game->window, scroll_callback);
}

void input_update()
{
    double x, y;
    glfwGetCursorPos(game->window, &x, &y);
    input->delta.x = (float)x - input->position.x;
    input->delta.y = (float)y - input->position.y;
    input->position.x = (float)x;
    input->position.y = (float)y;

    input->wheel.x = move_towardsf(input->wheel.x, 0, 0.2f);
    input->wheel.y = move_towardsf(input->wheel.y, 0, 0.2f);

    for (int i = 0; i < KEY_COUNT; i++)
    {
        InputState *state = &(globalInput->keyState[i]);
        state->prev = state->current;
    }
    for (int i = 0; i < MOUSE_COUNT; i++)
    {
        InputState *state = &(globalInput->mouseState[i]);
        state->prev = state->current;
    }

    update_axis(AXIS_HORIZONTAL,
                input_keypress(KEY_LEFT) || input_keypress(KEY_A),
                input_keypress(KEY_RIGHT) || input_keypress(KEY_D));

    update_axis(AXIS_VERTICAL,
                input_keypress(KEY_DOWN) || input_keypress(KEY_S),
                input_keypress(KEY_UP) || input_keypress(KEY_W));
}

void input_infinite()
{
    float pad = 10;
    char changed = 0;
    float width = game->size.x;
    float height = game->size.y;
    if (input->delta.x < 0 && input->position.x < pad)
    {
        input->position.x = width;
        if (input->delta.x < -(width - pad))
            input->delta.x += width - pad;
        changed = 1;
    }
    if (input->delta.x > 0 && input->position.x > width - pad)
    {
        input->position.x = 0;
        if (input->delta.x > width - pad)
            input->delta.x -= width - pad;
        changed = 1;
    }
    if (input->delta.y < 0 && input->position.y < pad)
    {
        input->position.y = height;
        if (input->delta.y < -(height - pad))
            input->delta.y += height - pad;
        changed = 1;
    }
    if (input->delta.y > 0 && input->position.y > height - pad)
    {
        input->position.y = 0;
        if (input->delta.y > (height - pad))
            input->delta.y -= (height - pad);
        changed = 1;
    }
    if (changed)
    {
        glfwSetCursorPos(game->window, input->position.x, input->position.y);
    }
}

int input_keypress(KeyEnum key)
{
    InputState *state = &globalInput->keyState[key];
    state->current = glfwGetKey(game->window, key);
    return state->current;
}

int input_keyup(KeyEnum key)
{
    InputState *state = &globalInput->keyState[key];
    state->current = glfwGetKey(game->window, key);
    return (!(state->current) && state->prev);
}

int input_keydown(KeyEnum key)
{
    InputState *state = &globalInput->keyState[key];
    state->current = glfwGetKey(game->window, key);
    return state->current && !state->prev;
}

int input_mousepress(MouseEnum key)
{
    InputState *state = &globalInput->mouseState[key];
    state->current = glfwGetMouseButton(game->window, key);
    return state->current;
}

int input_mouseup(MouseEnum key)
{
    InputState *state = &globalInput->mouseState[key];
    state->current = glfwGetMouseButton(game->window, key);
    return !state->current && state->prev;
}

int input_mousedown(MouseEnum key)
{
    InputState *state = &globalInput->mouseState[key];
    state->current = glfwGetMouseButton(game->window, key);
    return state->current && !state->prev;
}

float input_axis(AxisEnum a)
{
    InputAxis *state = &(globalInput->axes[a]);
    return state->value;
}
