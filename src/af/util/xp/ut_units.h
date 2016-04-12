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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_UNITS_H
#define UT_UNITS_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

G_BEGIN_DECLS

#define UT_PAPER_UNITS_PER_INCH				100
/* Units of UT_LAYOUT_RESOLUTION = dots per inch */
#define UT_LAYOUT_RESOLUTION				1440

/* 2^31 -1 */

#define UT_INT_MAX 2147483647

enum UT_Dimension {
  DIM_IN,
  DIM_CM,
  DIM_MM,
  DIM_PI,
  DIM_PT,
  DIM_PX,
  DIM_PERCENT,
  DIM_STAR,
  DIM_none
};

/*
 *	DIM_IN := inches
 *	DIM_CM := centimeters
 *	DIM_MM := millimeters
 *	DIM_PI := picas
 *	DIM_PT := points
 *	DIM_PX := pixels
 */

ABI_EXPORT double UT_convertToInches(const char* s);
ABI_EXPORT double UT_convertDimToInches (double f, UT_Dimension dim);
ABI_EXPORT double UT_convertDimensions(double f, UT_Dimension from, UT_Dimension to);
ABI_EXPORT double UT_convertToPoints(const char* s);
ABI_EXPORT double UT_convertToDimension(const char* s, UT_Dimension dim);
ABI_EXPORT UT_sint32 UT_convertToLogicalUnits(const char* s);
ABI_EXPORT UT_sint32 UT_convertSizeToLayoutUnits(double Value, UT_Dimension dim);
ABI_EXPORT double UT_convertDimensionless(const char * sz);
ABI_EXPORT double UT_convertInchesToDimension(double inches, UT_Dimension dim);

ABI_EXPORT UT_sint32 UT_paperUnits(const char * sz);
ABI_EXPORT double    UT_inchesFromPaperUnits(UT_sint32 iPaperUnits);
ABI_EXPORT UT_sint32 UT_paperUnitsFromInches(double dInches);

ABI_EXPORT const char * UT_incrementDimString(const char * dimString, double inc);
ABI_EXPORT const char * UT_multiplyDimString(const char * dimString, double mult);
ABI_EXPORT UT_Dimension UT_determineDimension(const char * sz, UT_Dimension fallback = DIM_IN);
ABI_EXPORT const char * UT_dimensionName(UT_Dimension dim);
ABI_EXPORT const char * UT_convertInchesToDimensionString(UT_Dimension, double valueInInches, const char * szPrecision = NULL);
ABI_EXPORT const char * UT_formatDimensionString(UT_Dimension, double value, const char * szPrecision = NULL);
ABI_EXPORT const char * UT_reformatDimensionString(UT_Dimension dim, const char *sz, const char * szPrecision = NULL);
ABI_EXPORT const char * UT_convertToDimensionlessString(double value, const char * szPrecision = NULL);
ABI_EXPORT const char * UT_formatDimensionedValue(double value, const char * szUnits, const char * szPrecision = NULL);

ABI_EXPORT bool UT_hasDimensionComponent(const char * sz);
ABI_EXPORT bool UT_isValidDimensionString(const char * sz, size_t max_length = 0);

ABI_EXPORT UT_uint32 UT_getDimensionPrecisicion (UT_Dimension dim);
ABI_EXPORT double UT_getDimensionResolution (UT_Dimension dim);
ABI_EXPORT double UT_convertFraction(const char * sz);

G_END_DECLS

#endif /* UT_UNITS_H */
