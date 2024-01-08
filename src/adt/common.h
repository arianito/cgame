#pragma once

#include <memory.h>
#include "math/edge.h"
#include "math/vec3.h"
#include "adt/murmur.h"
#include <string.h>

inline static bool adt_compare_Vec3(Vec3 a, Vec3 b)
{
    return vec3_near_eq(a, b);
}

inline static uint64_t adt_hashof_Vec3(Vec3 key, uint64_t seed)
{
    return murmurhash((char *)(&key), 12, seed);
}

inline static int adt_compare_Edge(Edge a, Edge b)
{
    return edge_near_eq(a, b);
}

inline static uint64_t adt_hashof_Edge(Edge e, int seed)
{
    return adt_hashof_Vec3(e.a, seed) ^ adt_hashof_Vec3(e.b, seed);
}

inline static bool adt_compare_string(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

inline static uint64_t adt_hash_string(const char *key, uint64_t seed)
{
    return murmurhash(key, strlen(key), seed);
}
