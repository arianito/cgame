#pragma once

#include "bone.h"


#define CONSTR_SOLVER_FABRIC 1
#define CONSTR_SOLVER_FABRIC 2

typedef struct
{
    int bones[4];
    int target;
    int n;
    int solver;
} Constr;
