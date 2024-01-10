#pragma once

#include "types.h"
#include "adt/fastvec.h"
#include "mem/arena.h"

make_fastvec_directives(Bone, Bone);

typedef struct
{
    ArenaMemory* buffer;
    Fastvec_Bone *bones;
} SkelPrv;
