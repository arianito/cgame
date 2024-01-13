#pragma once

#include "bone.h"
#include "adt/fastvec.h"
#include "adt/fastmap.h"
#include "mem/arena.h"
#include "adt/common.h"
#include "constr.h"

make_fastvec_directives(Bone, Bone);
make_fastvec_directives(Constr, Constr);
make_fastmap_directives(StrInt, StrView, int, adt_compare_string, adt_hashof_string);
make_fastvec_directives(Stack, StrView);

typedef struct
{
    ArenaMemory* buffer;
    
    Fastvec_Bone *bones;
    Fastvec_Constr *constraints;

    Fastmap_StrInt *map;
    int dirty_local;
    int dirty_world;
} SkelPrv;
