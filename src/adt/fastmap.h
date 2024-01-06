#ifndef cgame_FASTMAP_H
#define cgame_FASTMAP_H

#include "fast.h"

#define __fastmap_group_type(t_name) __fastmapgroup_##t_name##_t
#define __fastmap_node_type(t_name) FastmapNode_##t_name
#define __fastmap_map_type(t_name) Fastmap_##t_name
#define __fastmap_itter_type(t_name) FastmapItter_##t_name

#define make_fastmap_directives(t_name, t_key, t_value, t_compare, t_hash)                                                                               \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        t_key key;                                                                                                                                       \
        t_value value;                                                                                                                                   \
    } __fastmap_node_type(t_name);                                                                                                                       \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __Byte16 control;                                                                                                                                \
        __fastmap_node_type(t_name) nodes[16];                                                                                                           \
        uint32_t overflow;                                                                                                                               \
    } __fastmap_group_type(t_name);                                                                                                                      \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __fastmap_group_type(t_name) * groups;                                                                                                           \
        uint32_t length;                                                                                                                                 \
        uint64_t groupSize;                                                                                                                              \
        uint32_t primeIndex;                                                                                                                             \
        uint32_t seed;                                                                                                                                   \
        double loadFactor;                                                                                                                               \
    } __fastmap_map_type(t_name);                                                                                                                        \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __fastmap_group_type(t_name) * begin;                                                                                                            \
        __fastmap_group_type(t_name) * end;                                                                                                              \
        __fastmap_node_type(t_name) * node;                                                                                                              \
        uint32_t index;                                                                                                                                  \
    } __fastmap_itter_type(t_name);                                                                                                                      \
                                                                                                                                                         \
    inline static __fastmap_node_type(t_name) * __fastmap_##t_name##_itter_next(__fastmap_itter_type(t_name) * self)                                     \
    {                                                                                                                                                    \
        while (self->begin != self->end)                                                                                                                 \
        {                                                                                                                                                \
            uint16_t matches = __fast_match_empty(self->begin->control);                                                                                 \
            matches >>= self->index;                                                                                                                     \
            while (self->index < 16)                                                                                                                     \
            {                                                                                                                                            \
                if (!(matches & 1))                                                                                                                      \
                    return &self->begin->nodes[self->index++];                                                                                           \
                matches >>= 1;                                                                                                                           \
                self->index++;                                                                                                                           \
            }                                                                                                                                            \
            self->begin++;                                                                                                                               \
            self->index = 0;                                                                                                                             \
        }                                                                                                                                                \
        return NULL;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastmap_node_type(t_name) * fastmap_##t_name##_put(__fastmap_map_type(t_name) * self, t_key key);                                    \
                                                                                                                                                         \
    inline static __fastmap_itter_type(t_name) __fastmap_##t_name##_itter_init(__fastmap_group_type(t_name) * begin, __fastmap_group_type(t_name) * end) \
    {                                                                                                                                                    \
        __fastmap_itter_type(t_name) self = {                                                                                                            \
            begin : begin,                                                                                                                               \
            end : end,                                                                                                                                   \
        };                                                                                                                                               \
        self.index = 0;                                                                                                                                  \
        self.node = __fastmap_##t_name##_itter_next(&self);                                                                                              \
        return self;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastmap_##t_name##_next(__fastmap_itter_type(t_name) * it)                                                                        \
    {                                                                                                                                                    \
        it->node = __fastmap_##t_name##_itter_next(it);                                                                                                  \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static bool fastmap_##t_name##_eof(__fastmap_itter_type(t_name) * it)                                                                         \
    {                                                                                                                                                    \
        return it->begin == it->end;                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastmap_##t_name##_reserve(__fastmap_map_type(t_name) * self, uint32_t newSize)                                                 \
    {                                                                                                                                                    \
        uint32_t size = newSize * sizeof(__fastmap_group_type(t_name));                                                                                  \
        self->groups = (__fastmap_group_type(t_name) *)xxmalloc(size);                                                                                   \
        memset(self->groups, 0, size);                                                                                                                   \
        for (uint32_t i = 0; i < newSize; i++)                                                                                                           \
            self->groups[i].control = __fast_set1_epi8(__fast_enum_empty);                                                                               \
        self->groupSize = newSize;                                                                                                                       \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastmap_itter_type(t_name) fastmap_##t_name##_begin(__fastmap_map_type(t_name) * self)                                               \
    {                                                                                                                                                    \
        return __fastmap_##t_name##_itter_init(self->groups, self->groups + self->groupSize);                                                            \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastmap__##t_name##_rehash(__fastmap_map_type(t_name) * self, uint32_t newSize)                                                 \
    {                                                                                                                                                    \
        __fastmap_group_type(t_name) *oldGroups = self->groups;                                                                                          \
        uint32_t nNumOldGroups = self->groupSize;                                                                                                        \
        __fastmap_##t_name##_reserve(self, newSize);                                                                                                     \
        __fastmap_itter_type(t_name) it = __fastmap_##t_name##_itter_init(oldGroups, oldGroups + nNumOldGroups);                                         \
        self->length = 0;                                                                                                                                \
        for (; !fastmap_##t_name##_eof(&it); fastmap_##t_name##_next(&it))                                                                               \
        {                                                                                                                                                \
            __fastmap_node_type(t_name) *node = it.node;                                                                                                 \
            __fastmap_node_type(t_name) *new_node = fastmap_##t_name##_put(self, node->key);                                                             \
            if (new_node != NULL)                                                                                                                        \
            {                                                                                                                                            \
                new_node->value = node->value;                                                                                                           \
            }                                                                                                                                            \
        }                                                                                                                                                \
        xxfree(oldGroups);                                                                                                                               \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastmap__##t_name##_rehash_grow(__fastmap_map_type(t_name) * self, int force)                                                   \
    {                                                                                                                                                    \
        self->loadFactor = ((double)self->length / (self->groupSize << 4));                                                                              \
        if (force || self->loadFactor >= 0.5f)                                                                                                           \
        {                                                                                                                                                \
            __fastmap__##t_name##_rehash(self, __fast_primes[self->primeIndex++]);                                                                       \
        }                                                                                                                                                \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastmap__##t_name##_rehash_fit(__fastmap_map_type(t_name) * self)                                                               \
    {                                                                                                                                                    \
        for (int i = 0; i < 90; i++)                                                                                                                     \
        {                                                                                                                                                \
            if (__fast_primes[i] > (self->length >> 4))                                                                                                  \
            {                                                                                                                                            \
                self->primeIndex = i;                                                                                                                    \
                __fastmap__##t_name##_rehash(self, __fast_primes[self->primeIndex]);                                                                     \
                return;                                                                                                                                  \
            }                                                                                                                                            \
        }                                                                                                                                                \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastmap_map_type(t_name) * fastmap_##t_name##_init()                                                                                 \
    {                                                                                                                                                    \
        __fastmap_map_type(t_name) *self = (__fastmap_map_type(t_name) *)xxmalloc(sizeof(__fastmap_map_type(t_name)));                                   \
        self->length = 0;                                                                                                                                \
        self->groupSize = 0;                                                                                                                             \
        self->primeIndex = 0;                                                                                                                            \
        self->seed = 0;                                                                                                                                  \
        self->loadFactor = 0;                                                                                                                            \
        __fastmap_##t_name##_reserve(self, 1);                                                                                                           \
        return self;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastmap_node_type(t_name) * fastmap_##t_name##_put(__fastmap_map_type(t_name) * self, t_key key)                                     \
    {                                                                                                                                                    \
        __fastmap__##t_name##_rehash_grow(self, 0);                                                                                                      \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastmap_group_type(t_name) * g;                                                                                                                \
        uint16_t matches;                                                                                                                                \
        int8_t ovf = 0;                                                                                                                                  \
        while (1)                                                                                                                                        \
        {                                                                                                                                                \
            g = &self->groups[groupIndex];                                                                                                               \
            matches = __fast_match(g->control, h2);                                                                                                      \
            int8_t i = 0;                                                                                                                                \
            while (matches)                                                                                                                              \
            {                                                                                                                                            \
                if ((matches & 1) && t_compare(g->nodes[i].key, key))                                                                                    \
                    return &g->nodes[i];                                                                                                                 \
                matches >>= 1;                                                                                                                           \
                i++;                                                                                                                                     \
            }                                                                                                                                            \
            matches = __fast_match_empty(g->control);                                                                                                    \
            if (matches)                                                                                                                                 \
                break;                                                                                                                                   \
            g->overflow = 1;                                                                                                                             \
            groupIndex = (groupIndex + 1) % self->groupSize;                                                                                             \
            ovf++;                                                                                                                                       \
        }                                                                                                                                                \
        if (ovf > 1)                                                                                                                                     \
        {                                                                                                                                                \
            __fastmap__##t_name##_rehash_grow(self, 1);                                                                                                  \
            return fastmap_##t_name##_put(self, key);                                                                                                    \
        }                                                                                                                                                \
        int8_t freeIndex = 0;                                                                                                                            \
        int8_t i = 0;                                                                                                                                    \
        while (matches)                                                                                                                                  \
        {                                                                                                                                                \
            if (matches & 1)                                                                                                                             \
            {                                                                                                                                            \
                freeIndex = i;                                                                                                                           \
                break;                                                                                                                                   \
            }                                                                                                                                            \
            matches >>= 1;                                                                                                                               \
            i++;                                                                                                                                         \
        }                                                                                                                                                \
        uint8_t *simdArray = (uint8_t *)(&g->control);                                                                                                   \
        simdArray[freeIndex] = h2;                                                                                                                       \
        g->nodes[freeIndex].key = key;                                                                                                                   \
        self->length++;                                                                                                                                  \
        return &g->nodes[freeIndex];                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastmap_node_type(t_name) * fastmap_##t_name##_get(__fastmap_map_type(t_name) * self, t_key key)                                     \
    {                                                                                                                                                    \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastmap_group_type(t_name) * g;                                                                                                                \
        uint16_t matches;                                                                                                                                \
        while (1)                                                                                                                                        \
        {                                                                                                                                                \
            g = &self->groups[groupIndex];                                                                                                               \
            matches = __fast_match(g->control, h2);                                                                                                      \
            int8_t i = 0;                                                                                                                                \
            while (matches)                                                                                                                              \
            {                                                                                                                                            \
                if ((matches & 1) && t_compare(g->nodes[i].key, key))                                                                                    \
                    return &g->nodes[i];                                                                                                                 \
                matches >>= 1;                                                                                                                           \
                i++;                                                                                                                                     \
            }                                                                                                                                            \
            if (__fast_match_empty(g->control))                                                                                                          \
                break;                                                                                                                                   \
            if (!g->overflow)                                                                                                                            \
                break;                                                                                                                                   \
            groupIndex = (groupIndex + 1) % self->groupSize;                                                                                             \
        }                                                                                                                                                \
        return NULL;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastmap_##t_name##_clear(__fastmap_map_type(t_name) * self)                                                                       \
    {                                                                                                                                                    \
        __fastmap_group_type(t_name) *oldGroups = self->groups;                                                                                          \
        __fastmap_##t_name##_reserve(self, 1);                                                                                                           \
        self->length = 0;                                                                                                                                \
        xxfree(oldGroups);                                                                                                                               \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastmap_##t_name##_remove_itter(__fastmap_map_type(t_name) * self, __fastmap_itter_type(t_name) * it)                             \
    {                                                                                                                                                    \
        __fast_set_byte(&it->begin->control, __fast_enum_deleted, it->index - 1);                                                                        \
        self->length--;                                                                                                                                  \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static bool fastmap_##t_name##_remove(__fastmap_map_type(t_name) * self, t_key key)                                                           \
    {                                                                                                                                                    \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastmap_group_type(t_name) * g;                                                                                                                \
        uint16_t matches;                                                                                                                                \
        while (1)                                                                                                                                        \
        {                                                                                                                                                \
            g = &self->groups[groupIndex];                                                                                                               \
            matches = __fast_match(g->control, h2);                                                                                                      \
            int8_t i = 0;                                                                                                                                \
            while (matches)                                                                                                                              \
            {                                                                                                                                            \
                if ((matches & 1) && t_compare(g->nodes[i].key, key))                                                                                    \
                {                                                                                                                                        \
                    uint8_t *simdArray = (uint8_t *)(&g->control);                                                                                       \
                    simdArray[i] = __fast_enum_deleted;                                                                                                  \
                    self->length--;                                                                                                                      \
                    return true;                                                                                                                         \
                }                                                                                                                                        \
                matches >>= 1;                                                                                                                           \
                i++;                                                                                                                                     \
            }                                                                                                                                            \
            if (!g->overflow)                                                                                                                            \
                break;                                                                                                                                   \
            groupIndex = (groupIndex + 1) % self->groupSize;                                                                                             \
        }                                                                                                                                                \
        return false;                                                                                                                                    \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastmap_##t_name##_destroy(__fastmap_map_type(t_name) * self)                                                                     \
    {                                                                                                                                                    \
        xxfree(self->groups);                                                                                                                            \
        xxfree(self);                                                                                                                                    \
    }

#define fastmap_for(t_name, self, it) for (__fastmap_itter_type(t_name) it = fastmap_##t_name##_begin(self); !fastmap_##t_name##_eof(&it); fastmap_##t_name##_next(&it))

#endif