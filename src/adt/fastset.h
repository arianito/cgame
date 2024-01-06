#ifndef cgame_FASTSET_H
#define cgame_FASTSET_H

#include "fast.h"

#define __fastset_group_type(t_name) __fastsetgroup_##t_name##_t
#define __fastset_node_type(t_name) FastsetNode_##t_name
#define __fastset_map_type(t_name) Fastset_##t_name
#define __fastset_itter_type(t_name) FastsetItter_##t_name

#define make_fastset_directives(t_name, t_key, t_compare, t_hash)                                                                                        \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        t_key key;                                                                                                                                       \
    } __fastset_node_type(t_name);                                                                                                                       \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __fastset_node_type(t_name) nodes[16];                                                                                                           \
        uint32_t overflow;                                                                                                                               \
        __m128i control;                                                                                                                                 \
    } __fastset_group_type(t_name);                                                                                                                      \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __fastset_group_type(t_name) * groups;                                                                                                           \
        uint32_t length;                                                                                                                                 \
        uint64_t groupSize;                                                                                                                              \
        uint32_t state;                                                                                                                                  \
        uint32_t seed;                                                                                                                                   \
        double loadFactor;                                                                                                                               \
    } __fastset_map_type(t_name);                                                                                                                        \
                                                                                                                                                         \
    typedef struct                                                                                                                                       \
    {                                                                                                                                                    \
        __fastset_group_type(t_name) * begin;                                                                                                            \
        __fastset_group_type(t_name) * end;                                                                                                              \
        __fastset_node_type(t_name) * node;                                                                                                              \
        uint32_t index;                                                                                                                                  \
    } __fastset_itter_type(t_name);                                                                                                                      \
                                                                                                                                                         \
    inline static __fastset_node_type(t_name) * __fastset_##t_name##_itter_next(__fastset_itter_type(t_name) * self)                                     \
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
    inline static __fastset_node_type(t_name) * fastset_##t_name##_put(__fastset_map_type(t_name) * self, t_key key);                                    \
                                                                                                                                                         \
    inline static __fastset_itter_type(t_name) __fastset_##t_name##_itter_init(__fastset_group_type(t_name) * begin, __fastset_group_type(t_name) * end) \
    {                                                                                                                                                    \
        __fastset_itter_type(t_name) self = {                                                                                                            \
            begin : begin,                                                                                                                               \
            end : end,                                                                                                                                   \
        };                                                                                                                                               \
        self.index = 0;                                                                                                                                  \
        self.node = __fastset_##t_name##_itter_next(&self);                                                                                              \
        return self;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastset_##t_name##_next(__fastset_itter_type(t_name) * it)                                                                        \
    {                                                                                                                                                    \
        it->node = __fastset_##t_name##_itter_next(it);                                                                                                  \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static bool fastset_##t_name##_eof(__fastset_itter_type(t_name) * it)                                                                         \
    {                                                                                                                                                    \
        return it->begin == it->end;                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastset_##t_name##_reserve(__fastset_map_type(t_name) * self, uint32_t newSize)                                                 \
    {                                                                                                                                                    \
        uint32_t size = newSize * sizeof(__fastset_group_type(t_name));                                                                                  \
        self->groups = (__fastset_group_type(t_name) *)xxmalloc(size);                                                                                   \
        memset(self->groups, 0, size);                                                                                                                   \
        for (uint32_t i = 0; i < newSize; i++)                                                                                                                \
        {                                                                                                                                                \
            (self->groups[i]).control = _mm_set1_epi8(__fast_enum_empty);                                                                                \
        }                                                                                                                                                \
        self->groupSize = newSize;                                                                                                                       \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastset_itter_type(t_name) fastset_##t_name##_begin(__fastset_map_type(t_name) * self)                                               \
    {                                                                                                                                                    \
        return __fastset_##t_name##_itter_init(self->groups, self->groups + self->groupSize);                                                            \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastset__##t_name##_rehash(__fastset_map_type(t_name) * self, uint32_t newSize)                                                 \
    {                                                                                                                                                    \
        __fastset_group_type(t_name) *oldGroups = self->groups;                                                                                          \
        uint32_t nNumOldGroups = self->groupSize;                                                                                                        \
        __fastset_##t_name##_reserve(self, newSize);                                                                                                     \
        __fastset_itter_type(t_name) it = __fastset_##t_name##_itter_init(oldGroups, oldGroups + nNumOldGroups);                                         \
        self->length = 0;                                                                                                                                \
        int i = 0;                                                                                                                                       \
        for (; !fastset_##t_name##_eof(&it); fastset_##t_name##_next(&it))                                                                               \
        {                                                                                                                                                \
            __fastset_node_type(t_name) *node = it.node;                                                                                                 \
            __fastset_node_type(t_name) *new_node = fastset_##t_name##_put(self, node->key);                                                             \
        }                                                                                                                                                \
        xxfree(oldGroups);                                                                                                                               \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastset__##t_name##_rehash_grow(__fastset_map_type(t_name) * self, int force)                                                   \
    {                                                                                                                                                    \
        self->loadFactor = ((double)self->length / (self->groupSize << 4));                                                                              \
        if (force || self->loadFactor >= 0.5f)                                                                                                           \
        {                                                                                                                                                \
            __fastset__##t_name##_rehash(self, __fast_primes[self->state++]);                                                                            \
        }                                                                                                                                                \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void __fastset__##t_name##_rehash_fit(__fastset_map_type(t_name) * self)                                                               \
    {                                                                                                                                                    \
        for (int i = 0; i < 90; i++)                                                                                                                     \
        {                                                                                                                                                \
            if (__fast_primes[i] > (self->length >> 4))                                                                                                  \
            {                                                                                                                                            \
                self->state = i;                                                                                                                         \
                __fastset__##t_name##_rehash(self, __fast_primes[self->state]);                                                                          \
                return;                                                                                                                                  \
            }                                                                                                                                            \
        }                                                                                                                                                \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastset_map_type(t_name) * fastset_##t_name##_init()                                                                                 \
    {                                                                                                                                                    \
        __fastset_map_type(t_name) *self = (__fastset_map_type(t_name) *)xxmalloc(sizeof(__fastset_map_type(t_name)));                                   \
        self->length = 0;                                                                                                                                \
        self->groupSize = 0;                                                                                                                             \
        self->state = 0;                                                                                                                                 \
        self->seed = 0;                                                                                                                                  \
        self->loadFactor = 0;                                                                                                                            \
        __fastset_##t_name##_reserve(self, 1);                                                                                                           \
        return self;                                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastset_node_type(t_name) * fastset_##t_name##_put(__fastset_map_type(t_name) * self, t_key key)                                     \
    {                                                                                                                                                    \
        __fastset__##t_name##_rehash_grow(self, 0);                                                                                                      \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastset_group_type(t_name) *g = NULL;                                                                                                          \
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
                    return &(g->nodes[i]);                                                                                                               \
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
            __fastset__##t_name##_rehash_grow(self, 1);                                                                                                  \
            return fastset_##t_name##_put(self, key);                                                                                                    \
        }                                                                                                                                                \
        int8_t freeIndex = 0;                                                                                                                            \
        int8_t i = 0;                                                                                                                                    \
        while (matches)                                                                                                                                  \
        {                                                                                                                                                \
            if (matches & 0b1)                                                                                                                           \
            {                                                                                                                                            \
                freeIndex = i;                                                                                                                           \
                break;                                                                                                                                   \
            }                                                                                                                                            \
            matches >>= 1;                                                                                                                               \
            i++;                                                                                                                                         \
        }                                                                                                                                                \
        __fast_set_byte(&g->control, h2, freeIndex);                                                                                                     \
        g->nodes[freeIndex].key = key;                                                                                                                   \
        self->length++;                                                                                                                                  \
        return &g->nodes[freeIndex];                                                                                                                     \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static __fastset_node_type(t_name) * fastset_##t_name##_get(__fastset_map_type(t_name) * self, t_key key)                                     \
    {                                                                                                                                                    \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastset_group_type(t_name) * g;                                                                                                                \
        uint16_t matches;                                                                                                                                \
        while (1)                                                                                                                                        \
        {                                                                                                                                                \
            g = &self->groups[groupIndex];                                                                                                               \
            matches = __fast_match(g->control, h2);                                                                                                      \
            int8_t i = 0;                                                                                                                                \
            while (matches)                                                                                                                              \
            {                                                                                                                                            \
                if ((matches & 1) && t_compare(g->nodes[i].key, key))                                                                                    \
                    return &(g->nodes[i]);                                                                                                               \
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
    inline static void fastset_##t_name##_remove_itter(__fastset_map_type(t_name) * self, __fastset_itter_type(t_name) * it)                             \
    {                                                                                                                                                    \
        __fast_set_byte(&it->begin->control, __fast_enum_deleted, it->index - 1);                                                                        \
        self->length--;                                                                                                                                  \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static void fastset_##t_name##_clear(__fastset_map_type(t_name) * self)                                                                       \
    {                                                                                                                                                    \
        __fastset_group_type(t_name) *oldGroups = self->groups;                                                                                          \
        __fastset_##t_name##_reserve(self, 1);                                                                                                           \
        self->length = 0;                                                                                                                                \
        xxfree(oldGroups);                                                                                                                               \
    }                                                                                                                                                    \
                                                                                                                                                         \
    inline static bool fastset_##t_name##_remove(__fastset_map_type(t_name) * self, t_key key)                                                           \
    {                                                                                                                                                    \
        uint64_t hash = t_hash(key, self->seed);                                                                                                         \
        uint64_t groupIndex = __fast_h1(hash) % self->groupSize;                                                                                         \
        uint8_t h2 = __fast_h2(hash);                                                                                                                    \
        __fastset_group_type(t_name) * g;                                                                                                                \
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
                    __fast_set_byte(&g->control, __fast_enum_deleted, i);                                                                                \
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
    inline static void fastset_##t_name##_destroy(__fastset_map_type(t_name) * self)                                                                     \
    {                                                                                                                                                    \
        xxfree(self->groups);                                                                                                                            \
        xxfree(self);                                                                                                                                    \
    }

#define fastset_for(t_name, self, it) for (__fastset_itter_type(t_name) it = fastset_##t_name##_begin(self); !fastset_##t_name##_eof(&it); fastset_##t_name##_next(&it))

#endif