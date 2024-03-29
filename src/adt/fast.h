#ifndef cgame_FAST_H
#define cgame_FAST_H

#include <stdint.h>
#include <stdbool.h>

#include "mem/mem.h"

#define __fast_h1(hash) (hash)
#define __fast_h2(hash) (hash & 0x7F)

typedef long long __Byte16 __attribute__((__vector_size__(16), __aligned__(16)));

enum
{
    __fast_enum_empty = -128,
    __fast_enum_deleted = -2,
};

static const unsigned int __fast_primes[90] = {
    2ul, 3ul, 5ul, 7ul, 11ul, 13ul, 17ul, 23ul, 29ul, 37ul, 47ul,
    59ul, 73ul, 97ul, 127ul, 151ul, 197ul, 251ul, 313ul, 397ul,
    499ul, 631ul, 797ul, 1009ul, 1259ul, 1597ul, 2011ul, 2539ul,
    3203ul, 4027ul, 5087ul, 6421ul, 8089ul, 10193ul, 12853ul, 16193ul,
    20399ul, 25717ul, 32401ul, 40823ul, 51437ul, 64811ul, 81649ul,
    102877ul, 129607ul, 163307ul, 205759ul, 259229ul, 326617ul,
    411527ul, 518509ul, 653267ul, 823117ul, 1037059ul, 1306601ul,
    1646237ul, 2074129ul, 2613229ul, 3292489ul, 4148279ul, 5226491ul,
    6584983ul, 8296553ul, 10453007ul, 13169977ul, 16593127ul, 20906033ul,
    26339969ul, 33186281ul, 41812097ul, 52679969ul, 66372617ul,
    83624237ul, 105359939ul, 132745199ul, 167248483ul, 210719881ul,
    265490441ul, 334496971ul, 421439783ul, 530980861ul, 668993977ul,
    842879579ul, 1061961721ul, 1337987929ul, 1685759167ul, 2123923447ul,
    2675975881ul, 3371518343ul, 4247846927ul};

uint16_t __fast_match_empty(__Byte16 ctrl);

uint16_t __fast_match(__Byte16 ctrl, int8_t hash);

void __fast_set_byte(__Byte16 *current, uint8_t value, uint8_t index);

__Byte16 __fast_set1_epi8(char c);

#define __fast_swap(data, i, j) ({         \
    if (i != j)                            \
    {                                      \
        typeof(data[0]) tmp = data[i]; \
        data[i] = data[j];         \
        data[j] = tmp;                 \
    }                                      \
})

#endif