#include "bone.h"
#include "skel.h"
#include "skel_prv.h"
#include "bone_prv.h"
#include "engine/file.h"
#include <stdio.h>
#include "adt/fastvec.h"
#include "adt/common.h"
#include "constr.h"

const StrView IDENTIFIER_BONE = str("b");
const StrView IDENTIFIER_CONSTR = str("c");
const StrView IDENTIFIER_BONE_POS = str("pos");
const StrView IDENTIFIER_BONE_ROT = str("rot");
const StrView IDENTIFIER_BONE_LEN = str("len");
const StrView IDENTIFIER_BONE_SCALE = str("scl");
const StrView IDENTIFIER_BONE_TYPE = str("type");
const StrView IDENTIFIER_BONE_INHERIT = str("inherit");
const StrView IDENTIFIER_CONSTR_FROM = str("from");
const StrView IDENTIFIER_CONSTR_TO = str("to");
const StrView IDENTIFIER_CONSTR_TARGET = str("target");
const StrView IDENTIFIER_CONSTR_POLE = str("pole");


const StrView IDENTIFIER_ANIM = str("adnim");

void skeleton_loadfile(Skel *self, const char *p)
{
   SkelPrv *skel = self->context;

   size_t cursor = 0;
   StrView line;
   StrView pt = resolve_stack(p);
   FILE *f = fopen(pt.string, "r");
   xxfreestack(pt.string);

   if (f != NULL)
   {
      Bone tmp_bone;
      Constr tmp_constr;
      Fastvec_Stack *stack = fastvec_Stack_init(8);

#define SAFE_RETURN()            \
   fastvec_Stack_destroy(stack); \
   fclose(f);                    \
   xxfreestack(line.string);

#define CLEAR_BONES()                       \
   arena_reset(skel->buffer);               \
   fastmap_StrInt_clear(skel->map);         \
   fastvec_Constr_clear(skel->constraints); \
   fastvec_Bone_clear(skel->bones);

      while ((line = readline_stack(f, &cursor)).string != NULL)
      {

         StrView ft = str_first_token(line, ' ');
         if (str_eq(ft, IDENTIFIER_CONSTR))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(*fastvec_Stack_top(stack), IDENTIFIER_CONSTR))
            {
               fastvec_Stack_push(stack, IDENTIFIER_CONSTR);
               StrView splits[5];
               int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
               if (n == 1)
               {
                  str_truncate(splits, n);
                  tmp_constr = (Constr){0};
                  for (int i = 0; i < CONSTR_MAX_BONES; i++)
                     tmp_constr.bones[i] = -1;
                  tmp_constr.target = -1;
                  tmp_constr.solver = 0;
                  tmp_constr.n = 0;
                  tmp_constr.pole = -1;
                  tmp_constr.awake = true;
                  if (str_eq(splits[0], str("fabric")))
                     tmp_constr.solver = CONSTR_SOLVER_FABRIC;
                  else if (str_eq(splits[0], str("ccd")))
                     tmp_constr.solver = CONSTR_SOLVER_CCD;
               }
               else
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: constraint type not provided\n");
                  return;
               }
            }
            else
            {
               fastvec_Constr_push(skel->constraints, tmp_constr);
               fastvec_Stack_pop(stack);
            }
         }
         else if (str_eq(ft, IDENTIFIER_CONSTR_FROM) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_CONSTR))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);

               FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[0]);
               if (node == NULL)
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: bone not found: %s\n", cstr(splits[0]));
                  return;
               }

               tmp_constr.bones[0] = node->value;
            }
         }
         else if (str_eq(ft, IDENTIFIER_CONSTR_POLE) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_CONSTR))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n > 0)
            {
               str_truncate(splits, n);

               FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[0]);
               if (node == NULL)
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: bone not found: %s\n", cstr(splits[0]));
                  return;
               }
               tmp_constr.pole = node->value;

               if (n > 1)
               {
                  tmp_constr.pole_stiffness = str_tolong(splits[1]);
               }
            }
         }
         else if (str_eq(ft, IDENTIFIER_CONSTR_TO) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_CONSTR))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);

               FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[0]);
               if (node == NULL)
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: bone not found: %s\n", cstr(splits[0]));
                  return;
               }
               int from = tmp_constr.bones[0];
               int to = node->value;
               int ptr = to;
               tmp_constr.n = 0;
               for (int i = 0; i < CONSTR_MAX_BONES; i++)
               {
                  Bone *it = &skel->bones->vector[ptr];
                  tmp_constr.bones[tmp_constr.n++] = ptr;
                  if (it->index == from)
                     break;

                  if (it->parent == -1)
                  {
                     SAFE_RETURN();
                     CLEAR_BONES();
                     printf("skelfile: constraint can't reach source\n");
                     return;
                  }

                  ptr = it->parent;
               }
               if (tmp_constr.bones[0] != to || tmp_constr.bones[tmp_constr.n - 1] != from)
               {

                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: this solver has maximum joint size of %d\n", CONSTR_MAX_BONES);
                  return;
               }
               reverse(tmp_constr.bones, tmp_constr.n);
            }
         }
         else if (str_eq(ft, IDENTIFIER_CONSTR_TARGET) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_CONSTR))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);

               FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[0]);
               if (node == NULL)
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: bone not found: %s\n", cstr(splits[0]));
                  return;
               }

               tmp_constr.target = node->value;
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
            {
               fastvec_Stack_push(stack, IDENTIFIER_BONE);
               StrView splits[5];
               int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
               if (n > 0)
               {
                  str_truncate(splits, n);

                  // copy to local buffer
                  char *str = arena_alloc(skel->buffer, splits[0].length + 1);
                  str_copy(splits[0], str);
                  tmp_bone = (Bone){0};
                  tmp_bone.name = strv(str, splits[0].length);
                  tmp_bone.parent = -1;
                  tmp_bone.inherit = SKEL_INHERIT_ROTATION | SKEL_INHERIT_SCALE;
                  tmp_bone.type = n == 1 ? SKEL_TYP_ROOT : SKEL_TYP_BONE;
                  tmp_bone.lock = false;

                  tmp_bone.world = mat3_identity;
                  tmp_bone.local = mat3_identity;

                  tmp_bone.world_scale0 = vec2(1, 1);
                  tmp_bone.local_scale = vec2(1, 1);

                  if (n == 2)
                  {
                     FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[1]);
                     if (node == NULL)
                     {
                        printf("skelfile: parent not found: %s\n", cstr(splits[1]));
                        SAFE_RETURN();
                        CLEAR_BONES();
                        return;
                     }
                     tmp_bone.parent = node->value;
                  }
               }
               else
               {
                  SAFE_RETURN();
                  CLEAR_BONES();
                  printf("skelfile: bone name not provided\n");
                  return;
               }
            }
            else
            {
               FastmapNode_StrInt *node = fastmap_StrInt_put(skel->map, tmp_bone.name);
               int index = skel->bones->length;

               node->value = index;
               tmp_bone.index = index;

               tmp_bone.local_position = tmp_bone.world_position0;
               tmp_bone.local_rotation = tmp_bone.world_rotation0;
               tmp_bone.local_scale = tmp_bone.world_scale0;

               fastvec_Bone_push(skel->bones, tmp_bone);
               fastvec_Stack_pop(stack);

               update_matrices(self, false);
               bone_upd_transform(self, tmp_bone.index);
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_POS) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 2)
            {
               str_truncate(splits, n);
               tmp_bone.world_position0 = vec2(str_tofloat(splits[0]), str_tofloat(splits[1]));
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_ROT) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp_bone.world_rotation0 = str_tofloat(splits[0]);
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_LEN) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp_bone.len0 = str_tofloat(splits[0]);
               tmp_bone.len = tmp_bone.len0;
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_SCALE) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 2)
            {
               str_truncate(splits, n);
               tmp_bone.world_scale0 = vec2(str_tofloat(splits[0]), str_tofloat(splits[1]));
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_TYPE) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp_bone.type = str_tolong(splits[0]);
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE_INHERIT) && str_eq(*fastvec_Stack_top(stack), IDENTIFIER_BONE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp_bone.inherit = str_tolong(splits[0]);
            }
         }
         xxfreestack(line.string);
      }
      SAFE_RETURN();
   }
}