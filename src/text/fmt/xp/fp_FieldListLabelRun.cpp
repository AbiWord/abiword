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


#include "fp_FieldListLabelRun.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"

#include <string.h>

fp_FieldListLabelRun::fp_FieldListLabelRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL, pG, iOffsetFirst, iLen)
{
}

UT_Bool fp_FieldListLabelRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	UT_uint32 i = 0;
	XML_Char *  listlabel = (XML_Char *) m_pBL->getListLabel();
	if(listlabel == NULL)
	{
		sz_ucs_FieldValue[0] = 0;
	}
	else
	{
	  //
	  // This code is here because UT_UCS_copy_char is broken
	  //
		i = 0;
		UT_uint32 len = UT_MIN(strlen(listlabel),FPFIELD_MAX_LENGTH + 1)  ;
		for(i=0; i<=len;i++)
		{
		        sz_ucs_FieldValue[i] = (UT_UCSChar) (unsigned char) *listlabel++;
		}
		sz_ucs_FieldValue[len] = 0;
		m_sFieldValue[0] =  0; // Force an update!!!
	}
	return _setValue(sz_ucs_FieldValue);
}

void fp_FieldListLabelRun::_draw(dg_DrawArgs* pDA)
{
	_defaultDraw(pDA);
}

