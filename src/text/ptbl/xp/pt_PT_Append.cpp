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


#include "ut_types.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "pd_Style.h"

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

	// because we are loading, we do not create change
	// records or any of the other stuff that an insertSpan
	// would do.

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendObject(PTObjectType pto, const XML_Char ** attributes)
{
	// create a new object fragment at the current end of the document.
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	pf_Frag_Object * pfo = NULL;
	if (!_createObject(pto,indexAP,&pfo))
		return UT_FALSE;
	
	m_fragments.appendFrag(pfo);
	return UT_TRUE;
}


///////////////////////////////////////////////////////////////////
// Styles represent named collections of formatting properties.

UT_Bool pt_PieceTable::appendStyle(const XML_Char ** attributes)
{
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	// verify unique name

	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	const char * szName = UT_getAttribute("name", attributes);
	if (!szName || !*szName)
		return UT_TRUE;		// silently ignore unnamed styles

	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == UT_TRUE)
	{
		// duplicate name
		UT_ASSERT(pStyle);
		if (pStyle->isUserDefined())
			return UT_TRUE;	// already loaded, ignore redefinition

		// override builtin definition
		return pStyle->setIndexAP(indexAP);
	}
	else
	{
		// this is a new name
		pStyle = new PD_Style(this, indexAP);
		if (pStyle)
			if (m_hashStyles.addEntry(szName,NULL,(void *)pStyle) != -1)
				return UT_TRUE;

		// cleanup after failure
		if (pStyle)
			delete pStyle;
		return UT_FALSE;
	}
}

UT_Bool pt_PieceTable::_createBuiltinStyle(const char * szName, PT_AttrPropIndex indexAP)
{
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// verify unique name

	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == UT_TRUE)
		return UT_FALSE;		// duplicate name

	pStyle = new PD_BuiltinStyle(this, indexAP);
	if (pStyle)
		if (m_hashStyles.addEntry(szName,NULL,(void *)pStyle) != -1)
			return UT_TRUE;

	// cleanup after failure
	if (pStyle)
		delete pStyle;
	return UT_FALSE;
}

UT_Bool pt_PieceTable::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	UT_ASSERT(szName && *szName);
	
	UT_AlphaHashTable::UT_HashEntry * pHashEntry = m_hashStyles.findEntry(szName);
	if (!pHashEntry)
		return UT_FALSE;

	PD_Style * pStyle = (PD_Style *) pHashEntry->pData;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::enumStyles(UT_uint32 k,
								const char ** pszName, const PD_Style ** ppStyle) const
{
	// return the kth style.

	UT_uint32 kLimit = m_hashStyles.getEntryCount();
	if (k >= kLimit)
		return UT_FALSE;
	
	const UT_AlphaHashTable::UT_HashEntry * pHashEntry = m_hashStyles.getNthEntryAlpha(k);
	UT_ASSERT(pHashEntry);

	PD_Style * pStyle = (PD_Style *) pHashEntry->pData;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}

	if (pszName)
	{
		*pszName = pHashEntry->pszLeft;
	}
	
	return UT_TRUE;
}
	
