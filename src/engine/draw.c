
#include "draw.h"

#define GLFW_INCLUDE_NONE

#include "glad.h"
#include <string.h>
#include "mathf.h"

#include "shader.h"
#include "camera.h"

enum
{
    types_n = 3,
    object_count = 50000,
};

typedef struct
{
    GLuint vaoIds[types_n];
    GLuint vboIds[types_n];
    int types[types_n];
    int counter[types_n];
    Shader shader;
    Vertex vertices[types_n][object_count];
} DrawData;

static DrawData *drawData;

void draw_init()
{
    drawData = (DrawData *)arena_alloc(alloc->global, sizeof(DrawData), sizeof(size_t));
    memset(drawData, 0, sizeof(DrawData));

    drawData->types[0] = GL_POINTS;
    drawData->types[1] = GL_LINES;
    drawData->types[2] = GL_TRIANGLES;

    drawData->counter[0] = 0;
    drawData->counter[1] = 0;
    drawData->counter[2] = 0;

    drawData->shader = shader_load("shaders/draw.vs", "shaders/draw.fs");
    glGenVertexArrays(types_n, drawData->vaoIds);
    glGenBuffers(types_n, drawData->vboIds);

    for (int i = 0; i < types_n; i++)
    {
        glBindVertexArray(drawData->vaoIds[i]);
        glBindBuffer(GL_ARRAY_BUFFER, drawData->vboIds[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(drawData->vertices[i]),
                     drawData->vertices[i], GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, size));

        drawData->counter[i] = 0;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void draw_render()
{
    shader_begin(drawData->shader);
    shader_mat4(drawData->shader, "projection", &camera->projection);
    shader_mat4(drawData->shader, "view", &camera->view);

    glLineWidth(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glBlendEquation(GL_ADD);

    for (int i = 0; i < types_n; i++)
    {
        int count = drawData->counter[i];
        if (count == 0)
            continue;

        glBindVertexArray(drawData->vaoIds[i]);
        glBindBuffer(GL_ARRAY_BUFFER, drawData->vboIds[i]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * (GLsizeiptr)sizeof(Vertex), drawData->vertices[i]);
        glDrawArrays(drawData->types[i], 0, count);

        drawData->counter[i] = 0;
    }

    shader_end();
}

void draw_terminate()
{
    glDeleteVertexArrays(types_n, drawData->vaoIds);
    glDeleteBuffers(types_n, drawData->vboIds);
    shader_destroy(drawData->shader);
}

void add_vertex(int type, Vertex v)
{
    if (drawData->counter[type] == object_count)
        return;
    int count = drawData->counter[type];
    drawData->vertices[type][count] = v;
    drawData->counter[type]++;
}

void draw_point(Vec3 pos, float size, Color c)
{
    Vertex v;
    v.color = c;
    v.position = pos;
    v.size = size;
    add_vertex(0, v);
}

void draw_line(Vec3 a, Vec3 b, Color c)
{
    Vertex va;
    va.color = c;
    va.position = a;
    add_vertex(1, va);
    va.position = b;
    add_vertex(1, va);
}

void draw_bbox(BBox bbox, Color c)
{
    Vec3 vertices[8];
    bbox_vertices(bbox, vertices);

    Vertex va;
    va.color = c;

    for (int i = 0; i < 4; i++)
    {
        va.position = vertices[i];
        add_vertex(1, va);
        va.position = vertices[(i + 1) % 4];
        add_vertex(1, va);

        va.position = vertices[i + 4];
        add_vertex(1, va);
        va.position = vertices[(i + 1) % 4 + 4];
        add_vertex(1, va);

        va.position = vertices[i];
        add_vertex(1, va);
        va.position = vertices[i + 4];
        add_vertex(1, va);
    }
}

static inline void draw_face(Vertex *va, const Vec3 *a, const Vec3 *b, const Vec3 *c, const Vec3 *d)
{
    va->position = *a;
    add_vertex(1, *va);
    va->position = *b;
    add_vertex(1, *va);

    va->position = *b;
    add_vertex(1, *va);
    va->position = *c;
    add_vertex(1, *va);

    va->position = *c;
    add_vertex(1, *va);
    va->position = *d;
    add_vertex(1, *va);

    va->position = *d;
    add_vertex(1, *va);
    va->position = *a;
    add_vertex(1, *va);
}

static inline void fill_face(Vertex *va, const Vec3 *a, const Vec3 *b, const Vec3 *c, const Vec3 *d)
{
    va->position = *a;
    add_vertex(2, *va);
    va->position = *b;
    add_vertex(2, *va);
    va->position = *c;
    add_vertex(2, *va);

    va->position = *a;
    add_vertex(2, *va);
    va->position = *c;
    add_vertex(2, *va);
    va->position = *d;
    add_vertex(2, *va);
}

void fill_bbox(BBox bbox, Color c)
{
    Vec3 vertices[8];
    bbox_vertices(bbox, vertices);

    Vertex va;
    va.color = c;
    fill_face(&va, &vertices[0], &vertices[1], &vertices[2], &vertices[3]);
    fill_face(&va, &vertices[7], &vertices[6], &vertices[5], &vertices[4]);
    fill_face(&va, &vertices[2], &vertices[6], &vertices[7], &vertices[3]);
    fill_face(&va, &vertices[0], &vertices[4], &vertices[5], &vertices[1]);
    fill_face(&va, &vertices[1], &vertices[5], &vertices[6], &vertices[2]);
    fill_face(&va, &vertices[3], &vertices[7], &vertices[4], &vertices[0]);
}

void draw_quad(Quad q, Color cl)
{
    Vertex va;
    va.color = cl;
    draw_face(&va, &q.a, &q.b, &q.c, &q.d);
}

void fill_quad(Quad q, Color cl)
{
    Vertex va;
    va.color = cl;
    fill_face(&va, &q.a, &q.b, &q.c, &q.d);
}

void draw_cube(Vec3 a, Vec3 s, Color c)
{
    Vec3 size = vec3_mulf(s, 0.5f);
    draw_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}
void fill_cube(Vec3 a, Vec3 s, Color c)
{
    Vec3 size = vec3_mulf(s, 0.5f);
    fill_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}

void draw_cubef(Vec3 a, float s, Color c)
{
    Vec3 size = vec3f(s * 0.5f);
    draw_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}
void fill_cubef(Vec3 a, float s, Color c)
{
    Vec3 size = vec3f(s * 0.5f);
    fill_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}

void draw_edge(Edge e, Color c)
{
    Vertex va;
    va.color = c;
    va.position = e.a;
    add_vertex(1, va);
    va.position = e.b;
    add_vertex(1, va);
}

void draw_triangle(Triangle t, Color c)
{
    Vertex va;
    va.color = c;
    va.position = t.a;
    add_vertex(1, va);
    va.position = t.b;
    add_vertex(1, va);

    va.position = t.b;
    add_vertex(1, va);
    va.position = t.c;
    add_vertex(1, va);

    va.position = t.c;
    add_vertex(1, va);
    va.position = t.a;
    add_vertex(1, va);
}

void fill_triangle(Triangle t, Color c)
{
    Vertex va;
    va.color = c;
    va.position = t.c;
    add_vertex(2, va);

    va.position = t.b;
    add_vertex(2, va);

    va.position = t.a;
    add_vertex(2, va);
}

void draw_tetrahedron(Tetrahedron t, Color c)
{
    Vertex va;
    va.color = c;
    va.position = t.a;
    add_vertex(1, va);
    va.position = t.b;
    add_vertex(1, va);

    va.position = t.b;
    add_vertex(1, va);
    va.position = t.c;
    add_vertex(1, va);

    va.position = t.c;
    add_vertex(1, va);
    va.position = t.a;
    add_vertex(1, va);

    va.position = t.a;
    add_vertex(1, va);
    va.position = t.d;
    add_vertex(1, va);

    va.position = t.b;
    add_vertex(1, va);
    va.position = t.d;
    add_vertex(1, va);

    va.position = t.c;
    add_vertex(1, va);
    va.position = t.d;
    add_vertex(1, va);
}

void fill_tetrahedron(Tetrahedron t, Color c)
{
    Vertex va;
    va.color = c;

    va.position = t.c;
    add_vertex(2, va);
    va.position = t.b;
    add_vertex(2, va);
    va.position = t.a;
    add_vertex(2, va);

    va.position = t.d;
    add_vertex(2, va);
    va.position = t.c;
    add_vertex(2, va);
    va.position = t.a;
    add_vertex(2, va);

    va.position = t.b;
    add_vertex(2, va);
    va.position = t.c;
    add_vertex(2, va);
    va.position = t.d;
    add_vertex(2, va);

    va.position = t.a;
    add_vertex(2, va);
    va.position = t.b;
    add_vertex(2, va);
    va.position = t.d;
    add_vertex(2, va);
}

void draw_circleXY(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sind(p);
    float cp = cosd(p);
    Vec3 r1 = {1.0f, 0.0f, 0.0f};
    Vec3 v1 = vec3_mulf(r1, r);
    v1 = vec3_add(v1, a);
    for (int i = 0; i < s; i++)
    {
        Vec3 r2;
        r2.x = cp * r1.x - sp * r1.y;
        r2.y = sp * r1.x + cp * r1.y;
        r2.z = 0;
        Vec3 v2 = vec3_add(vec3_mulf(r2, r), a);

        Vertex va;
        va.color = c;
        va.position = v1;
        add_vertex(1, va);
        va.position = v2;
        add_vertex(1, va);

        r1 = r2;
        v1 = v2;
    }
}

void fill_circleXY(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sind(p);
    float cp = cosd(p);
    Vec3 r1 = {1.0f, 0.0f, 0.0f};
    Vec3 v1 = vec3_mulf(r1, r);
    v1 = vec3_add(v1, a);
    for (int i = 0; i < s; i++)
    {
        Vec3 r2;
        r2.x = cp * r1.x - sp * r1.y;
        r2.y = sp * r1.x + cp * r1.y;
        r2.z = 0;
        Vec3 v2 = vec3_add(vec3_mulf(r2, r), a);

        Vertex va;
        va.color = c;
        va.position = v1;
        add_vertex(2, va);
        va.position = v2;
        add_vertex(2, va);
        va.position = a;
        add_vertex(2, va);

        r1 = r2;
        v1 = v2;
    }
}
void draw_circleXZ(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sind(p);
    float cp = cosd(p);
    Vec3 r1 = {1.0f, 0.0f, 0.0f};
    Vec3 v1 = vec3_mulf(r1, r);
    v1 = vec3_add(v1, a);
    for (int i = 0; i < s; i++)
    {
        Vec3 r2;
        r2.x = cp * r1.x - sp * r1.z;
        r2.z = sp * r1.x + cp * r1.z;
        r2.y = 0;
        Vec3 v2 = vec3_add(vec3_mulf(r2, r), a);

        Vertex va;
        va.color = c;
        va.position = v1;
        add_vertex(1, va);
        va.position = v2;
        add_vertex(1, va);

        r1 = r2;
        v1 = v2;
    }
}

void draw_circleYZ(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sind(p);
    float cp = cosd(p);
    Vec3 r1 = {0.0f, 1.0f, 0.0f};
    Vec3 v1 = vec3_mulf(r1, r);
    v1 = vec3_add(v1, a);
    for (int i = 0; i < s; i++)
    {
        Vec3 r2;
        r2.y = cp * r1.y - sp * r1.z;
        r2.z = sp * r1.y + cp * r1.z;
        r2.x = 0;
        Vec3 v2 = vec3_add(vec3_mulf(r2, r), a);

        Vertex va;
        va.color = c;
        va.position = v1;
        add_vertex(1, va);
        va.position = v2;
        add_vertex(1, va);

        r1 = r2;
        v1 = v2;
    }
}

void fill_circleYZ(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sind(p);
    float cp = cosd(p);
    Vec3 r1 = {0.0f, 1.0f, 0.0f};
    Vec3 v1 = vec3_mulf(r1, r);
    v1 = vec3_add(v1, a);
    for (int i = 0; i < s; i++)
    {
        Vec3 r2;
        r2.y = cp * r1.y - sp * r1.z;
        r2.z = sp * r1.y + cp * r1.z;
        r2.x = 0;
        Vec3 v2 = vec3_add(vec3_mulf(r2, r), a);

        Vertex va;
        va.color = c;
        va.position = v1;
        add_vertex(2, va);
        va.position = v2;
        add_vertex(2, va);
        va.position = a;
        add_vertex(2, va);

        r1 = r2;
        v1 = v2;
    }
}
void draw_sphere(Vec3 a, float r, Color c, int s)
{
    draw_circleXZ(a, r, c, s);
    draw_circleXY(a, r, c, s);
    draw_circleYZ(a, r, c, s);
}

void draw_arrow(Vec3 a, Vec3 b, Vec3 up, Color c, float p)
{
    float d = vec3_dist(a, b);
    Vec3 dirX = vec3_norm(vec3_sub(b, a));
    Vec3 dirY = vec3_cross(vec3_norm(up), dirX);
    Vec3 dirZ = vec3_cross(dirX, dirY);
    Vec3 forward = vec3_add(a, vec3_mulf(dirX, d));
    Vec3 end;
    Tetrahedron t;

    end = vec3_add(a, vec3_mulf(dirX, d - p));
    draw_line(a, forward, c);
    t = tetrahedron(forward, end, end, end);
    t.c = vec3_add(t.c, vec3_mulf(dirZ, p * 0.25f));
    t.d = vec3_add(t.d, vec3_mulf(dirY, p * 0.25f));
    fill_tetrahedron(t, c);
}

void draw_ray(Ray r, Color c)
{
    draw_arrow(r.origin, vec3_add(r.origin, r.direction), vec3_up, c,
               vec3_mag(r.direction) * 0.25f);
}

void draw_axis(Vec3 a, float s, Quat q)
{
    Vec3 dirX = quat_forward(q);
    Vec3 dirY = quat_right(q);
    Vec3 dirZ = quat_up(q);
    Vec3 forward = vec3_add(a, vec3_mulf(dirX, s));
    Vec3 right = vec3_add(a, vec3_mulf(dirY, s));
    Vec3 up = vec3_add(a, vec3_mulf(dirZ, s));

    draw_arrow(a,
               forward,
               vec3_add(dirX, dirZ),
               color_red,
               s * 0.2f);
    draw_arrow(a,
               right,
               vec3_add(dirY, dirX),
               color_green,
               s * 0.2f);
    draw_arrow(a,
               up,
               vec3_add(dirZ, dirY),
               color_blue,
               s * 0.2f);
}

void draw_axisRot(Vec3 a, float s, Rot r)
{
    Vec3 dirX = rot_forward(r);
    Vec3 dirY = rot_right(r);
    Vec3 dirZ = rot_up(r);
    Vec3 forward = vec3_add(a, vec3_mulf(dirX, s));
    Vec3 right = vec3_add(a, vec3_mulf(dirY, s));
    Vec3 up = vec3_add(a, vec3_mulf(dirZ, s));

    draw_arrow(a,
               forward,
               vec3_add(dirX, dirZ),
               color_red,
               s * 0.2f);
    draw_arrow(a,
               right,
               vec3_add(dirY, dirX),
               color_green,
               s * 0.2f);
    draw_arrow(a,
               up,
               vec3_add(dirZ, dirY),
               color_blue,
               s * 0.2f);
}

void draw_frustum(Vec3 pos, Rot rt, float fov, float ratio, float nr, float fr, Color c)
{
    Mat4 view = mat4_view(pos, rt);
    Mat4 projection = mat4_perspective(fov, ratio, nr, fr);

    Mat4 projView = mat4_inv(mat4_mul(view, projection));

    Vec3 points[8] = {{-1, -1, 0},
                      {1, -1, 0},
                      {1, 1, 0},
                      {-1, 1, 0},
                      {-1, -1, 1},
                      {1, -1, 1},
                      {1, 1, 1},
                      {-1, 1, 1}};

    for (int i = 0; i < 8; i++)
    {
        Vec3 *p = &points[i];
        Vec4 v = {p->x, p->y, p->z, 1.0f};
        v = mat4_mulv4(projView, v);
        p->x = v.x / v.w;
        p->y = v.y / v.w;
        p->z = v.z / v.w;
    }

    Vertex va;
    va.color = color_alpha(c, 0.4f);
    fill_face(&va, &points[0], &points[1], &points[2], &points[3]);

    draw_line(pos, points[0], c);
    draw_line(pos, points[1], c);
    draw_line(pos, points[2], c);
    draw_line(pos, points[3], c);

    draw_line(points[0], points[1], c);
    draw_line(points[1], points[2], c);
    draw_line(points[2], points[3], c);
    draw_line(points[3], points[0], c);

    draw_line(points[4], points[5], c);
    draw_line(points[5], points[6], c);
    draw_line(points[6], points[7], c);
    draw_line(points[7], points[4], c);

    draw_line(points[0], points[4], c);
    draw_line(points[1], points[5], c);
    draw_line(points[2], points[6], c);
    draw_line(points[3], points[7], c);
}