#pragma once

#include "scalar.h"
#include "vec2_ifc.h"
#include "vec3_ifc.h"

typedef struct
{
    float m[3][3];
} Mat3;

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
    float ct = cosdf(theta);
    float st = sindf(theta);
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