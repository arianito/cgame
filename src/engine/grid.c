
#define UNIT_SCALE 10
#include <string.h>
#include "glad.h"
#include "shader.h"
#include "camera.h"
#include "mathf.h"
#include "draw.h"

enum
{
    n = 100,
    d = 10,
    size = n * UNIT_SCALE,
    ne = (8 * n + 4),
};

typedef struct
{
    int displayState;
    int showAxis;
    Shader shader;
    GLuint vaoIds[1];
    GLuint vboIds[1];
    Vertex vertices[ne];
} GridData;

static GridData *gridData;

void grid_init()
{

    gridData = (GridData *)arena_alloc(alloc->global, sizeof(GridData), sizeof(size_t));
    memset(gridData, 0, sizeof(GridData));

    gridData->displayState = 3;

    gridData->shader = shader_load("shaders/grid.vs", "shaders/grid.fs");
    glGenVertexArrays(2, gridData->vaoIds);
    glGenBuffers(2, gridData->vboIds);

    glBindVertexArray(gridData->vaoIds[0]);
    glBindBuffer(GL_ARRAY_BUFFER, gridData->vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridData->vertices), gridData->vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Vertex a;
    Color c;
    int k = 0, i;
    float gap = (float)size / n;

    c = color_alpha(color_white, 0.1f);
    for (i = -n; i < n + 1; i++)
    {
        a.color = c;
        a.position.x = (float)i * gap;
        a.position.y = -size;
        a.position.z = 0;
        gridData->vertices[k++] = a;
        a.position.x = (float)i * gap;
        a.position.y = size;
        a.position.z = 0;
        gridData->vertices[k++] = a;
    }
    for (i = -n; i < n + 1; i++)
    {
        a.color = c;
        a.position.x = -size;
        a.position.y = (float)i * gap;
        a.position.z = 0;
        gridData->vertices[k++] = a;
        a.position.x = size;
        a.position.y = (float)i * gap;
        a.position.z = 0;
        gridData->vertices[k++] = a;
    }
}

void grid_terminate()
{
    glDeleteVertexArrays(2, gridData->vaoIds);
    glDeleteBuffers(2, gridData->vboIds);
    shader_destroy(gridData->shader);
}

void grid_render()
{
    if (input_keydown(KEY_5))
    {
        gridData->displayState = (gridData->displayState + 1) % 4;
    }

    if (gridData->displayState == 3 || gridData->displayState == 0)
    {
        Rot r2 = camera->rotation;
        r2.pitch += 90.0f;
        Rot r3 = camera->rotation;
        r3.pitch += 90.0f;
        r3 = rot_snap(r3, 90);

        Mat4 world1;
        Mat4 world2;
        Vec3 t = vec3_zero;

        shader_begin(gridData->shader);
        shader_mat4(gridData->shader, "projection", &camera->projection);
        shader_mat4(gridData->shader, "view", &camera->view);

        glDisable(GL_DEPTH_TEST);
        glLineWidth(1);
        glBindVertexArray(gridData->vaoIds[0]);
        glBindBuffer(GL_ARRAY_BUFFER, gridData->vboIds[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gridData->vertices), gridData->vertices);
        char isOrtho = (char)(camera->ortho & VIEW_ORTHOGRAPHIC);

        t.x = camera->position.x;
        t.y = camera->position.y;
        t.z = 0;

        float flt = 0.5f;
        flt = (sind(fabsf(fmodf(camera->rotation.pitch, 180)))) * flt;
        if (!isOrtho)
        {
            flt = flt * 0.1f + 0.25f;
        }
        if (isOrtho && rot_nearEq(r2, r3))
        {

            float r = camera->ortho & (VIEW_TOP) ? 1.0f : -1.0f;
            t.x *= r;
            if (camera->ortho & (VIEW_FRONT | VIEW_BACK))
            {
                r = camera->ortho & (VIEW_BACK) ? 1.0f : -1.0f;
                t.x = camera->position.z;
                t.y = r * camera->position.y;
                t.z = 0;
            }
            else if (camera->ortho & (VIEW_LEFT | VIEW_RIGHT))
            {
                r = camera->ortho & (VIEW_RIGHT) ? 1.0f : -1.0f;
                t.x = camera->position.z;
                t.y = r * camera->position.x;
                t.z = 0;
            }
            flt = 0.25f;
            world1 = mat4_mul(mat4_origin(vec3_snap(t, UNIT_SCALE * d)), rot_matrix(r3, vec3_zero));
            world2 = mat4_mul(mat4_scalef(d), world1);
        }
        else
        {
            world1 = mat4_origin(vec3_snap(t, UNIT_SCALE * d));
            world2 = mat4_mul(mat4_scalef(d), world1);
        }
        float factor = (1.0f - clamp01((camera->zoom - UNIT_SCALE * 100) / (UNIT_SCALE * 10.0f)));

        shader_mat4(gridData->shader, "world", &world1);
        shader_float(gridData->shader, "alpha", flt * factor);
        glDrawArrays(GL_LINES, 0, sizeof(gridData->vertices));

        glLineWidth(2);
        shader_mat4(gridData->shader, "world", &world2);
        shader_float(gridData->shader, "alpha", flt * 2);
        glDrawArrays(GL_LINES, 0, sizeof(gridData->vertices));

        glEnable(GL_DEPTH_TEST);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        shader_end();
    }

    if (gridData->displayState == 1 || gridData->displayState == 0)
    {
        draw_axis(vec3_zero, 10, quat_identity);
    }
}
