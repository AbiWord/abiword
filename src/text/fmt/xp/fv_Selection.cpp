/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_Selection.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "fv_View.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "fl_TableLayout.h"
#include "pd_Document.h"
#include "ie_exp.h"
#include "ie_exp_RTF.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"

FV_Selection::FV_Selection (FV_View * pView)
	: m_pView (pView), 
	  m_iSelectionMode(FV_SelectionMode_NONE),
	  m_iSelectAnchor(0),
	  m_iSelectLeftAnchor(0),
	  m_iSelectRightAnchor(0),
	  m_pTableOfSelectedColumn(NULL)
{
	UT_ASSERT (pView);
	m_vecSelRanges.clear();
	m_vecSelRTFBuffers.clear();
}

FV_Selection::~FV_Selection()
{
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
	UT_VECTOR_PURGEALL(IE_Exp_RTF *,m_vecSelRTFBuffers);
}

void FV_Selection::setMode(FV_SelectionMode iSelMode)
{
	m_iSelectionMode = iSelMode;
	if((m_iSelectionMode == FV_SelectionMode_NONE) || (m_iSelectionMode == FV_SelectionMode_Single) )
	{
		m_pTableOfSelectedColumn = NULL;
		UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
		UT_VECTOR_PURGEALL(IE_Exp_RTF *,m_vecSelRTFBuffers);
	}
}

PD_Document * FV_Selection::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout * FV_Selection::getLayout(void) const
{
	return m_pView->getLayout();
}

PT_DocPosition FV_Selection::getSelectionAnchor(void) const
{
	return m_iSelectAnchor;
}

void FV_Selection::setSelectionAnchor(PT_DocPosition pos)
{
	m_iSelectAnchor = pos;
}


PT_DocPosition FV_Selection::getSelectionLeftAnchor(void) const
{
	return m_iSelectLeftAnchor;
}

void FV_Selection::setSelectionLeftAnchor(PT_DocPosition pos)
{
	m_iSelectLeftAnchor = pos;
}


PT_DocPosition FV_Selection::getSelectionRightAnchor(void) const
{
	return m_iSelectRightAnchor;
}

void FV_Selection::setSelectionRightAnchor(PT_DocPosition pos)
{
	m_iSelectRightAnchor = pos;
}

bool FV_Selection::isPosSelected(PT_DocPosition pos) const
{
	PT_DocPosition posLow = m_iSelectAnchor;
	PT_DocPosition posHigh = m_pView->getPoint();
	if(posHigh > posLow)
	{
		posHigh = m_iSelectAnchor;
		posLow = m_pView->getPoint();
	}
	return (pos >= posLow) && (pos <=posHigh);
}

bool FV_Selection::isSelected(void) const
{
	return FV_SelectionMode_NONE != m_iSelectionMode;
}

void FV_Selection::clearSelection(void)
{
	setMode(FV_SelectionMode_NONE);
}
