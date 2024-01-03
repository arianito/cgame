#ifndef cgame_CUSTOM_TYPES_H
#define cgame_CUSTOM_TYPES_H

#include "murmur.h"

#include "fastset.h"
#include "fastmap.h"
#include "fastvec.h"
#include "engine/mathf.h"
#include "engine/mathf.h"

inline static int __compare_Vec3(Vec3 a, Vec3 b)
{
    return vec3_nearEq(a, b);
}

inline static uint64_t __hashof_Vec3(Vec3 key, uint64_t seed)
{
    char bytes[12];
    memcpy(bytes, &key, 12);
    return murmurhash(bytes, 12, seed);
}

inline static int __compare_Edge(Edge a, Edge b)
{
    return edge_nearEq(a, b);
}

inline static uint64_t __hashof_Edge(Edge e, int seed)
{
    return __hashof_Vec3(e.a, seed) ^ __hashof_Vec3(e.b, seed);
}
make_fastset_directives(Edge, Edge, __compare_Edge, __hashof_Edge);
make_fastset_directives(Vec3, Vec3, __compare_Vec3, __hashof_Vec3);
make_fastvec_directives(Edge, Edge);

#endif
