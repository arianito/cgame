
#include "draw.h"

#include <string.h>
#include "shader.h"
#include "camera.h"
#include "glad.h"

#include "math/scalar.h"

enum
{
    types_n = 4,
    object_count = 50000,
};

typedef struct
{
    Vec3 position;
    Color color;
    float size;
} Vertex;

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
    drawData->types[3] = GL_TRIANGLES;

    drawData->counter[0] = 0;
    drawData->counter[1] = 0;
    drawData->counter[2] = 0;
    drawData->counter[3] = 0;

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
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glBlendEquation(GL_ADD);

    for (int i = 0; i < types_n; i++)
    {
        int count = drawData->counter[i];
        if (count == 0)
            continue;
        if (i == 3)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);

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

static void add_vertex(int type, Vertex v)
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

void draw_normal(Vec3 a, Vec3 n, float scale, Color c)
{

    Vertex va;
    va.color = c;
    va.position = a;
    add_vertex(1, va);
    va.position = vec3_add(a, vec3_mulf(vec3_norm(n), scale));
    add_vertex(1, va);
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

static void draw_face(Vertex *va, const Vec3 *a, const Vec3 *b, const Vec3 *c, const Vec3 *d)
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

static void fill_face(Vertex *va, const Vec3 *a, const Vec3 *b, const Vec3 *c, const Vec3 *d, bool cull)
{
    const int vz = cull ? 2 : 3;
    va->position = *a;
    add_vertex(vz, *va);
    va->position = *b;
    add_vertex(vz, *va);
    va->position = *c;
    add_vertex(vz, *va);

    va->position = *a;
    add_vertex(vz, *va);
    va->position = *c;
    add_vertex(vz, *va);
    va->position = *d;
    add_vertex(vz, *va);
}

void fill_bbox(BBox bbox, Color c, bool cull)
{
    Vec3 vertices[8];
    bbox_vertices(bbox, vertices);

    Vertex va;
    va.color = c;
    fill_face(&va, &vertices[0], &vertices[1], &vertices[2], &vertices[3], cull);
    fill_face(&va, &vertices[7], &vertices[6], &vertices[5], &vertices[4], cull);
    fill_face(&va, &vertices[2], &vertices[6], &vertices[7], &vertices[3], cull);
    fill_face(&va, &vertices[0], &vertices[4], &vertices[5], &vertices[1], cull);
    fill_face(&va, &vertices[1], &vertices[5], &vertices[6], &vertices[2], cull);
    fill_face(&va, &vertices[3], &vertices[7], &vertices[4], &vertices[0], cull);
}

void draw_quad(Quad q, Color cl)
{
    Vertex va;
    va.color = cl;
    draw_face(&va, &q.a, &q.b, &q.c, &q.d);
}

void fill_quad(Quad q, Color cl, bool cull)
{
    Vertex va;
    va.color = cl;
    fill_face(&va, &q.a, &q.b, &q.c, &q.d, cull);
}

void draw_aabb_yz(AABB a, Color cl)
{
    draw_quad(quad(
                  vec3(0, a.min.x, a.min.y),
                  vec3(0, a.max.x, a.min.y),
                  vec3(0, a.max.x, a.max.y),
                  vec3(0, a.min.x, a.max.y)),
              cl);
}
void fill_aabb_yz(AABB a, Color cl, bool cull)
{
    fill_quad(quad(
                  vec3(0, a.min.x, a.min.y),
                  vec3(0, a.max.x, a.min.y),
                  vec3(0, a.max.x, a.max.y),
                  vec3(0, a.min.x, a.max.y)),
              cl, cull);
}

void draw_cube(Vec3 a, Vec3 s, Color c)
{
    Vec3 size = vec3_mulf(s, 0.5f);
    draw_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}
void fill_cube(Vec3 a, Vec3 s, Color c, bool cull)
{
    Vec3 size = vec3_mulf(s, 0.5f);
    fill_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c, cull);
}

void draw_cubef(Vec3 a, float s, Color c)
{
    Vec3 size = vec3f(s * 0.5f);
    draw_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c);
}
void fill_cubef(Vec3 a, float s, Color c, bool cull)
{
    Vec3 size = vec3f(s * 0.5f);
    fill_bbox(bbox(vec3_sub(a, size), vec3_add(a, size)), c, cull);
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

void fill_triangle(Triangle t, Color c, bool cull)
{
    Vertex va;
    va.color = c;
    va.position = t.c;
    const int vz = cull ? 2 : 3;
    add_vertex(vz, va);

    va.position = t.b;
    add_vertex(vz, va);

    va.position = t.a;
    add_vertex(vz, va);
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

void fill_tetrahedron(Tetrahedron t, Color c, bool cull)
{
    Vertex va;
    va.color = c;

    const int vz = cull ? 2 : 3;

    va.position = t.c;
    add_vertex(vz, va);
    va.position = t.b;
    add_vertex(vz, va);
    va.position = t.a;
    add_vertex(vz, va);

    va.position = t.d;
    add_vertex(vz, va);
    va.position = t.c;
    add_vertex(vz, va);
    va.position = t.a;
    add_vertex(vz, va);

    va.position = t.b;
    add_vertex(vz, va);
    va.position = t.c;
    add_vertex(vz, va);
    va.position = t.d;
    add_vertex(vz, va);

    va.position = t.a;
    add_vertex(vz, va);
    va.position = t.b;
    add_vertex(vz, va);
    va.position = t.d;
    add_vertex(vz, va);
}

void draw_circle_xy(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sindf(p);
    float cp = cosdf(p);
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

void fill_circle_xy(Vec3 a, float r, Color c, int s, bool cull)
{
    const int vz = cull ? 2 : 3;
    float p = 360.0f / (float)s;
    float sp = sindf(p);
    float cp = cosdf(p);
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
        add_vertex(vz, va);
        va.position = v2;
        add_vertex(vz, va);
        va.position = a;
        add_vertex(vz, va);

        r1 = r2;
        v1 = v2;
    }
}
void draw_circle_xz(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sindf(p);
    float cp = cosdf(p);
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

void draw_circle_yz(Vec3 a, float r, Color c, int s)
{
    float p = 360.0f / (float)s;
    float sp = sindf(p);
    float cp = cosdf(p);
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

void fill_circle_yz(Vec3 a, float r, Color c, int s, bool cull)
{
    const int vz = cull ? 2 : 3;
    float p = 360.0f / (float)s;
    float sp = sindf(p);
    float cp = cosdf(p);
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
        add_vertex(vz, va);
        va.position = v2;
        add_vertex(vz, va);
        va.position = a;
        add_vertex(vz, va);

        r1 = r2;
        v1 = v2;
    }
}
void draw_sphere(Vec3 a, float r, Color c, int s)
{
    draw_circle_xz(a, r, c, s);
    draw_circle_xy(a, r, c, s);
    draw_circle_yz(a, r, c, s);
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
    fill_tetrahedron(t, c, true);
}

void draw_ray(Ray r, Color c)
{
    draw_arrow(r.origin, vec3_add(r.origin, r.direction), vec3_up, c, vec3_length(r.direction) * 0.25f);
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
    fill_face(&va, &points[0], &points[1], &points[2], &points[3], true);

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

void draw_capsule_yz(Vec3 p1, Vec3 p2, float radius, Color color, int seg)
{

    Vertex va;
    va.color = color;

    float length;
    Vec2 axis = vec2_length_normal(&length, vec2_sub(vec2yz(p2), vec2yz(p1)));

    if (near0f(length))
    {
        draw_circle_yz(p1, radius, color, seg);
        return;
    }

    const float k_increment = 180.0 / seg;
    float sinInc = sindf(k_increment);
    float cosInc = cosdf(k_increment);

    Vec2 r1 = vec2(-axis.y, axis.x);
    Vec2 v1 = vec2_mul_add(vec2yz(p1), radius, r1);
    Vec2 a = v1;
    for (int32_t i = 0; i < seg; ++i)
    {
        // Perform rotation to avoid additional trigonometry.
        Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        Vec2 v2 = vec2_mul_add(vec2yz(p1), radius, r2);
        va.position = vec3yz(v1);
        add_vertex(1, va);
        va.position = vec3yz(v2);
        add_vertex(1, va);
        r1 = r2;
        v1 = v2;
    }
    Vec2 b = v1;

    r1 = vec2(axis.y, -axis.x);
    v1 = vec2_mul_add(vec2yz(p2), radius, r1);
    Vec2 c = v1;
    for (int32_t i = 0; i < seg; ++i)
    {
        // Perform rotation to avoid additional trigonometry.
        Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        Vec2 v2 = vec2_mul_add(vec2yz(p2), radius, r2);
        va.position = vec3yz(v1);
        add_vertex(1, va);
        va.position = vec3yz(v2);
        add_vertex(1, va);
        r1 = r2;
        v1 = v2;
    }
    Vec2 d = v1;

    va.position = vec3yz(a);
    add_vertex(1, va);
    va.position = vec3yz(d);
    add_vertex(1, va);

    va.position = vec3yz(b);
    add_vertex(1, va);
    va.position = vec3yz(c);
    add_vertex(1, va);

    va.position = p1;
    add_vertex(1, va);
    va.position = p2;
    add_vertex(1, va);
}

void fill_capsule_yz(Vec3 p1, Vec3 p2, float radius, Color color, int seg, bool cull)
{
    const int vz = cull ? 2 : 3;

    Vertex va;
    va.color = color;

    float length;
    Vec2 axis = vec2_length_normal(&length, vec2_sub(vec2yz(p2), vec2yz(p1)));

    if (near0f(length))
    {
        fill_circle_yz(p1, radius, color, seg, cull);
        return;
    }

    const float k_increment = 180 / seg;

    float sinInc = sindf(k_increment);
    float cosInc = cosdf(k_increment);

    Vec2 r1 = {-axis.y, axis.x};
    Vec2 v1 = vec2_mul_add(vec2yz(p1), radius, r1);
    Vec2 a = v1;
    for (int32_t i = 0; i < seg; ++i)
    {
        // Perform rotation to avoid additional trigonometry.
        Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        Vec2 v2 = vec2_mul_add(vec2yz(p1), radius, r2);

        va.position = vec3yz(v2);
        add_vertex(vz, va);
        va.position = vec3yz(v1);
        add_vertex(vz, va);
        va.position = p1;
        add_vertex(vz, va);

        r1 = r2;
        v1 = v2;
    }
    Vec2 b = v1;

    r1 = vec2(axis.y, -axis.x);
    v1 = vec2_mul_add(vec2yz(p2), radius, r1);
    Vec2 c = v1;
    for (int32_t i = 0; i < seg; ++i)
    {
        // Perform rotation to avoid additional trigonometry.
        Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        Vec2 v2 = vec2_mul_add(vec2yz(p2), radius, r2);

        va.position = vec3yz(v2);
        add_vertex(vz, va);
        va.position = vec3yz(v1);
        add_vertex(vz, va);
        va.position = p2;
        add_vertex(vz, va);

        r1 = r2;
        v1 = v2;
    }
    Vec2 d = v1;

    va.position = vec3yz(c);
    add_vertex(vz, va);
    va.position = vec3yz(b);
    add_vertex(vz, va);
    va.position = vec3yz(a);
    add_vertex(vz, va);

    va.position = vec3yz(c);
    add_vertex(vz, va);
    va.position = vec3yz(a);
    add_vertex(vz, va);
    va.position = vec3yz(d);
    add_vertex(vz, va);
}

void draw_polygon(const Vec3 vertices[], int n, Color color)
{
    Vertex va;
    va.color = color;
    Vec3 p1 = vertices[n - 1];
    for (int32_t i = 0; i < n; ++i)
    {
        Vec3 p2 = vertices[i];
        va.position = p1;
        add_vertex(1, va);
        va.position = p2;
        add_vertex(1, va);

        p1 = p2;
    }
}

void fill_polygon(const Vec3 vertices[], int n, Color color, bool cull)
{
    const int vz = cull ? 2 : 3;
    Vertex va;
    va.color = color;

	for (int32_t i = 1; i < n - 1; ++i)
	{
        va.position = vertices[0];
        add_vertex(vz, va);
        va.position = vertices[i];
        add_vertex(vz, va);
        va.position = vertices[i + 1];
        add_vertex(vz, va);
	}
}

void draw_polygon_yz(const Vec2 vertices[], int n, Color color) {
    Vertex va;
    va.color = color;
    Vec3 p1 = vec3yz(vertices[n - 1]);
    for (int32_t i = 0; i < n; ++i)
    {
        Vec3 p2 = vec3yz(vertices[i]);
        va.position = p1;
        add_vertex(1, va);
        va.position = p2;
        add_vertex(1, va);

        p1 = p2;
    }
}

void fill_polygon_yz(const Vec2 vertices[], int n, Color color, bool cull) {

    const int vz = cull ? 2 : 3;
    Vertex va;
    va.color = color;
	for (int32_t i = 1; i < n - 1; ++i)
	{
        va.position = vec3yz(vertices[0]);
        add_vertex(vz, va);
        va.position = vec3yz(vertices[i]);
        add_vertex(vz, va);
        va.position = vec3yz(vertices[i + 1]);
        add_vertex(vz, va);
	}
}