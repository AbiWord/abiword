/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
 * (C) Martin Sevior 2004, , <msevior@physics.unimelb.edu.au>
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


#include "fp_FieldTOCNum.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"
#include "pd_Document.h"
#include "fl_DocLayout.h"
#include "fp_Line.h"
#include "ut_debugmsg.h"

#include <string.h>

fp_FieldTOCNumRun::fp_FieldTOCNumRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL,iOffsetFirst, iLen)
{
    UT_ASSERT(pBL);
	_setDirection(pBL->getDominantDirection());
}

bool fp_FieldTOCNumRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
//
// First Find page number.
//
	PL_StruxDocHandle sdh = getBlock()->getStruxDocHandle();
	PD_Document * pDoc = getBlock()->getDocument();
	PT_DocPosition pos = pDoc->getStruxPosition(sdh);
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	fl_BlockLayout * pBlockInDoc = pLayout->findBlockAtPosition(pos);
	if(pBlockInDoc == NULL)
	{
		sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
		sz_ucs_FieldValue[1] = 0;
		return _setValue(sz_ucs_FieldValue);
	}
	fp_Line * pLine =  static_cast<fp_Line *>(pBlockInDoc->getFirstContainer());
	if(pLine == NULL)
	{
		sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
		sz_ucs_FieldValue[1] = 0;
		return _setValue(sz_ucs_FieldValue);
	}
	fp_Page * pPage = pLine->getPage();
	UT_sint32 iPage = pLayout->findPage(pPage);
	if( iPage < 0)
	{
		sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
		sz_ucs_FieldValue[1] = 0;
		return _setValue(sz_ucs_FieldValue);
	}
	iPage++; // Start from Page 1.
	UT_String sVal("");
	FootnoteType iType = getBlock()->getTOCNumType();
	pLayout->getStringFromFootnoteVal(sVal,iPage,iType);
	char * psz = const_cast<char *>(sVal.c_str());
	bool bStop = false;
	UT_sint32 i = 0;
	sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
	for(i=1; (i<FPFIELD_MAX_LENGTH) && !bStop; i++)
	{
		sz_ucs_FieldValue[i] = static_cast<UT_UCSChar>(*psz);
		if(*psz == 0)
		{
			bStop = true;
		}
		else
		{
			psz++;
		}
	}
	return _setValue(sz_ucs_FieldValue);
}

void fp_FieldTOCNumRun::_draw(dg_DrawArgs* pDA)
{
	_defaultDraw(pDA);
}

void fp_FieldTOCNumRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP,
											 GR_Graphics * pG)
{
  fp_FieldRun::_lookupProperties (pSpanAP, pBlockAP, pSectionAP,pG) ;
}




