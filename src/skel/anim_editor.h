#pragma once

#include <stdbool.h>
#include "bone.h"

typedef struct
{
    float time;
    Anim *anim;
} AnimSequenceContext;

bool igSequencer(const char *name, const float height, AnimSequenceContext *context);