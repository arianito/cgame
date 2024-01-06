#ifndef cgame_IVEC2_H
#define cgame_IVEC2_H

typedef struct
{
    int x;
    int y;
} iVec2;

static const iVec2 ivec2_zero = {0, 0};
static const iVec2 ivec2_one = {1, 1};
static const iVec2 ivec2_up = {0, 1};
static const iVec2 ivec2_down = {0, -1};
static const iVec2 ivec2_left = {-1, 0};
static const iVec2 ivec2_right = {1, 0};

#define ivec2(x, y) ((iVec2){x, y})

#endif