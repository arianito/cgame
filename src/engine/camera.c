#include "camera.h"

Camera *camera;

void camera_update()
{
    if (!(camera->ortho & VIEW_ORTHOGRAPHIC))
    {
        camera->view = mat4_view(camera->position, camera->rotation);
        camera->projection = mat4_perspective(camera->fov, game->ratio, 1.0f, camera->farPlane);
    }
    else
    {
        Rot r = camera->rotation;
        float height = camera->zoom * (camera->fov * 0.005556f);
        float width = height * game->ratio;
        camera->projection = mat4_orthographic(-width, width, -height, height, -camera->farPlane, camera->farPlane);
        camera->view = mat4_view(camera->position, r);
    }
    camera->view_projection = mat4_mul(camera->view, camera->projection);
}

void camera_init()
{
    camera = (Camera *)arena_alloc(alloc->global, sizeof(Camera), sizeof(size_t));
    camera->rotation = rot(-40, 0, 0);
    camera->farPlane = 10000;
    camera->zoom = 100.0f;
    Vec3 backward = vec3_mulf(rot_forward(camera->rotation), -camera->zoom);
    camera->position = vec3_add(backward, vec3_zero);
    camera->fov = 80.0f;
    camera->ortho = VIEW_INITIAL;
    camera_update();
}

Vec2 camera_worldToScreen(Vec3 p)
{
    Vec4 r = mat4_mulv4(camera->view_projection, vec4(p.x, p.y, p.z, 1));
    if (r.w > 0)
    {
        float rhw = 1.0f / r.w;
        float rx = r.x * rhw;
        float ry = r.y * rhw;
        float nx = (rx / 2.f) + 0.5f;
        float ny = 1.f - (ry / 2.f) - 0.5f;
        return vec2(nx * game->width, ny * game->height);
    }
    return vec2(MAX, MAX);
}

Ray camera_screenToWorld(Vec2 s)
{
    Mat4 inv = mat4_inv(camera->view_projection);
    float nx = s.x / game->width;
    float ny = s.y / game->height;
    float ssx = (nx - 0.5f) * 2.0f;
    float ssy = ((1.0f - ny) - 0.5f) * 2.0f;
    Vec4 rsp = vec4(ssx, ssy, 1.0f, 1.0f);
    Vec4 rep = vec4(ssx, ssy, 0.01f, 1.0f);

    Vec4 hrs = mat4_mulv4(inv, rsp);
    Vec4 hre = mat4_mulv4(inv, rep);
    Vec3 rsw = {hrs.x, hrs.y, hrs.z};
    Vec3 rew = {hre.x, hre.y, hre.z};

    if (hrs.w != 0.0f)
    {
        rsw.x /= hrs.w;
        rsw.y /= hrs.w;
        rsw.z /= hrs.w;
    }
    if (hre.w != 0.0f)
    {
        rew.x /= hre.w;
        rew.y /= hre.w;
        rew.z /= hre.w;
    }

    Ray r;
    r.origin = rew;
    r.direction = vec3_sub(rew, rsw);
    return r;
}
