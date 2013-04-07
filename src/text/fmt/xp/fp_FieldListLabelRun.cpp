/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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


#include "fp_FieldListLabelRun.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include <string.h>

fp_FieldListLabelRun::fp_FieldListLabelRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL,iOffsetFirst, iLen)
{
    UT_ASSERT(pBL);
	_setDirection(pBL->getDominantDirection());
}

bool fp_FieldListLabelRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	UT_uint32 i = 0;
	UT_UCSChar *  listlabel = NULL;
	if(getBlock()->isContainedByTOC())
	{
		xxx_UT_DEBUGMSG(("!!!!!!------!!!! ListLabel in TOC!!!! \n"));
//
// First Find the block in the document.
//
		pf_Frag_Strux* sdh = getBlock()->getStruxDocHandle();
		PD_Document * pDoc = getBlock()->getDocument();
		PT_DocPosition pos = pDoc->getStruxPosition(sdh)+1;
		FL_DocLayout * pLayout = getBlock()->getDocLayout();
		fl_BlockLayout * pBlockInDoc = pLayout->findBlockAtPosition(pos);
		if(pBlockInDoc == NULL)
		{
			sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
			sz_ucs_FieldValue[1] = 0;
			return _setValue(sz_ucs_FieldValue);
		}
		i = 0;
		listlabel = pBlockInDoc->getListLabel();

	}
	else
	{
		i = 0;
		listlabel = getBlock()->getListLabel();
	}
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
		UT_uint32 len = UT_MIN( UT_UCS4_strlen(listlabel),FPFIELD_MAX_LENGTH + 1)  ;
		for(i=0; i<=len;i++)
		{
			sz_ucs_FieldValue[i] =  *listlabel++;
		}
//		sz_ucs_FieldValue[len] = 0;
//		m_sFieldValue[0] =  0; // Force an update!!!
	}
	return _setValue(sz_ucs_FieldValue);
}

void fp_FieldListLabelRun::_draw(dg_DrawArgs* pDA)
{
	_defaultDraw(pDA);
}

void fp_FieldListLabelRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP,
											 GR_Graphics * pG)
{
  fp_FieldRun::_lookupProperties (pSpanAP, pBlockAP, pSectionAP,pG) ;
}




