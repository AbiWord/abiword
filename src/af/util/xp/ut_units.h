
#ifndef UT_UNITS_H
#define UT_UNITS_H

#include "ut_types.h"
class DG_Graphics;

NSPR_BEGIN_EXTERN_C

#define UT_PAPER_UNITS_PER_INCH				100

double UT_convertToInches(const char* s);
UT_sint32 UT_paperUnits(const char * sz);
UT_sint32 UT_docUnitsFromPaperUnits(DG_Graphics * pG, UT_sint32 iPaperUnits);

NSPR_END_EXTERN_C

#endif /* UT_UNITS_H */
