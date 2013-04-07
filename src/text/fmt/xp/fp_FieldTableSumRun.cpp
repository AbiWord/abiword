/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2004, Martin Sevior
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


#include "fp_Run.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include "fp_Line.h"
#include "fp_ContainerObject.h"
#include <string.h>
#include <math.h>
#include <sstream>

static bool bUseCurrency = false;
static char cCurrency = '$';

static double dGetVal(UT_UTF8String sVal)
{
	std::istringstream iStream(sVal.utf8_str());
	double d;
	iStream >> d;

	// any input succesfully converted?
	if(!iStream)
	{
		return 0;
	}
	// if so, and buffer is not empty yet, check
	// if all remaining characters are whitespaces
	if(iStream.rdbuf()->in_avail() != 0)
	{
		char c;
		do
		{
			iStream.get(c);
			if(!isspace(c))
			{
				return 0;
			}
		}
		while(iStream.gcount() != 0);
	}

	return d;
}

static void sFormatDouble(UT_UTF8String & sVal, double d)
{
	bool bUseFix2 = bUseCurrency;
	bool bUseInt = false;
	double res = 0.0000000001;
	if(fabs(d) > 0.0000000001)
	{
		res = res*d;
	}
	if(d < 1e9 && !bUseCurrency)
	{
		UT_sint32 iVal = static_cast<UT_sint32>(d);
		if(iVal >= 0)
		{
			double dd = static_cast<double>(iVal);
			if((d - dd) < res)
			{
				bUseInt = true;
			}
			if(!bUseInt)
			{
				dd = static_cast<double>(iVal+1);
				if((dd-d) < res)
				{
					bUseInt = true;
					d += 1.0;
				}
			}
		}
		else
		{
			double dd = static_cast<double>(iVal);
			if((dd -d) < res)
			{
				bUseInt = true;
			}
			if(!bUseInt)
			{
				dd = static_cast<double>(iVal-1);
				if((d - dd) < res)
				{
					bUseInt = true;
					d -= 1;
				}
			}
		}
		if(!bUseInt && !bUseFix2)
		{
			double a = d*100.0;
			double aa = a;
			UT_sint32 iValH = static_cast<UT_sint32>(a);
			if(iValH >= 0)
			{
				aa = static_cast<double>(iValH);
				if((a - aa) < res)
				{
					bUseFix2 = true;
				}
				if(!bUseFix2)
				{
					aa = static_cast<double>(iValH+1);
					if((aa-a) < res)
					{
						bUseFix2 = true;
						a += 1.0;
					}
				}
			}
			else
			{
				aa = static_cast<double>(iValH);
				if((aa -a) < res)
				{
					bUseFix2 = true;
				}
				if(!bUseFix2)
				{
					aa = static_cast<double>(iValH-1);
					if((a - aa) < res)
					{
						bUseFix2 = true;
						a -= 1;
					}
				}
			}
			if(bUseFix2)
			{
				if(fabs(a) < 1e9)
				{
					iValH = static_cast<UT_sint32>(a);
					d = static_cast<double>(iValH)/100.0;
				}
			}
		}
	}
	if(bUseInt)
	{
		UT_sint32 iVal = static_cast<UT_sint32>(d);
		UT_UTF8String_sprintf(sVal,"%d",iVal);
		return;
	}
	if(bUseFix2)
	{
		UT_String sFull("");
		if(bUseCurrency)
		{
			sFull += cCurrency;
		}
		sFull += "%.2f";
		const char * szFmt = sFull.c_str();
		UT_UTF8String_sprintf(sVal,szFmt,d);
		return;
	}
	UT_UTF8String_sprintf(sVal,"%g",d);
	return;
}

	fp_FieldTableSumRows::fp_FieldTableSumRows(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen)	: fp_FieldRun(pBL,  iOffsetFirst, iLen)
{
}

bool fp_FieldTableSumRows::calculateValue(void)
{
	FV_View * pView = _getView();
	pf_Frag_Strux* tableSDH= NULL;
	UT_sint32 numRows =0;
	UT_sint32 numCols = 0;
	bUseCurrency = false;
	cCurrency = '$';
	pf_Frag_Strux* sdh = getBlock()->getStruxDocHandle();
	PD_Document * pDoc = getBlock()->getDocument();
	if(pDoc->isPieceTableChanging())
	{
		return false;
	}
	if(getLine() == NULL)
	{
		return false;
	}
	fp_Container * pCol = getLine()->getColumn();
	if(pCol == NULL)
	{
		return false;
	}
	fp_ShadowContainer * pShad =NULL;
	fl_HdrFtrShadow * pShadL = NULL;
	if(pCol->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		pShad = static_cast<fp_ShadowContainer *>(pCol);
		pShadL = pShad->getShadow();
	}
	PT_DocPosition pos = pDoc->getStruxPosition(sdh)+1;
	pDoc->getStruxOfTypeFromPosition(pos,PTX_SectionTable,&tableSDH);
	pDoc-> getRowsColsFromTableSDH(tableSDH, pView->isShowRevisions(), pView->getRevisionLevel(), &numRows, &numCols);
	UT_UTF8String sValF;
	if(!pView->isInTable(pos))
	{
		sValF = "???";
		return _setValue(sValF.ucs4_str().ucs4_str());
	}
	fl_CellLayout * pCell = NULL;
	UT_sint32 myLeft,myRight,myTop,myBot;
	pView->getCellParams(pos,&myLeft,&myRight,&myTop,&myBot);
	UT_sint32 row = 0;
	UT_sint32 col = myLeft;
	UT_sint32 lastRow = -1;
	double dSum = 0.0;
	for(row = 0; row < numRows; row++)
	{
		pf_Frag_Strux* sdhCell = pDoc->getCellSDHFromRowCol(tableSDH,true,99999,row,col);
		UT_sint32 i = getBlock()->getDocLayout()->getLID();
		fl_ContainerLayout* fmtCell = pDoc->getNthFmtHandle(sdhCell,i);
		pCell = static_cast<fl_CellLayout *>(fmtCell);
		if(pCell->getTopAttach() == lastRow)
		{
			continue;
		}
		if((pCell->getTopAttach() == myTop) && (pCell->getLeftAttach() == myLeft))
		{
			continue;
		}
		UT_GrowBuf grText;
		pCell->appendTextToBuf(grText);
		if(grText.getLength() == 0)
		{
			fl_ContainerLayout * pC = pCell->getFirstLayout();
			while(pC)
			{
				if(pC->getContainerType() == FL_CONTAINER_BLOCK)
				{
					fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pC);
					if(pShadL)
					{
						pBL = static_cast<fl_BlockLayout *>(pShadL->findMatchingContainer(pBL));
					}
					if(pBL == NULL)
					{
						continue;
					}
					fp_Run * pRun = pBL->getFirstRun();
					while(pRun)
					{
						if(pRun->getType() == FPRUN_FIELD)
						{
							fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
							const UT_UCSChar * szVal = pFRun->getValue(); 
							sValF.clear();
							sValF.appendUCS4(szVal);
							dSum += dGetVal(sValF.utf8_str());
							pRun = NULL;
							pC = NULL;
							break;
						}
						pRun = pRun->getNextRun();
					}
				}
				if(pC)
				{
					pC = pC->getNext();
				}
			}
		}
		else
		{
			sValF.clear();
			sValF.appendUCS4(reinterpret_cast<const UT_UCS4Char *>(grText.getPointer(0)),grText.getLength());
			dSum += dGetVal(sValF.utf8_str());
		}
		lastRow = row;
	}
	sFormatDouble(sValF,dSum);
	return _setValue(sValF.ucs4_str().ucs4_str());
}

	fp_FieldTableSumCols::fp_FieldTableSumCols(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_FieldRun(pBL,  iOffsetFirst, iLen)
{
}

bool fp_FieldTableSumCols::calculateValue(void)
{
	FV_View * pView = _getView();
	pf_Frag_Strux* tableSDH= NULL;
	UT_sint32 numRows =0;
	UT_sint32 numCols = 0;
	bUseCurrency = false;
	cCurrency = '$';
	pf_Frag_Strux* sdh = getBlock()->getStruxDocHandle();
	PD_Document * pDoc = getBlock()->getDocument();
	if(pDoc->isPieceTableChanging())
	{
		return false;
	}
	if(getLine() == NULL)
	{
		return false;
	}
	fp_Container * pCol = getLine()->getColumn();
	if(pCol == NULL)
	{
		return false;
	}
	fp_ShadowContainer * pShad =NULL;
	fl_HdrFtrShadow * pShadL = NULL;
	if(pCol->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		pShad = static_cast<fp_ShadowContainer *>(pCol);
		pShadL = pShad->getShadow();
	}
	PT_DocPosition pos = pDoc->getStruxPosition(sdh)+1;
	pDoc->getStruxOfTypeFromPosition(pos,PTX_SectionTable,&tableSDH);
	pDoc-> getRowsColsFromTableSDH(tableSDH, pView->isShowRevisions(), pView->getRevisionLevel(), &numRows, &numCols);

	UT_UTF8String sValF;
	if(!pView->isInTable(pos))
	{
		sValF = "???";
		return _setValue(sValF.ucs4_str().ucs4_str());
	}

	fl_CellLayout * pCell = NULL;
	UT_sint32 myLeft,myRight,myTop,myBot;
	pView->getCellParams(pos,&myLeft,&myRight,&myTop,&myBot);
	UT_sint32 col = 0;
	UT_sint32 row = myTop;
	UT_sint32 lastCol = -1;
	double dSum = 0.0;
	for(col = 0; col < numCols; col++)
	{
		pf_Frag_Strux* sdhCell = pDoc->getCellSDHFromRowCol(tableSDH,true,99999,row,col);
		UT_sint32 i = getBlock()->getDocLayout()->getLID();
		fl_ContainerLayout* fmtCell = pDoc->getNthFmtHandle(sdhCell,i);
		pCell = static_cast<fl_CellLayout *>(fmtCell);
		if(pCell->getLeftAttach() == lastCol)
		{
			continue;
		}
		if((pCell->getTopAttach() == myTop) && (pCell->getLeftAttach() == myLeft))
		{
			continue;
		}
		UT_GrowBuf grText;
		pCell->appendTextToBuf(grText);
		if(grText.getLength() == 0)
		{
			fl_ContainerLayout * pC = pCell->getFirstLayout();
			while(pC)
			{
				if(pC->getContainerType() == FL_CONTAINER_BLOCK)
				{
					fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pC);
					if(pShadL)
					{
						pBL = static_cast<fl_BlockLayout *>(pShadL->findMatchingContainer(pBL));
					}
					if(pBL == NULL)
					{
						continue;
					}
					fp_Run * pRun = pBL->getFirstRun();
					while(pRun)
					{
						if(pRun->getType() == FPRUN_FIELD)
						{
							fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
							const  UT_UCS4Char * szVal = pFRun->getValue(); 
							sValF.clear();
							sValF.appendUCS4(szVal);
							dSum += dGetVal(sValF.utf8_str());
							pRun = NULL;
							pC = NULL;
							break;
						}
						pRun = pRun->getNextRun();
					}
				}
				if(pC)
				{
					pC = pC->getNext();
				}
			}
		}
		else
		{
			sValF.clear();
			sValF.appendUCS4(reinterpret_cast<const UT_UCS4Char *>(grText.getPointer(0)),grText.getLength());
			dSum += dGetVal(sValF.utf8_str());
		}
		lastCol = col;
	}
	sFormatDouble(sValF,dSum);
	return _setValue(sValF.ucs4_str().ucs4_str());
}
