#include "mesh.h"

#include <stdlib.h>
#include <stdio.h>

#include "mem/alloc.h"
#include "file.h"
#include "string.h"


// inline static StrView firstToken(StrView line) {
//     if (str_empty(line))
//         return line;

//     size_t start = line.find_first_not_of(' ');
//     size_t end = line.find_first_of(' ', start);

//     if (start != TStrView::npos && end != TStrView::npos) {
//         return line.substr(start, end - start);
//     }

//     if (start != TStrView::npos) {
//         return line.substr(start);
//     }

//     return "";
// }

Mesh *mesh_from_obj(const char *p)
{
    size_t cursor = 0;
    StrView line;
    StrView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    xxfreestack(pt.string);


    if (f != NULL)
    {
        while ((line = readline_stack(f, &cursor)).string != NULL)
        {
            StrView token = str_untilchar(str_skipchar(line, ' ', 0), ' ', 0);
            char* o = str_tostack(token);
            xxfreestack((void*)o);

            
            xxfreestack(line.string);
        }
        fclose(f);
    }

    return NULL;
}

void *mesh_free(Mesh *ptr)
{
}