#ifndef IE_MATH_CONVERT_H
#define IE_MATH_CONVERT_H
#endif

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_string.h"
#include "ut_string_class.h"

ABI_EXPORT bool convertMathMLtoLaTeX(const UT_UTF8String & sMathML, UT_UTF8String & sLaTeX);

ABI_EXPORT bool convertLaTeXtoEqn(const UT_UTF8String & sLaTeX,UT_UTF8String & eqnLaTeX);
