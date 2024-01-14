#pragma once

#include "fast.h"

#define __fastheap_type(t_name) Fastheap_##t_name

#define make_fastheap_directives(t_name, t_key)                                              \
    typedef struct                                                                           \
    {                                                                                        \
        int length;                                                                          \
        int capacity;                                                                        \
        t_key *vector;                                                                       \
    } __fastheap_type(t_name);                                                               \
                                                                                             \
    inline static __fastheap_type(t_name) * fastheap_##t_name##_init(int cap)                \
    {                                                                                        \
        __fastheap_type(t_name) *self = xxmalloc(sizeof(__fastheap_type(t_name)));           \
        self->capacity = cap;                                                                \
        self->length = 0;                                                                    \
        self->vector = xxmalloc(self->capacity * sizeof(t_key));                             \
        return self;                                                                         \
    }                                                                                        \
    inline static void fastheap_##t_name##_destroy(__fastheap_type(t_name) * self)           \
    {                                                                                        \
        xxfree(self->vector, self->capacity * sizeof(t_key));                                \
        xxfree(self, sizeof(__fastheap_type(t_name)));                                       \
    }                                                                                        \
    inline static void fastheap_##t_name##_push(__fastheap_type(t_name) * self, t_key value) \
    {                                                                                        \
    }                                                                                        \
    inline static t_key fastheap_##t_name##_pop(__fastheap_type(t_name) * self)              \
    {                                                                                        \
    }                                                                                        \
    inline static t_key *fastheap_##t_name##_top(__fastheap_type(t_name) * self)             \
    {                                                                                        \
        return &self->vector[0];                                                             \
    }                                                                                        \
    inline static bool fastheap_##t_name##_empty(__fastheap_type(t_name) * self)             \
    {                                                                                        \
        return self->length == 0;                                                            \
    }                                                                                        \
    inline static void fastheap_##t_name##_clear(__fastheap_type(t_name) * self)             \
    {                                                                                        \
        self->length = 0;                                                                    \
    }
