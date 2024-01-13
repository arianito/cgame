#pragma once

#include "bone.h"


#define CONSTR_MAX_BONES 10
#define CONSTR_SOLVER_FABRIC 1
#define CONSTR_SOLVER_CCD 2

typedef struct
{
    int bones[CONSTR_MAX_BONES];
    int target;
    int n;
    int solver;
    int pole;
    bool awake;
} Constr;
