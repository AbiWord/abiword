/* AbiSource Application Framework
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dialog_FontChooser.h"

#define FREEP(p)	do { if (p) free(p); (p) = NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p) = NULL; } while (0)
#define CLONEP(p,q)	do { FREEP(p); if (q && *q) UT_cloneString(p,q); } while (0)

/*****************************************************************/

AP_Dialog_FontChooser::AP_Dialog_FontChooser(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer				= a_CANCEL;
	m_pGraphics				= NULL;
	m_pFontFamily			= NULL;
	m_pFontSize				= NULL;
	m_pFontWeight			= NULL;
	m_pFontStyle			= NULL;
	m_pColor				= NULL;
	m_bUnderline			= UT_FALSE;
	m_bStrikeOut			= UT_FALSE;

	m_bChangedFontFamily	= UT_FALSE;
	m_bChangedFontSize		= UT_FALSE;
	m_bChangedFontWeight	= UT_FALSE;
	m_bChangedFontStyle		= UT_FALSE;
	m_bChangedColor			= UT_FALSE;
	m_bChangedUnderline		= UT_FALSE;
	m_bChangedStrikeOut		= UT_FALSE;
}

AP_Dialog_FontChooser::~AP_Dialog_FontChooser(void)
{
	FREEP(m_pFontFamily);
	FREEP(m_pFontSize);
	FREEP(m_pFontWeight);
	FREEP(m_pFontStyle);
	FREEP(m_pColor);
}

void AP_Dialog_FontChooser::setGraphicsContext(GR_Graphics * pGraphics)
{
	m_pGraphics = pGraphics;
}

void AP_Dialog_FontChooser::setFontFamily(const XML_Char * pFontFamily)
{
	CLONEP(m_pFontFamily, pFontFamily);
}

void AP_Dialog_FontChooser::setFontSize(const XML_Char * pFontSize)
{
	CLONEP(m_pFontSize, pFontSize);
}

void AP_Dialog_FontChooser::setFontWeight(const XML_Char * pFontWeight)
{
	CLONEP(m_pFontWeight, pFontWeight);
}

void AP_Dialog_FontChooser::setFontStyle(const XML_Char * pFontStyle)
{
	CLONEP(m_pFontStyle, pFontStyle);
}

void AP_Dialog_FontChooser::setColor(const XML_Char * pColor)
{
	CLONEP(m_pColor, pColor);
}

void AP_Dialog_FontChooser::setFontDecoration(UT_Bool bUnderline, UT_Bool bStrikeOut)
{
	m_bUnderline = bUnderline;
	m_bStrikeOut = bStrikeOut;
}


AP_Dialog_FontChooser::tAnswer AP_Dialog_FontChooser::getAnswer(void) const
{
	return m_answer;
}

UT_Bool AP_Dialog_FontChooser::getChangedFontFamily(const XML_Char ** pszFontFamily) const
{
	if (pszFontFamily)
		*pszFontFamily = m_pFontFamily;
	return m_bChangedFontFamily;
}

UT_Bool AP_Dialog_FontChooser::getChangedFontSize(const XML_Char ** pszFontSize) const
{
	if (pszFontSize)
		*pszFontSize = m_pFontSize;
	return m_bChangedFontSize;
}

UT_Bool AP_Dialog_FontChooser::getChangedFontWeight(const XML_Char ** pszFontWeight) const
{
	if (pszFontWeight)
		*pszFontWeight = m_pFontWeight;
	return m_bChangedFontWeight;
}

UT_Bool AP_Dialog_FontChooser::getChangedFontStyle(const XML_Char ** pszFontStyle) const
{
	if (pszFontStyle)
		*pszFontStyle = m_pFontStyle;
	return m_bChangedFontStyle;
}

UT_Bool AP_Dialog_FontChooser::getChangedColor(const XML_Char ** pszColor) const
{
	if (pszColor)
		*pszColor = m_pColor;
	return m_bChangedColor;
}

UT_Bool AP_Dialog_FontChooser::getChangedUnderline(UT_Bool * pbUnderline) const
{
	if (pbUnderline)
		*pbUnderline = m_bUnderline;
	return m_bChangedUnderline;
}

UT_Bool AP_Dialog_FontChooser::getChangedStrikeOut(UT_Bool * pbStrikeOut) const
{
	if (pbStrikeOut)
		*pbStrikeOut = m_bStrikeOut;
	return m_bChangedStrikeOut;
}
