#ifndef cgame_STRING_H
#define cgame_STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mem/alloc.h"

typedef struct
{
    char *string;
    uint32_t length;
} StrView;

static const StrView str_null = {NULL, 0};

#define str(text) ((StrView){text, sizeof(text) - 1})
#define strv(text, n) ((StrView){text, n})
#define cstr(s) (s.string)
#define len(s) (s.length)

#define __string_repeat_index(i, n) ((i) < 0 ? ((n) - ((-(i)-1) % (n) + 1)) : ((i) % (n)))

#define reverse(arr, n) ({     \
    int start = 0;             \
    int end = n - 1;           \
    while (start < end)        \
    {                          \
        typeof(arr[0]) temp = arr[start]; \
        arr[start] = arr[end]; \
        arr[end] = temp;       \
        start++;               \
        end--;                 \
    }                          \
})

static inline bool str_empty(const StrView a)
{
    return a.string == NULL || a.length == 0;
}

static inline void str_lps(const StrView needle, int32_t *lps)
{
    int32_t j = 0, i = 1;
    while (i < needle.length)
    {
        if (needle.string[i] == needle.string[j])
            lps[i++] = ++j;
        else if (j == 0)
            i++;
        else
            j = lps[j - 1];
    }
}

static inline int32_t str_find(const StrView haystack, const StrView needle, int32_t start)
{

    if (str_empty(haystack) || str_empty(needle))
        return -1;
    if (start < 0 || start >= haystack.length)
        return -1;
    int32_t j = 0, i = start;
    uint32_t n = needle.length;
    int32_t *lps = (int *)xxstack(sizeof(int32_t) * n);
    memset(lps, 0, sizeof(int32_t) * n);
    str_lps(needle, lps);
    while (i < haystack.length)
    {
        if (haystack.string[i] == needle.string[j])
        {
            i++;
            j++;
        }
        else if (j == 0)
            i++;
        else
            j = lps[j - 1];
        if (j == n)
        {
            xxfreestack(lps);
            return i - n;
        }
    }
    xxfreestack(lps);
    return -1;
}

static inline float str_tofloat(const StrView str)
{
    return (float)strtof(str.string, NULL);
}

static inline long str_tolong(const StrView str)
{
    return strtol(str.string, NULL, 10);
}
static inline int32_t str_skipchar(const StrView str, char c, int32_t start)
{
    if (start < 0 || start >= str.length)
        return -1;
    for (uint32_t i = start; i < str.length; i++)
        if (str.string[i] != c)
            return i;
    return -1;
}

static inline int32_t str_untilchar(const StrView str, char c, int32_t start)
{
    if (start < 0 || start >= str.length)
        return -1;
    for (uint32_t i = start; i < str.length; i++)
        if (str.string[i] == c)
            return i;
    return -1;
}

static inline int32_t str_untilchar_rev(const StrView str, char c, int32_t start)
{
    if (start < 0 || start >= str.length)
        return -1;

    start = str.length - start - 1;
    for (uint32_t i = str.length - 1; i > start; i--)
        if (str.string[i] == c)
            return i;
    return -1;
}

static inline char *str_tostack(const StrView str)
{
    char *o = (char *)xxstack(str.length + 1);
    memcpy(o, str.string, str.length);
    o[str.length] = 0;
    return o;
}
static inline void str_copy(const StrView str, char *out)
{
    memcpy(out, str.string, str.length);
    out[str.length] = 0;
}

static inline StrView str_substr(const StrView str, int32_t start, uint32_t len)
{
    if (str_empty(str))
        return str;
    start = __string_repeat_index(start, str.length);
    if (len == 0 || (start + len) >= str.length)
        len = str.length - start;

    return strv(str.string + start, len);
}

static inline bool str_eq(const StrView a, const StrView b)
{
    if (a.length != b.length)
        return false;

    if (a.string == b.string)
        return true;

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

inline static int str_splitchar(StrView line, char c, StrView splits[])
{
    if (str_empty(line))
        return 0;
    int start = str_skipchar(line, c, 0);
    if (start == -1)
        return 0;
    int i = 0;
    while (start != -1 && start < line.length)
    {
        int32_t next = str_untilchar(line, c, start);
        splits[i++] = str_substr(line, start, next - start);
        start = str_skipchar(line, c, next);
    }
    return i;
}

inline static void str_truncate(StrView splits[], int n)
{
    for (int i = 0; i < n; i++)
        splits[i].string[splits[i].length] = 0;
}

inline static StrView str_first_token(StrView line, char c)
{
    if (str_empty(line))
        return str_null;
    int32_t start = str_skipchar(line, c, 0);
    int32_t end = str_untilchar(line, c, start);
    if (start != -1 && end != -1)
        return str_substr(line, start, end - start);
    if (start != -1)
        return str_substr(line, start, 0);
    return str_null;
}

inline static StrView str_last_token(StrView line, char c)
{
    if (str_empty(line))
        return str_null;
    int32_t start = str_skipchar(line, c, 0);
    int32_t next = str_untilchar(line, c, start);
    start = str_skipchar(line, c, next);
    next = str_untilchar_rev(line, c, 0);
    if (start != -1 && next != -1)
        return str_substr(line, start, next - start);
    if (start != -1)
        return str_substr(line, start, 0);
    return str_null;
}

#endif