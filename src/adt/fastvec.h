#ifndef cgame_FASTVEC_H
#define cgame_FASTVEC_H

#include "fast.h"

#define __fastvec_type(t_name) Fastvec_##t_name

#define make_fastvec_directives(t_name, t_key)                                             \
    typedef struct                                                                         \
    {                                                                                      \
        int length;                                                                        \
        int capacity;                                                                      \
        t_key *vector;                                                                     \
    } __fastvec_type(t_name);                                                              \
                                                                                           \
    inline static __fastvec_type(t_name) * fastvec_##t_name##_init(int cap)                \
    {                                                                                      \
        __fastvec_type(t_name) *self = xxmalloc(sizeof(__fastvec_type(t_name)));           \
        self->capacity = cap;                                                              \
        self->length = 0;                                                                  \
        self->vector = xxmalloc(self->capacity * sizeof(t_key));                           \
        return self;                                                                       \
    }                                                                                      \
    inline static void fastvec_##t_name##_destroy(__fastvec_type(t_name) * self)           \
    {                                                                                      \
        xxfree(self->vector, self->capacity * sizeof(t_key));                              \
        xxfree(self, sizeof(__fastvec_type(t_name)));                                      \
    }                                                                                      \
    inline static void fastvec_##t_name##_push(__fastvec_type(t_name) * self, t_key value) \
    {                                                                                      \
        if (self->length == self->capacity)                                                \
        {                                                                                  \
            int nOldCap = self->capacity;                                                  \
            self->capacity = self->capacity << 1;                                          \
            t_key *vector = xxmalloc(self->capacity * sizeof(t_key));                      \
            memcpy(vector, self->vector, nOldCap * sizeof(t_key));                         \
            xxfree(self->vector, nOldCap * sizeof(t_key));                                 \
            self->vector = vector;                                                         \
        }                                                                                  \
        self->vector[self->length++] = value;                                              \
    }                                                                                      \
    inline static t_key fastvec_##t_name##_pop(__fastvec_type(t_name) * self)              \
    {                                                                                      \
        return self->vector[--self->length];                                               \
    }                                                                                      \
    inline static bool fastvec_##t_name##_empty(__fastvec_type(t_name) * self)            \
    {                                                                                      \
        return self->length == 0;                                                          \
    }                                                                                      \
    inline static void fastvec_##t_name##_remove(__fastvec_type(t_name) * self, int index) \
    {                                                                                      \
        self->length--;                                                                    \
        t_key *tmp = &self->vector[index];                                                 \
        self->vector[index] = self->vector[self->length];                                  \
        self->vector[self->length] = *tmp;                                                 \
    }                                                                                      \
    inline static t_key *fastvec_##t_name##_top(__fastvec_type(t_name) * self)              \
    {                                                                                      \
        return &self->vector[self->length - 1];                                             \
    }                                                                                      \
    inline static void fastvec_##t_name##_clear(__fastvec_type(t_name) * self)             \
    {                                                                                      \
        self->length = 0;                                                                  \
    }

#endif