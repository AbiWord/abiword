#include "ut_OverstrikingChars.h"
#include "ut_assert.h"
#include <stdlib.h>

typedef struct {UT_UCSChar min; UT_UCSChar max; overstr_type dir;} char_bounds;

// the following table is heuristic; we will add to it more characters
// when we need to (we could try to generate this table for the whole
// UCS-2 space, but at the moment that might be an overkill)

// the table contains pairs of UCS-2 values between which all characters
// are overstriking (this is inclusive of the two boundary values in the
// table)

// !!! the table MUST be sorted to allow binary search !!!
char_bounds overstr_lut[]=
{
//Hebrew
{0x0591,0x05bd, UT_OVERSTRIKING_RTL}, //this includes two currently undefined characters 0x05a2 and 0x05ba
{0x05bf,0x05bf, UT_OVERSTRIKING_RTL},
{0x05c1,0x05c2, UT_OVERSTRIKING_RTL},
{0x05c4,0x05c4, UT_OVERSTRIKING_RTL},

//Syriac
{0x0711,0x0711, UT_OVERSTRIKING_RTL},
{0x0730,0x074a, UT_OVERSTRIKING_RTL}
};

static UT_sint32 s_compare(const void * a, const void * b)
{
	const UT_UCSChar  * c  = (const UT_UCSChar *)  a;
	const char_bounds * bn = (const char_bounds *) b;

	if(*c < bn->min)
		return -1;

	if(*c > bn->max)
		return 1;

	return 0;
}

overstr_type isOverstrikingChar(UT_UCSChar c)
{
	char_bounds * e;
	if((e = (char_bounds *) bsearch(&c, overstr_lut, NrElements(overstr_lut), sizeof(char_bounds), s_compare)))
	{
		return e->dir;
	}
	else
		return UT_NOT_OVERSTRIKING;
}

