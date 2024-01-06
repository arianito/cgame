// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "constants.h"
#include "types.h"
#include "math/aabb.h"


b2RayCastOutput b2AABB_RayCast(AABB a, Vec2 p1, Vec2 p2);
