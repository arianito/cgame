#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "mem/alloc.h"

#include "adt/common.h"
#include "adt/fastree.h"

make_fastree_directives(Int, int, adt_compare_int)

    typedef struct
{
    Fastree_Int *tree;
} Graph1Context;

void inorder(FastreeNode_Int *node)
{
    if (node == NULL)
        return;
    inorder(node->left);
    printf("%d \n", node->value);
    inorder(node->right);
}
static void create(Graph1Context *self)
{
    self->tree = fastree_Int_init();

    fastree_Int_add(self->tree, 4);
    fastree_Int_add(self->tree, 3);
    fastree_Int_add(self->tree, 6);
    fastree_Int_add(self->tree, 5);
    fastree_Int_add(self->tree, 2);
    fastree_Int_add(self->tree, 1);

    inorder(self->tree->head);
}

static void render(Graph1Context *self)
{
    draw_point(vec3_zero, 2, color_yellow);
}

static void destroy(Graph1Context *self)
{
    fastree_Int_destroy(self->tree);
}

Level make_graph1()
{
    return (Level){
        context : xxarena(sizeof(Graph1Context)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}