/* AbiWord
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

// Common utilities for the left and top rulers.

#include <stdio.h>
#include "ut_types.h"
#include "ut_locale.h"
#include "ut_assert.h"
#include "gr_Graphics.h"
#include "ap_Ruler.h"

#define SCALE_IN 10
#define SCALE_CM 10
#define SCALE_MM 10
#define SCALE_PT 10

ap_RulerTicks::ap_RulerTicks(GR_Graphics * pG, UT_Dimension dim)
{
	char Buffer[30];

	m_pG = pG;
	dimType = dim;
	
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	// we scale the units up by UnitScale to avoid round-off problems.
	
	switch (dimType)
	{
	case DIM_IN:
		// For english, we draw numbers on the inches, long ticks 
		// on the half inches and short ticks on the eighth inches.  
		// We round up/down mouse actions to the nearest 1/16th.

		// On a 75 dpi screen, a 1/32 inch is 2.34375, so i set the scale to 100000.
		
		dBasicUnit = 0.125;
		tickUnitScale = SCALE_IN;
		sprintf(Buffer, "%fin", dBasicUnit * (double)tickUnitScale);
		tickUnit = UT_convertToLogicalUnits(Buffer); // 1/8th inch is our basic unit
		tickLong = 4;					// draw long ticks every 4 units (1/2 inch)
		tickLabel = 8;					// draw labeled ticks every 8 units (1 inch)
		tickScale = 1;					// label increment
		sprintf(Buffer, "%fin", (dBasicUnit / 2 * (double)tickUnitScale));
		dragDelta = UT_convertToLogicalUnits(Buffer); // 1/16th inch is mouse resolution
		break;

	case DIM_CM:
		dBasicUnit = 0.25;
		tickUnitScale = SCALE_CM;
		sprintf(Buffer, "%fcm", (dBasicUnit * (double)tickUnitScale));
		tickUnit = UT_convertToLogicalUnits(Buffer);
		tickLong = 2;
		tickLabel = 4;
		tickScale = 1;
		sprintf(Buffer, "%fcm", (dBasicUnit / 2 * (double)tickUnitScale));
		dragDelta = UT_convertToLogicalUnits(Buffer);
		break;

	case DIM_MM:
		dBasicUnit = 2.5;
		tickUnitScale = SCALE_MM;
		sprintf(Buffer, "%fmm", (dBasicUnit * (double)tickUnitScale));
		tickUnit = UT_convertToLogicalUnits(Buffer);
		tickLong = 2;
		tickLabel = 4;
		tickScale = 10;
		sprintf(Buffer, "%fmm", (dBasicUnit / 2 * (double)tickUnitScale));
		dragDelta = UT_convertToLogicalUnits(Buffer);
		break;

	case DIM_PI:						// picas
		dBasicUnit = 1.0;
		tickUnitScale = SCALE_PT;
		sprintf(Buffer, "%fpi", (dBasicUnit * (double)tickUnitScale));
		tickUnit = UT_convertToLogicalUnits(Buffer);
		tickLong = 6;
		tickLabel = 6;
		tickScale = 6;
		sprintf(Buffer, "%fpi", (dBasicUnit / 2 * (double)tickUnitScale));
		dragDelta = UT_convertToLogicalUnits(Buffer);
		break;
		
	case DIM_PT:						// points
		dBasicUnit = 6.0;
		tickUnitScale = SCALE_PT;
		sprintf(Buffer, "%fpt", (dBasicUnit * (double)tickUnitScale));
		tickUnit = UT_convertToLogicalUnits(Buffer);
		tickLong = 6;
		tickLabel = 6;
		tickScale = 36;
		sprintf(Buffer, "%fpt", (dBasicUnit / 2 * (double)tickUnitScale));
		dragDelta = UT_convertToLogicalUnits(Buffer);
		break;

	default:
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		break;
	}
}

/*! 
    Snap pixel value to nearest grid line
    \param dist Raw distance value on grid
    \param tick Ruler to which we snap
*/

UT_sint32 ap_RulerTicks::snapPixelToGrid(UT_sint32 dist)
{
	UT_sint32 rel = dist * tickUnitScale;
	if (rel > 0)
		rel = ((rel + (dragDelta/2) - 1) / dragDelta) * dragDelta / tickUnitScale;
	else
		rel = -(UT_sint32)((((-rel) + (dragDelta/2) - 1) / dragDelta) * dragDelta / tickUnitScale);

	return rel;
}

/*! 
    Convert pixel distance into units used by ruler
    \param dist Raw distance value on grid
    \param tick Ruler to which we snap
*/

double ap_RulerTicks::scalePixelDistanceToUnits(UT_sint32 dist)
{
	UT_sint32 rel = dist * tickUnitScale;
	if (rel > 0)
		rel = ((rel + (dragDelta/2) - 1) / dragDelta) * dragDelta;
	else
		rel = -(UT_sint32)((((-rel) + (dragDelta/2) - 1) / dragDelta) * dragDelta);

	double drel = ((double)rel) / ((double)tickUnitScale);
	return drel;
}
