// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

/// @file
/// Constants used by box2d.
/// box2d uses meters-kilograms-seconds (MKS) units. Angles are always in radians unless
/// degrees are indicated.
/// Some values can be overridden by using a compiler definition.
/// Other values cannot be modified without causing stability and/or performance problems.
///	Although most of these are not user configurable, it can be interesting for a user to see
///	these to understand the tuning values Box2D uses.


/// box2d bases all length units on meters, but you may need different units for your game.
/// You can override this value to use different units.
#define b2_lengthUnitsPerMeter 1.0f

/// https://en.wikipedia.org/wiki/Pi
#define b2_pi 3.14159265359f

/// This is used to fatten AABBs in the dynamic tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
/// This is in meters.
/// @warning modifying this can have a significant impact on performance
#define b2_aabbMargin (0.1f * b2_lengthUnitsPerMeter)

/// A small length used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant. In meters.
/// @warning modifying this can have a significant impact on stability
#define b2_linearSlop (0.005f * b2_lengthUnitsPerMeter)

/// A small angle used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
/// @warning modifying this can have a significant impact on stability
#define b2_angularSlop (2.0f)

/// The maximum number of vertices on a convex polygon. Changing this affects performance even if you
///	don't use more vertices.
#define b2_maxPolygonVertices 8

/// Maximum number of simultaneous worlds that can be allocated
#define b2_maxWorlds 128

/// The maximum linear translation of a body per step. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this. Meters.
/// @warning modifying this can have a significant impact on stability
#define b2_maxTranslation (4.0f * b2_lengthUnitsPerMeter)

/// The maximum angular velocity of a body. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this.
/// @warning modifying this can have a significant impact on stability
#define b2_maxRotation (90.0f)

/// @warning modifying this can have a significant impact on performance and stability
#define b2_speculativeDistance (4.0f * b2_linearSlop)

/// The time that a body must be still before it will go to sleep. In seconds.
#define b2_timeToSleep 0.5f

/// A body cannot sleep if its linear velocity is above this tolerance. Meters per second.
#define b2_linearSleepTolerance (0.01f * b2_lengthUnitsPerMeter)

/// A body cannot sleep if its angular velocity is above this tolerance. Radians per second.
#define b2_angularSleepTolerance (2.0f)

/// Used to detect bad values. Positions greater than about 16km will have precision
/// problems, so 100km as a limit should be fine in all cases.
#define b2_huge (100000.0f * b2_lengthUnitsPerMeter)

/// Maximum parallel workers. Used to size some static arrays.
#define b2_maxWorkers 64

/// Solver graph coloring
#define b2_graphColorCount 12