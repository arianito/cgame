#ifndef cgame_FILE_H
#define cgame_FILE_H

#include <stddef.h>

char *resolve_stack(const char *fmt, ...);

char *readfile_stack(const char *p);

char *readline_stack(void *f, long *cursor);

void file_init(const char *fmt, ...);

#endif