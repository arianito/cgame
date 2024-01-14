#ifndef cgame_FILE_H
#define cgame_FILE_H

#include <stddef.h>
#include "adt/str.h"

StrView resolve_stack(const char *fmt, ...);

StrView readfile_stack(const char *p);

StrView readline_stack(void *f, size_t *cursor);

void file_init(const char *fmt, ...);

#endif