/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#ifndef UT_UNITS_H
#define UT_UNITS_H

#include "ut_types.h"
class GR_Graphics;

UT_BEGIN_EXTERN_C

#define UT_PAPER_UNITS_PER_INCH				100
#define UT_LAYOUT_UNITS						1440


typedef enum _ut_dimension { DIM_IN, DIM_CM, DIM_MM, DIM_PI, DIM_PT, DIM_PX, DIM_none } UT_Dimension;

double UT_convertToInches(const char* s);
double UT_convertToPoints(const char* s);
UT_sint32 UT_convertToLayoutUnits(const char* s);
double UT_convertDimensionless(const char * sz);
double UT_convertInchesToDimension(double inches, UT_Dimension dim);

UT_sint32 UT_paperUnits(const char * sz);
UT_sint32 UT_docUnitsFromPaperUnits(GR_Graphics * pG, UT_sint32 iPaperUnits);
UT_sint32 UT_layoutUnitsFromPaperUnits(UT_sint32 iPaperUnits);
UT_sint32 UT_paperUnitsFromLayoutUnits(UT_sint32 iLayoutUnits);

UT_Dimension UT_determineDimension(const char * sz, UT_Dimension fallback = DIM_IN);
const char * UT_dimensionName(UT_Dimension dim);
const char * UT_convertToDimensionString(UT_Dimension, double value, const char * szPrecision = NULL);
const char * UT_convertToDimensionlessString(double value, const char * szPrecision = NULL);
const char * UT_formatDimensionedValue(double value, const char * szUnits, const char * szPrecision = NULL);

UT_Bool UT_hasDimensionComponent(const char * sz);

UT_END_EXTERN_C

#endif /* UT_UNITS_H */
