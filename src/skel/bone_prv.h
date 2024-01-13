
#include "math/vec2.h"
#include "math/mat3.h"
#include "skel.h"

void bone_upd_transform(Skel *self, int bone);
void update_matrices(Skel *self, bool update_world);
void update_transforms(Skel *self);