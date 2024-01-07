#ifndef cgame_FILE_H
#define cgame_FILE_H

#include <stddef.h>
#include "string.h"

StringView resolve_stack(const char *fmt, ...);

StringView readfile_stack(const char *p);

StringView readline_stack(void *f, size_t *cursor);

void file_init(const char *fmt, ...);

#endif