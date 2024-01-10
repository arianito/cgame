#pragma once

#include "types.h"
#include "adt/fastvec.h"


make_fastvec_directives(Bone, Bone);

typedef struct
{
    Fastvec_Bone *bones;
} SkelPrv;
