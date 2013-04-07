/* AbiWord
 * Copyright (C) 2012 Prashant Bafna (appu.bafna@gmail.com)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

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

ABI_EXPORT bool convertOMMLtoMathML(const std::string & pOMML, std::string & pMathML);

ABI_EXPORT bool convertMathMLtoOMML(const std::string & rMathML, std::string & rOMML);
