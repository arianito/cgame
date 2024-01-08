

#include "file.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "mem/alloc.h"
#include <stdlib.h>
#include "mem/defs.h"
#include <sys/stat.h>

static char *prefix;
static int prefixLength;

#define MAX_FILE_LENGTH = 1 * MEGABYTES

StrView readfile_stack(const char *p)
{
    const int buffSize = 128;
    size_t n = 0;
    StrView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    xxfreestack(pt.string);
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f) + buffSize;
    char *data = (char *)xxstack(file_size);
    fseek(f, 0, SEEK_SET);
    size_t readBytes;
    while ((readBytes = fread(data + n, 1, buffSize, f)) > 0)
    {
        n += (int)readBytes;
    }
    fclose(f);
    data[n] = 0;
    return (StrView){data, n};
}

StrView readline_stack(void *f, size_t *cursor)
{
    fseek(f, *cursor, SEEK_SET);
    const int buffSize = 128;
    char *data = xxstack(4 * KILOBYTES + buffSize);
    size_t n = 0;
    size_t readBytes = 0;
    while ((readBytes = fread(&data[n], 1, buffSize, f)) > 0)
    {
        for (int i = 0; i < readBytes; i++)
        {
            if (data[n + i] == '\n')
            {
                n += i + 1;
                *cursor += n;
                data[n - 1] = 0;
                return (StrView){data, n - 1};
            }
        }
        n += readBytes;
        
    }
    if (n == 0)
    {
        xxfreestack(data);
        return (StrView){NULL, 0};
    }
    data[n] = 0;
    *cursor += n;
    return (StrView){data, n};
}

StrView resolve_stack(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *out = (char *)xxstack(prefixLength + len + 1);
    char buffer[4 * KILOBYTES];
    va_start(args, fmt);
    vsnprintf(buffer, len + 1, fmt, args);
    va_end(args);
    buffer[len] = '\0';
    sprintf(out, "%s%s", prefix, buffer);
    return (StrView){out, prefixLength + len};
}

void file_init(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    prefixLength = len;
    prefix = (char *)xxarena(len + 1);
    if (prefix != NULL)
    {
        va_start(args, fmt);
        vsnprintf(prefix, len + 1, fmt, args);
        va_end(args);
        prefix[len] = '\0';
    }
}
