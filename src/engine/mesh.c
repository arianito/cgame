#include "mesh.h"

#include <stdlib.h>
#include <stdio.h>

#include "mem/alloc.h"
#include "file.h"
#include "string.h"


// inline static StringView firstToken(StringView line) {
//     if (line.empty())
//         return "";

//     size_t start = line.find_first_not_of(' ');
//     size_t end = line.find_first_of(' ', start);

//     if (start != TStringView::npos && end != TStringView::npos) {
//         return line.substr(start, end - start);
//     }

//     if (start != TStringView::npos) {
//         return line.substr(start);
//     }

//     return "";
// }

Mesh *mesh_from_obj(const char *p)
{
    size_t cursor = 0;
    StringView line;
    StringView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    stack_free(alloc->stack, pt.string);
    if (f != NULL)
    {
        while ((line = readline_stack(f, &cursor)).string != NULL)
        {
            printf("%zu\n", cursor);
            stack_free(alloc->stack, line.string);
        }
        fclose(f);
    }

    return NULL;
}

void *mesh_free(Mesh *ptr)
{
}