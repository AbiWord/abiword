#ifndef UT_OVERCHARS_H
#define UT_OVERCHARS_H

#include "ut_types.h"

typedef enum {UT_NOT_OVERSTRIKING, UT_OVERSTRIKING_RTL, UT_OVERSTRIKING_LTR} overstr_type;

overstr_type isOverstrikingChar(UT_UCSChar c);
#endif

