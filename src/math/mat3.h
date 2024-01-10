#pragma once

#include "scalar.h"
#include "vec2_ifc.h"
#include "vec3_ifc.h"

typedef struct
{
    float m[3][3];
} Mat3;

static const Mat3 mat3_identity = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
static const Mat3 mat3_nan = {{{MAX_FLOAT, 0, 0}, {0, MAX_FLOAT, 0}, {0, 0, MAX_FLOAT}}};

#define mat3f(a) ((Mat3){{{a, 0, 0}, {0, a, 0}, {0, 0, a}}})

Mat3 mat3_mul(Mat3 a, Mat3 b);
Vec3 mat3_mulv3(Mat3 a, Vec3 b);
Vec2 mat3_mulv2(Mat3 a, Vec2 b, float w);

static inline Mat3 mat3_rot(float theta)
{
    float ct = cosdf(theta);
    float st = sindf(theta);
    return (Mat3){{
        {ct, -st, 0},
        {st, ct, 0},
        {0, 0, 1},
    }};
}

static inline Mat3 mat3_scale(Vec2 scale)
{
    return (Mat3){{
        {scale.x, 0, 0},
        {0, scale.y, 0},
        {0, 0, 1},
    }};
}
static inline Mat3 mat3_shear(Vec2 shear)
{
    return (Mat3){{
        {1, shear.y, 0},
        {shear.x, 1, 0},
        {0, 0, 1},
    }};
}
static inline Mat3 mat3_translate(Vec2 move)
{
    return (Mat3){{
        {1, 0, 0},
        {0, 1, 0},
        {move.x, move.y, 1},
    }};
}

static inline Mat3 mat3_transform(Vec2 position, float theta, Vec2 scale, Vec2 shear)
{
    float ct = cosdf(theta + shear.x);
    float st = sindf(theta + shear.y);
    shear.x = 0;
    shear.y = 0;
    return (Mat3){{
        {(scale.x * ct) + (scale.y * shear.y * st), (scale.x * -st) + (scale.y * shear.y * ct), 0},
        {(scale.x * shear.x * ct) + (scale.y * st), (scale.x * shear.x * -st) + (scale.y * ct), 0},
        {position.x, position.y, 1},
    }};
}

inline static Mat3 mat3_mul3(Mat3 a, Mat3 b, Mat3 c)
{
    return mat3_mul(mat3_mul(a, b), c);
}

inline static float determinant(Mat3 m)
{
    return m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
           m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
           m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
}

inline static Mat3 mat3_inv(Mat3 m)
{
    float det = determinant(m);

    if (det == 0)
        return mat3_nan;

    float invDet = 1.0f / det;
    return (Mat3){{
        {(m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) * invDet, -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) * invDet, (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) * invDet},
        {-(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) * invDet, (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]) * invDet, -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]) * invDet},
        {(m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]) * invDet, -(m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]) * invDet, (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) * invDet},
    }};
}

inline static Vec2 mat3_unmulv2(Mat3 m, Vec2 p, float a)
{
    return mat3_mulv2(mat3_inv(m), p, a);
}
inline static Vec3 mat3_unmulv3(Mat3 m, Vec3 p)
{
    return mat3_mulv3(mat3_inv(m), p);
}
/*
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform2D {
public:
    glm::vec2 position;
    float rotation; // in degrees
    glm::vec2 scale;

    Transform2D() : position(0.0f), rotation(0.0f), scale(1.0f) {}

    // Calculate local matrix
    glm::mat3 getLocalMatrix() const {
        glm::mat3 translationMatrix = glm::translate(glm::mat3(1.0f), position);
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), glm::radians(rotation));
        glm::mat3 scaleMatrix = glm::scale(glm::mat3(1.0f), scale);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Calculate world matrix
    glm::mat3 getWorldMatrix() const {
        if (parent) {
            return parent->getWorldMatrix() * getLocalMatrix();
        } else {
            return getLocalMatrix();
        }
    }
    glm::vec2 convertPointToWorld(const glm::vec2& pointLocal) const {
        return glm::vec2(getWorldMatrix() * glm::vec3(pointLocal.x, pointLocal.y, 1.0f));
    }
    glm::vec2 convertPointToLocal(const glm::vec2& pointWorld) const {
        glm::mat3 inverseWorldMatrix = glm::inverse(getWorldMatrix());
        glm::vec3 pointWorldHomogeneous(pointWorld.x, pointWorld.y, 1.0f);
        glm::vec3 pointLocalHomogeneous = inverseWorldMatrix * pointWorldHomogeneous;
        return glm::vec2(pointLocalHomogeneous.x, pointLocalHomogeneous.y);
    }
    // Set parent of the object
    void setParent(Transform2D* newParent) {
        parent = newParent;
    }

private:
    Transform2D* parent = nullptr;
};


*/