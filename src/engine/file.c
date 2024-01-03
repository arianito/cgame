

#include "file.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "mem/alloc.h"

static char *prefix;
static int prefixLength;

char *readfile_stack(const char *p)
{
    const int buffSize = 64;
    char buffer[buffSize];
    int n = 0;

    char *pt = resolve_stack(p);
    FILE *f = fopen(pt, "r");
    stack_free(alloc->stack, pt);

    char *data = (char *)stack_alloc(alloc->stack, buffSize, sizeof(size_t));
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
    stack_realloc(alloc->stack, data, n + 1);
    data[n] = 0;
    return data;
}

char *readline_stack(void *f, long *cursor)
{
    fseek(f, *cursor, SEEK_SET);
    const int buffSize = 2;
    char buffer[buffSize];
    int n = 0;
    char lst = 1;
    char *data = (char *)stack_alloc(alloc->stack, buffSize, sizeof(size_t));
    while (1)
    {
        int i = 0;
        char ctu = 1;
        size_t r = fread(buffer, 1, buffSize, f);
        if (r == 0)
        {
            if (lst)
            {
                stack_free(alloc->stack, data);
                return NULL;
            }
            else
            {
                lst = 1;
                break;
            }
        }
        for (; i < r; i++)
        {
            if (buffer[i] == '\n')
            {
                ctu = 0;
                i++;
                break;
            }
        }
        stack_realloc(alloc->stack, data, n + i);
        memcpy(data + n, buffer, i);
        n += i;
        lst = 0;
        if (!ctu)
            break;
    }
    if (lst)
        stack_realloc(alloc->stack, data, n++);
    data[n - 1] = '\0';
    *cursor += n;
    return data;
}

char *resolve_stack(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *out = (char *)stack_alloc(alloc->stack, prefixLength + len + 1, sizeof(size_t));
    char *buffer = (char *)stack_alloc(alloc->stack, len + 1, sizeof(size_t));
    if (prefix != NULL)
    {
        va_start(args, fmt);
        vsnprintf(buffer, len + 1, fmt, args);
        va_end(args);
        buffer[len] = '\0';
    }

    sprintf(out, "%s%s", prefix, buffer);
    stack_free(alloc->stack, buffer);
    return out;
}

void file_init(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    prefixLength = len;
    prefix = (char *)arena_alloc(alloc->global, len + 1, sizeof(size_t));
    if (prefix != NULL)
    {
        va_start(args, fmt);
        vsnprintf(prefix, len + 1, fmt, args);
        va_end(args);
        prefix[len] = '\0';
    }
}
