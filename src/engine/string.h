#ifndef cgame_STRING_H
#define cgame_STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char *string;
    uint32_t length;
} StringView;

#define string(text) ((StringView){text, sizeof(text) - 1})

static inline bool string_compare(StringView a, StringView b)
{
    if (a.length != b.length)
        return false;

    uint32_t n = (a.length / sizeof(size_t));
    uint32_t rem = a.length - n * sizeof(size_t);
    
    for (uint32_t i = 0; i < n; i++)
        if (((size_t *)a.string)[i] != ((size_t *)b.string)[i])
            return false;

    for (uint32_t i = n * sizeof(size_t); i < n * sizeof(size_t) + rem; i++)
        if (a.string[i] != b.string[i])
            return false;

    return true;
}

#endif