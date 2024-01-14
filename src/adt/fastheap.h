#pragma once

#include "fast.h"

#define __fastheap_type(t_name) Fastheap_##t_name

#define make_fastheap_directives(t_name, t_key, t_compare)                                       \
    typedef struct                                                                               \
    {                                                                                            \
        int length;                                                                              \
        int capacity;                                                                            \
        t_key *vector;                                                                           \
    } __fastheap_type(t_name);                                                                   \
                                                                                                 \
    inline static __fastheap_type(t_name) * fastheap_##t_name##_init(int cap)                    \
    {                                                                                            \
        __fastheap_type(t_name) *self = xxmalloc(sizeof(__fastheap_type(t_name)));               \
        self->capacity = cap;                                                                    \
        self->length = 0;                                                                        \
        self->vector = xxmalloc(self->capacity * sizeof(t_key));                                 \
        return self;                                                                             \
    }                                                                                            \
    inline static void fastheap_##t_name##_destroy(__fastheap_type(t_name) * self)               \
    {                                                                                            \
        xxfree(self->vector, self->capacity * sizeof(t_key));                                    \
        xxfree(self, sizeof(__fastheap_type(t_name)));                                           \
    }                                                                                            \
    inline static void __fastheap_##t_name##_sortup(__fastheap_type(t_name) * self, int index)   \
    {                                                                                            \
        while (index > 1)                                                                        \
        {                                                                                        \
            int parent = index / 2;                                                              \
            int cmp = t_compare(self->vector[parent - 1], self->vector[index - 1]);              \
            if (cmp > 0)                                                                         \
                return;                                                                          \
            __fast_swap(self->vector, index - 1, parent - 1);                                    \
            index = parent;                                                                      \
        }                                                                                        \
    }                                                                                            \
    inline static void __fastheap_##t_name##_sortdown(__fastheap_type(t_name) * self, int index) \
    {                                                                                            \
        while (index <= self->length)                                                            \
        {                                                                                        \
            int leftIndex = index * 2;                                                           \
            int rightIndex = index * 2 + 1;                                                      \
            if (leftIndex > self->length)                                                        \
                return;                                                                          \
            int next = leftIndex;                                                                \
            int cmp = t_compare(self->vector[rightIndex - 1], self->vector[leftIndex - 1]);      \
            if (rightIndex <= self->length && cmp > 0)                                           \
            {                                                                                    \
                next = rightIndex;                                                               \
            }                                                                                    \
            cmp = t_compare(self->vector[index - 1], self->vector[next - 1]);                    \
            if (cmp > 0)                                                                         \
                return;                                                                          \
            __fast_swap(self->vector, index - 1, next - 1);                                      \
            index = next;                                                                        \
        }                                                                                        \
    }                                                                                            \
    inline static void fastheap_##t_name##_push(__fastheap_type(t_name) * self, t_key value)     \
    {                                                                                            \
        if (self->length == self->capacity)                                                      \
        {                                                                                        \
            int nOldCap = self->capacity;                                                        \
            self->capacity = self->capacity << 1;                                                \
            t_key *vector = xxmalloc(self->capacity * sizeof(t_key));                            \
            memcpy(vector, self->vector, nOldCap * sizeof(t_key));                               \
            xxfree(self->vector, nOldCap * sizeof(t_key));                                       \
            self->vector = vector;                                                               \
        }                                                                                        \
        self->vector[self->length++] = value;                                                    \
        __fastheap_##t_name##_sortup(self, self->length);                                        \
    }                                                                                            \
    inline static t_key fastheap_##t_name##_pop(__fastheap_type(t_name) * self)                  \
    {                                                                                            \
        t_key root = self->vector[0];                                                            \
        self->vector[0] = self->vector[--self->length];                                          \
        __fastheap_##t_name##_sortdown(self, 1);                                                 \
        return root;                                                                             \
    }                                                                                            \
    inline static t_key *fastheap_##t_name##_top(__fastheap_type(t_name) * self)                 \
    {                                                                                            \
        return &self->vector[0];                                                                 \
    }                                                                                            \
    inline static bool fastheap_##t_name##_empty(__fastheap_type(t_name) * self)                 \
    {                                                                                            \
        return self->length == 0;                                                                \
    }                                                                                            \
    inline static void fastheap_##t_name##_clear(__fastheap_type(t_name) * self)                 \
    {                                                                                            \
        self->length = 0;                                                                        \
    }
