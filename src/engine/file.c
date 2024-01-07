

#include "file.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "mem/alloc.h"
#include <stdlib.h>

static char *prefix;
static int prefixLength;

StringView readfile_stack(const char *p)
{
    const int buffSize = 128;
    size_t n = 0;
    StringView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    stack_free(alloc->stack, pt.string);
    char *data = (char *)stack_alloc(alloc->stack, buffSize);
    if (f != NULL)
    {
        fseek(f, 0, SEEK_SET);
        size_t readBytes;
        while ((readBytes = fread(data + n, 1, buffSize, f)) > 0)
        {
            n += (int)readBytes;
            stack_realloc(alloc->stack, data, n + buffSize);
        }
        fclose(f);
    }
    data[n] = 0;
    return (StringView){data, n};
}

StringView readline_stack(void *f, size_t *cursor)
{
    fseek(f, *cursor, SEEK_SET);
    const int buffSize = 128;
    char buffer[buffSize];
    size_t n = 0;
    bool lst = true;
    char *data = (char *)stack_alloc(alloc->stack, buffSize);
    while (true)
    {
        bool ctu = true;
        size_t i = 0;
        size_t r = fread(buffer, 1, buffSize, f);
        if (r == 0)
        {
            if (lst)
            {
                stack_free(alloc->stack, data);
                return (StringView){NULL, 0};
            }
            else
            {
                lst = true;
                break;
            }
        }
        for (; i < r; i++)
        {
            if (buffer[i] == '\n')
            {
                ctu = false;
                i++;
                break;
            }
        }
        stack_realloc(alloc->stack, data, n + i);
        memcpy(data + n, buffer, i);
        n += i;
        lst = false;
        if (!ctu)
            break;
    }
    data[n - 1] = '\0';
    *cursor += n;
    return (StringView){data, n - 1};
}

StringView resolve_stack(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *out = (char *)stack_alloc(alloc->stack, prefixLength + len + 1);
    char *buffer = (char *)stack_alloc(alloc->stack, len + 1);
    va_start(args, fmt);
    vsnprintf(buffer, len + 1, fmt, args);
    va_end(args);
    buffer[len] = '\0';
    sprintf(out, "%s%s", prefix, buffer);
    stack_free(alloc->stack, buffer);
    return (StringView){out, prefixLength + len};
}

void file_init(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    prefixLength = len;
    prefix = (char *)arena_alloc(alloc->global, len + 1);
    if (prefix != NULL)
    {
        va_start(args, fmt);
        vsnprintf(prefix, len + 1, fmt, args);
        va_end(args);
        prefix[len] = '\0';
    }
}
