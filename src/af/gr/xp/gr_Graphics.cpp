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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"

GR_Font::GR_Font() 
{
}

GR_Font::~GR_Font() 
{
	// need this so children can clean up
}

GR_Graphics::GR_Graphics()
{
	m_iZoomPercentage = 100;
	m_bLayoutResolutionModeEnabled = UT_FALSE;
}

GR_Graphics::~GR_Graphics()
{
	// need this so children can clean up
}

void GR_Graphics::setZoomPercentage(UT_uint32 iZoom)
{
	UT_ASSERT(iZoom > 0);
	
	m_iZoomPercentage = iZoom;
}

UT_uint32 GR_Graphics::getZoomPercentage(void) const
{
	return m_iZoomPercentage;
}

UT_uint32 GR_Graphics::getResolution(void) const
{
	return _getResolution() * m_iZoomPercentage / 100;
}

UT_sint32 GR_Graphics::convertDimension(const char * s) const
{
	double dInches = UT_convertToInches(s);
	double dResolution;
	if(m_bLayoutResolutionModeEnabled)
		{
		dResolution = UT_LAYOUT_UNITS;
		}
	else
		{
		dResolution = getResolution();		// NOTE: assumes square pixels/dpi/etc.
		}

	return (UT_sint32) (dInches * dResolution);
}

const char * GR_Graphics::invertDimension(UT_Dimension dim, double dValue) const
{
	// return pointer to static buffer -- use it quickly.
	
	double dResolution;
	if(m_bLayoutResolutionModeEnabled)
		{
		dResolution = UT_LAYOUT_UNITS;
		}
	else
		{
		dResolution = getResolution();		// NOTE: assumes square pixels/dpi/etc.
		}

	double dInches = dValue / dResolution;

	return UT_convertToDimensionString( dim,
				UT_convertInchesToDimension( dInches, dim ) );
}

UT_Bool GR_Graphics::scaleDimensions(const char * szLeftIn, const char * szWidthIn,
									 UT_uint32 iWidthAvail,
									 UT_sint32 * piLeft, UT_uint32 * piWidth) const
{
	/* Scale the given left-offset and width using the width available.
	** Compute the actual left-offset and actual width used.
	** We allow the given left-offset to be a number.
	** We allow the given width to be a number or "*"; where "*" indicates
	** we take all remaining space available.
	**
	** NOTE: This routine can also be used for vertical calculations.
	*/

	UT_ASSERT(szLeftIn);
	UT_ASSERT(szWidthIn);

	UT_sint32 iLeft = convertDimension(szLeftIn);
	UT_uint32 iWidth;

	if (szWidthIn[0] == '*')
		iWidth = iWidthAvail - iLeft;
	else
		iWidth = convertDimension(szWidthIn);

	if (piLeft)
		*piLeft = iLeft;
	if (piWidth)
		*piWidth = iWidth;

	return UT_TRUE;
}

void GR_Graphics::flush(void)
{
	// default implementation does nothing
}
