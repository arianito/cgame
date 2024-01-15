#pragma once

#include "fast.h"

#define __fastqu_type(t_name) Fastqu_##t_name

#define make_fastqu_directives(t_name, t_key)                                            \
    typedef struct                                                                       \
    {                                                                                    \
        int capacity;                                                                    \
        int head;                                                                        \
        int tail;                                                                        \
        t_key *vector;                                                                   \
    } __fastqu_type(t_name);                                                             \
                                                                                         \
    inline static __fastqu_type(t_name) * fastqu_##t_name##_init(int cap)                \
    {                                                                                    \
        __fastqu_type(t_name) *self = xxmalloc(sizeof(__fastqu_type(t_name)));           \
        self->capacity = cap;                                                            \
        self->head = 0;                                                                  \
        self->tail = 0;                                                                  \
        self->vector = xxmalloc(self->capacity * sizeof(t_key));                         \
        return self;                                                                     \
    }                                                                                    \
    inline static void fastqu_##t_name##_destroy(__fastqu_type(t_name) * self)           \
    {                                                                                    \
        xxfree(self->vector, self->capacity * sizeof(t_key));                            \
        xxfree(self, sizeof(__fastqu_type(t_name)));                                     \
    }                                                                                    \
    inline static void fastqu_##t_name##_push(__fastqu_type(t_name) * self, t_key value) \
    {                                                                                    \
        if (((self->tail + 1) % self->capacity) == self->head)                           \
            return;                                                                      \
        self->vector[self->tail] = value;                                                \
        self->tail = (self->tail + 1) % self->capacity;                                  \
    }                                                                                    \
    inline static t_key fastqu_##t_name##_pop(__fastqu_type(t_name) * self)              \
    {                                                                                    \
        t_key value = self->vector[self->head];                                          \
        self->head = (self->head + 1) % mCapacity;                                       \
        return value;                                                                    \
    }                                                                                    \
    inline static bool fastqu_##t_name##_empty(__fastqu_type(t_name) * self)             \
    {                                                                                    \
        return self->head == self->tail;                                                 \
    }                                                                                    \
    inline static int fastqu_##t_name##_length(__fastqu_type(t_name) * self)             \
    {                                                                                    \
        if (self->head == self->tail)                                                    \
            return 0;                                                                    \
        int d = self->tail - self->head;                                                 \
        if (d < 0)                                                                       \
            d += self->capacity;                                                         \
        return d;                                                                        \
    }                                                                                    \
    inline static t_key *fastqu_##t_name##_top(__fastqu_type(t_name) * self)             \
    {                                                                                    \
        return &self->vector[self->head];                                                \
    }                                                                                    \
    inline static void fastqu_##t_name##_clear(__fastqu_type(t_name) * self)             \
    {                                                                                    \
        self->head = 0;                                                                  \
        self->tail = 0;                                                                  \
    }