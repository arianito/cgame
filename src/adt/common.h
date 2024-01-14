#pragma once

#include <memory.h>
#include "math/edge.h"
#include "math/vec3.h"
#include "adt/murmur.h"
#include <string.h>

#include "adt/str.h"

inline static int adt_compare_vec3(Vec3 a, Vec3 b)
{
    if (vec3_near_eq(a, b))
        return 0;
    return -1;
}

inline static uint64_t adt_hashof_vec3(Vec3 key, uint64_t seed)
{
    return murmurhash((char *)(&key), 12, seed);
}

inline static int adt_compare_edge(Edge a, Edge b)
{
    if (edge_near_eq(a, b))
        return 0;
    return -1;
}

inline static uint64_t adt_hashof_edge(Edge e, int seed)
{
    return adt_hashof_vec3(e.a, seed) ^ adt_hashof_vec3(e.b, seed);
}

inline static int adt_compare_cstr(const char *a, const char *b)
{
    return strcmp(a, b);
}

inline static uint64_t adt_hashof_cstr(const char *key, uint64_t seed)
{
    return murmurhash(key, strlen(key), seed);
}

inline static int adt_compare_string(const StrView a, const StrView b)
{
    return str_compare(a, b);
}

inline static uint64_t adt_hashof_string(const StrView key, uint64_t seed)
{
    return murmurhash(key.string, key.length, seed);
}

#define adt_compare_primitive(a, b) ((a) - (b))
#define adt_hashof_primitive(key, seed) (murmurhash((char *)(&(key)), sizeof(key), (seed)))