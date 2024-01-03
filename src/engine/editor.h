#ifndef cgame_EDITOR_H
#define cgame_EDITOR_H

#include "mathf.h"

void editor_init();

void editor_update();

void editor_focus(Vec3 pos);
void editor_commit_focus();

#endif