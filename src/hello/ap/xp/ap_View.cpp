/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include "ut_types.h"
#include "ev_EditBits.h"

#include "ap_View.h"
#include "gr_Graphics.h"

AP_View::AP_View(XAP_App* pApp, GR_Graphics* pG, void* pData)
	: AV_View(pApp, pData)
{
	m_pG = pG;
}

AP_View::~AP_View(void)
{
}

void AP_View::setXScrollOffset(UT_sint32 dx)
{
}

void AP_View::setYScrollOffset(UT_sint32 dy)
{
}

void AP_View::draw(const UT_Rect* pRect)
{
	if (!m_pG)
		return;

	UT_UCSChar buf[2048];
	UT_UCS_strcpy_char(buf, "Hello, Abi");
	m_pG->drawChars(buf, 0, 10, 50, 50);
}

void AP_View::cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos)
{
}

UT_Bool AP_View::notifyListeners(const AV_ChangeMask hint)
{
	return UT_TRUE;
}

UT_Bool AP_View::canDo(UT_Bool bUndo) const
{
	return UT_FALSE;
}

void AP_View::cmdUndo(UT_uint32 count)
{

}

void AP_View::cmdRedo(UT_uint32 count)
{

}
	
UT_Bool AP_View::cmdSave(void)
{
	return UT_FALSE;
}

UT_Bool AP_View::cmdSaveAs(const char * szFilename, int ieft)
{
	return UT_FALSE;
}

EV_EditMouseContext AP_View::getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	return 0;
}

EV_EditMouseContext AP_View::getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos)
{
	return 0;
}


