#include "types.h"
#include "skel.h"
#include "skel_prv.h"
#include "engine/file.h"
#include <stdio.h>
#include "adt/fastvec.h"

make_fastvec_directives(Stack, StrView);

const StrView IDENTIFIER_SKEL = string_const("sk");
const StrView IDENTIFIER_BONE = string_const("b");

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
      Fastvec_Bone *bones = skel->bones;
      Fastvec_Stack *stack = fastvec_Stack_init(8);
      while ((line = readline_stack(f, &cursor)).string != NULL)
      {
         StrView ft = str_first_token(line, ' ');
         if (str_eq(ft, IDENTIFIER_SKEL))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(fastvec_Stack_top(stack), IDENTIFIER_SKEL))
            {
               fastvec_Bone_clear(bones);
               fastvec_Stack_push(stack, IDENTIFIER_SKEL);
            }
            else
            {
               // TODO: transform to local space
               fastvec_Stack_pop(stack);
            }
         }
         else if (str_eq(ft, IDENTIFIER_BONE))
         {
            if (fastvec_Stack_empty(stack) || !str_eq(fastvec_Stack_top(stack), IDENTIFIER_BONE))
            {
               fastvec_Stack_push(stack, IDENTIFIER_BONE);

               StrView splits[2];
               int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
               if(n == 2) {
                  str_truncate(splits, n);
               }
               tmp = (Bone){0};
            }
            else
            {
               fastvec_Stack_pop(stack);
            }
         }
         else if (str_eq(ft, string_const("pos")))
         {
         }
         else if (str_eq(ft, string_const("rot")))
         {
         }
         else if (str_eq(ft, string_const("len")))
         {
         }
         else if (str_eq(ft, string_const("sc")))
         {
         }
         else if (str_eq(ft, string_const("sh")))
         {
         }
         else if (str_eq(ft, string_const("sc")))
         {
         }
         xxfreestack(line.string);
      }
      fastvec_Stack_destroy(stack);
      fclose(f);
   }
}