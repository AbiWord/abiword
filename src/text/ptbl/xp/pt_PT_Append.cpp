 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_SpanChange.h"
#include "px_ChangeRecord_Strux.h"

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	// create a new structure fragment at the current end of the document.
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	pf_Frag_Strux * pfs = NULL;
	if (!_createStrux(pts,indexAP,&pfs))
		return UT_FALSE;
	
	m_fragments.appendFrag(pfs);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new Attribute/Property structure in the table
	// and set the current index to it.  the next span of text
	// (in this block) that comes in will then be set to these
	// attributes/properties.  becase we are loading, we do not
	// create a Fragment or a ChangeRecord.  (Formatting changes
	// are implicit at this point in time.)

	if (!m_varset.storeAP(attributes,&loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const UT_Vector * pVecAttributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	if (!m_varset.storeAP(pVecAttributes,&loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendSpan(UT_UCSChar * pbuf, UT_uint32 length)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// append the text data to the end of the buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(pbuf,length,&bi))
		return UT_FALSE;

	// set the formatting Attributes/Properties to that
	// of the last fmt set in this paragraph.

	// see if this span can be appended to the previous fragment
	// (perhaps the parser was a bit lazy in chunking up the data).

	pf_Frag * pfLast = m_fragments.getLast();
	if (pfLast->getType() == pf_Frag::PFT_Text)
	{
		pf_Frag_Text * pfLastText = static_cast<pf_Frag_Text *>(pfLast);
		if (   (pfLastText->getIndexAP() == loading.m_indexCurrentInlineAP)
			&& m_varset.isContiguous(pfLastText->getBufIndex(),pfLastText->getLength(),bi))
		{
			pfLastText->changeLength(pfLastText->getLength() + length);
			return UT_TRUE;
		}
	}
	
	// could not coalesce, so create a new fragment for this text span.

	pf_Frag_Text * pft = new pf_Frag_Text(this,bi,length,loading.m_indexCurrentInlineAP);
	if (!pft)
		return UT_FALSE;

	m_fragments.appendFrag(pft);

	// becauase we are loading, we do not create change
	// records or any of the other stuff that an insertSpan
	// would do.

	return UT_TRUE;
}
