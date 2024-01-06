// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "types.h"

/// Timer for profiling. This has platform specific code and may not work on every platform.
typedef struct b2Timer
{
#if defined(_WIN32)
	int64_t start;
#elif defined(__linux__) || defined(__APPLE__)
	unsigned long long start_sec;
	unsigned long long start_usec;
#else
	int dummy;
#endif
} b2Timer;

b2Timer b2CreateTimer(void);
int64_t b2GetTicks(b2Timer* timer);
float b2GetMilliseconds(const b2Timer* timer);
float b2GetMillisecondsAndReset(b2Timer* timer);
void b2SleepMilliseconds(float milliseconds);

/// Tracy profiler instrumentation
///	https://github.com/wolfpld/tracy
#ifdef BOX2D_PROFILE

#include <tracy/TracyC.h>
#define b2TracyCZoneC(ctx, color, active) TracyCZoneC(ctx, color, active)
#define b2TracyCZoneNC(ctx, name, color, active) TracyCZoneNC(ctx, name, color, active)
#define b2TracyCZoneEnd(ctx) TracyCZoneEnd(ctx)

#else

#define b2TracyCZoneC(ctx, color, active)
#define b2TracyCZoneNC(ctx, name, color, active)
#define b2TracyCZoneEnd(ctx)

#endif
