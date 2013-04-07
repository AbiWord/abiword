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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#include "fp_FieldTOCNum.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"
#include "pd_Document.h"
#include "fl_DocLayout.h"
#include "fp_Line.h"
#include "ut_debugmsg.h"
#include "fl_TOCLayout.h"
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
	fp_Line * pLine =  static_cast<fp_Line *>(pBlockInDoc->getFirstContainer());
	UT_sint32 kk = 0;
	bool b_goodLine = false;
	while (pLine && !b_goodLine)
	{
	    for (kk = 0; kk < pLine->getNumRunsInLine(); kk++)
	    {
		if(pLine->getRunFromIndex(kk)->getType() == FPRUN_TEXT)
		{
		    b_goodLine = true;
		    break;
		}
	    }
	    if (!b_goodLine)
	    {
		pLine = static_cast<fp_Line *>(pLine->getNext());
	    }
	}
	if(pLine == NULL)
	{
		sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
		sz_ucs_FieldValue[1] = 0;
		return _setValue(sz_ucs_FieldValue);
	}

	fp_Page * pPage = pLine->getPage();
        if (pPage == NULL)
        {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
            return false;
        }
	UT_sint32 iPage = pPage->getFieldPageNumber();
	bool b_hasSetFieldPageNumber = false;
	if(iPage < 0)
	{
	    pPage->resetFieldPageNumber();
	    iPage = pPage->getFieldPageNumber();
	    b_hasSetFieldPageNumber = true;
	    if (iPage < 0)
	    {
		sz_ucs_FieldValue[0] = static_cast<UT_UCSChar>(' ');
		sz_ucs_FieldValue[1] = 0;
		return _setValue(sz_ucs_FieldValue);
	    }
	}
	UT_String sVal("");
	FootnoteType iType = getBlock()->getTOCNumType();
	pLayout->getStringFromFootnoteVal(sVal,iPage,iType);
	const char * psz = sVal.c_str();

	if (b_hasSetFieldPageNumber)
	{
	    // We need to set the field page number value to -1 so that we 
	    // recalculate the page number next time we enter this function
	    pPage->setFieldPageNumber(-1);
	}
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

/////////////////////////////////////////////////////////////////////////////


fp_FieldTOCListLabelRun::fp_FieldTOCListLabelRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL,iOffsetFirst, iLen)
{
    UT_ASSERT(pBL);
	_setDirection(pBL->getDominantDirection());
	_setLength(0);
}

bool fp_FieldTOCListLabelRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
//
// First get owning TOC.
//
	UT_ASSERT(getLength() == 0);
	fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getBlock()->myContainingLayout());
	UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
	UT_String str = pTOCL->getTOCListLabel(getBlock()).utf8_str();
	if(str.size() == 0)
	{
		sz_ucs_FieldValue[0] = 0;
		return _setValue(sz_ucs_FieldValue);
	}
	UT_sint32 i = 0;
	bool bStop = false;
	for(i=0; (i<FPFIELD_MAX_LENGTH) && !bStop; i++)
	{
		sz_ucs_FieldValue[i] = static_cast<UT_UCSChar>(str[i]);
		if(str[i] == 0)
		{
			bStop = true;
		}
	}
	return _setValue(sz_ucs_FieldValue);
}

void fp_FieldTOCListLabelRun::_draw(dg_DrawArgs* pDA)
{
	_defaultDraw(pDA);
}

void fp_FieldTOCListLabelRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP,
											 GR_Graphics * pG)
{
  fp_FieldRun::_lookupProperties (pSpanAP, pBlockAP, pSectionAP,pG) ;
  _setLength(0);
}

////////////////////////////////////////////////////////////////////////////


fp_FieldTOCHeadingRun::fp_FieldTOCHeadingRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL,iOffsetFirst, iLen)
{
    UT_ASSERT(pBL);
	_setDirection(pBL->getDominantDirection());
	_setLength(0);
}

bool fp_FieldTOCHeadingRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
//
// First get owning TOC.
//
	UT_ASSERT(getLength() == 0);
	fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getBlock()->myContainingLayout());
	UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
	UT_UCS4String str = pTOCL->getTOCHeading().ucs4_str();
	if(str.size() == 0)
	{
		sz_ucs_FieldValue[0] = 0;
		return _setValue(sz_ucs_FieldValue);
	}
	UT_sint32 i = 0;
	bool bStop = false;
	
	for(i=0; (i<FPFIELD_MAX_LENGTH) && !bStop; i++)
	{
		sz_ucs_FieldValue[i] = static_cast<UT_UCSChar>(str[i]);
		if(str[i] == 0)
		{
			bStop = true;
		}
	}
	return _setValue(sz_ucs_FieldValue);
}

void fp_FieldTOCHeadingRun::_draw(dg_DrawArgs* pDA)
{
	_defaultDraw(pDA);
}

void fp_FieldTOCHeadingRun::_lookupProperties(const PP_AttrProp * pSpanAP,
											 const PP_AttrProp * pBlockAP,
											 const PP_AttrProp * pSectionAP,
											 GR_Graphics * pG)
{
  fp_FieldRun::_lookupProperties (pSpanAP, pBlockAP, pSectionAP,pG) ;
  _setLength(0);
}




