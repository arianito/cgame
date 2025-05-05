#ifndef cgame_EDITOR_H
#define cgame_EDITOR_H

#include "math/vec3.h"

void editor_init();
void editor_update();
void editor_focus(Vec3 pos);
void editor_commit_focus();
void editor_enable();
void editor_disable();
void editor_restore();

#endif