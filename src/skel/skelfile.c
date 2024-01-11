#include "bone.h"
#include "skel.h"
#include "skel_prv.h"
#include "engine/file.h"
#include <stdio.h>
#include "adt/fastvec.h"
#include "adt/common.h"

const StrView IDENTIFIER_SKEL = string_const("sk");
const StrView IDENTIFIER_BONE = string_const("b");
const StrView IDENTIFIER_POS = string_const("pos");
const StrView IDENTIFIER_ROT = string_const("rot");
const StrView IDENTIFIER_LEN = string_const("len");
const StrView IDENTIFIER_SCALE = string_const("sc");
const StrView IDENTIFIER_TYPE = string_const("typ");
const StrView IDENTIFIER_INHERIT = string_const("inh");

void bone_upd_transform(Skel *self, int bone)
{
   SkelPrv *skel = self->context;
   Bone *it = &skel->bones->vector[bone];
   if (it->parent != -1)
   {
      Bone *pt = &skel->bones->vector[it->parent];
      Mat3 m2 = mat3_inv(pt->world);

      float dx = it->world_position.x - pt->world_position.x;
      float dy = it->world_position.y - pt->world_position.y;
      it->local_position.x = dx * m2.m[0][0] + dy * m2.m[1][0];
      it->local_position.y = dx * m2.m[0][1] + dy * m2.m[1][1];

      m2 = mat3_mul(it->local, m2);
      it->local_rotation = clamp_axisf(atan2df(m2.m[1][0], m2.m[0][0]));

      float det = m2.m[0][0] * m2.m[1][1] - m2.m[0][1] * m2.m[1][0];
      it->local_scale.x = sqrf(m2.m[0][0] * m2.m[0][0] + m2.m[1][0] * m2.m[1][0]);
      it->local_scale.y = det / it->local_scale.x;
   }
   else
   {
      it->local_position = it->world_position;
      it->local_rotation = it->world_rotation;
      it->local_scale = it->world_scale;
   }
}

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
      Bone tmp;
      Fastvec_Stack *stack = fastvec_Stack_init(8);
#define SAFE_RETURN()            \
   fastvec_Stack_destroy(stack); \
   fclose(f);                    \
   xxfreestack(line.string);
#define CLEAR_BONES()               \
   arena_reset(skel->buffer);       \
   fastmap_StrInt_clear(skel->map); \
   fastvec_Bone_clear(skel->bones);
      while ((line = readline_stack(f, &cursor)).string != NULL)
      {

         StrView ft = str_first_token(line, ' ');
         if (str_eq(ft, IDENTIFIER_SKEL))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(fastvec_Stack_top(stack), IDENTIFIER_SKEL))
            {
               CLEAR_BONES();
               fastvec_Stack_push(stack, IDENTIFIER_SKEL);
            }
            else
            {
               fastvec_Stack_pop(stack);
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(fastvec_Stack_top(stack), IDENTIFIER_BONE))
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
                  tmp = (Bone){0};
                  tmp.name = string_view(str, splits[0].length);
                  tmp.parent = -1;
                  tmp.inherit = SKEL_INHERIT_ROTATION | SKEL_INHERIT_SCALE;
                  tmp.type = n == 1 ? SKEL_TYP_ROOT : SKEL_TYP_BONE;

                  tmp.world = mat3_identity;
                  tmp.local = mat3_identity;

                  tmp.world_scale = vec2(1, 1);
                  tmp.local_scale = vec2(1, 1);

                  if (n == 2)
                  {
                     FastmapNode_StrInt *node = fastmap_StrInt_get(skel->map, splits[1]);
                     if (node == NULL)
                     {
                        printf("skelfile: parent not found: %s\n", splits[1].string);
                        SAFE_RETURN();
                        CLEAR_BONES();
                        return;
                     }
                     tmp.parent = node->value;
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
               FastmapNode_StrInt *node = fastmap_StrInt_put(skel->map, tmp.name);
               int index = skel->bones->length;

               node->value = index;
               tmp.index = index;

               tmp.local_position = tmp.world_position;
               tmp.local_rotation = tmp.world_rotation;
               tmp.local_scale = tmp.world_scale;

               fastvec_Bone_push(skel->bones, tmp);
               fastvec_Stack_pop(stack);

               update_matrices(self);
               bone_upd_transform(self, tmp.index);
            }
         }
         else if (str_eq(ft, IDENTIFIER_POS))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 2)
            {
               str_truncate(splits, n);
               tmp.world_position = vec2(str_tofloat(splits[0]), str_tofloat(splits[1]));
            }
         }
         else if (str_eq(ft, IDENTIFIER_ROT))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp.world_rotation = str_tofloat(splits[0]);
            }
         }
         else if (str_eq(ft, IDENTIFIER_LEN))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp.len = str_tofloat(splits[0]);
            }
         }
         else if (str_eq(ft, IDENTIFIER_SCALE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 2)
            {
               str_truncate(splits, n);
               tmp.world_scale = vec2(str_tofloat(splits[0]), str_tofloat(splits[1]));
            }
         }
         else if (str_eq(ft, IDENTIFIER_TYPE))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp.type = str_tolong(splits[0]);
            }
         }
         else if (str_eq(ft, IDENTIFIER_INHERIT))
         {
            StrView splits[5];
            int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
            if (n == 1)
            {
               str_truncate(splits, n);
               tmp.inherit = str_tolong(splits[0]);
            }
         }
         xxfreestack(line.string);
      }
      SAFE_RETURN();
   }
}