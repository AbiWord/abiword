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

// insert-object-related routines for piece table.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"

/*****************************************************************/
/*****************************************************************/
bool pt_PieceTable::insertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const XML_Char ** attributes,
									  const XML_Char ** properties,  pf_Frag_Object ** ppfo)
{
	return _realInsertObject(dpos, pto, attributes, properties, ppfo);
}

bool pt_PieceTable::insertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const XML_Char ** attributes,
									  const XML_Char ** properties )
{
	return _realInsertObject(dpos, pto, attributes, properties);
}

bool pt_PieceTable::_realInsertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const XML_Char ** attributes,
									const XML_Char ** properties,  pf_Frag_Object ** ppfo)
{
	UT_ASSERT(properties == NULL);

	// dpos == 1 seems to be generally bad. - plam
	// I'm curious about how often it happens.  Please mail me if it does!
	UT_ASSERT(dpos > 1);

	// TODO currently we force the caller to pass in the attr/prop.
	// TODO this is probably a good thing for Images, but might be
	// TODO bogus for things like Fields.

	UT_ASSERT(m_pts==PTS_Editing);

	// store the attributes and properties and get an index to them.
	PT_AttrPropIndex apiOld = 0, indexAP;

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);
	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = _getStruxFromFrag(pf,&pfs);

	UT_ASSERT(bFoundStrux);
	if(isEndFootnote((pf_Frag *) pfs))
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *)pfs,&pfs);
	}
	UT_ASSERT(bFoundStrux);
	
	apiOld = _chooseIndexAP(pf,fragOffset);

	if (!m_varset.mergeAP(PTC_AddFmt, apiOld, attributes, properties, &indexAP, m_pDocument))
		return false;

	// get the fragment at the given document position.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
        pf_Frag_Object * pfo = NULL;
	if (!_insertObject(pf,fragOffset,pto,indexAP,pfo))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
									 dpos,indexAP,pto,blockOffset,
                                     pfo->getField());
	UT_ASSERT(pcr);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);
        *ppfo = pfo;
	return true;
}


bool pt_PieceTable::_realInsertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const XML_Char ** attributes,
									const XML_Char ** properties )
{
	UT_ASSERT(properties == NULL);

	// dpos == 1 seems to be generally bad. - plam
	// I'm curious about how often it happens.  Please mail me if it does!
	UT_ASSERT(dpos > 1);

	// TODO currently we force the caller to pass in the attr/prop.
	// TODO this is probably a good thing for Images, but might be
	// TODO bogus for things like Fields.

	UT_ASSERT(m_pts==PTS_Editing);

	// store the attributes and properties and get an index to them.

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	// get the fragment at the given document position.

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);

	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_ASSERT(bFoundStrux);
	if(isEndFootnote((pf_Frag *) pfs))
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *)pfs,&pfs);
	}
	UT_ASSERT(bFoundStrux);
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
    pf_Frag_Object * pfo = NULL;
	if (!_insertObject(pf,fragOffset,pto,indexAP,pfo))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
									 dpos,indexAP,pto,blockOffset,
                                     pfo->getField());
	UT_ASSERT(pcr);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

bool pt_PieceTable::_createObject(PTObjectType pto,
									 PT_AttrPropIndex indexAP,
									 pf_Frag_Object ** ppfo)
{
	// create an object frag for this.
	// return *pfo and true if successful.
	// create an unlinked object fragment.

	pf_Frag_Object * pfo = NULL;
	switch(pto)
	{
		case PTO_Hyperlink:
		case PTO_Image:
		case PTO_Field:
			{
				pfo = new pf_Frag_Object(this,pto,indexAP);
			}
			break;
		case PTO_Bookmark:
			{
				pfo = new pf_Frag_Object(this,pto,indexAP);
				po_Bookmark * pB = pfo->getBookmark();
				UT_ASSERT(pB);
				if(pB->getBookmarkType() == po_Bookmark::POBOOKMARK_START)
					m_pDocument->addBookmark(pB->getName());
			}
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	if (!pfo)
	{
		UT_DEBUGMSG(("Could not create object fragment.\n"));
		// we forget about the AP that we created.
		return false;
	}

	*ppfo = pfo;
	return true;
}

bool pt_PieceTable::_insertObject(pf_Frag * pf,
									 PT_BlockOffset fragOffset,
									 PTObjectType pto,
									 PT_AttrPropIndex indexAP,
                                     pf_Frag_Object * & pfo)
{
	pfo = NULL;
	if (!_createObject(pto,indexAP,&pfo))
		return false;

	if (fragOffset == 0)
	{
		// we are at the beginning of a fragment, insert the
		// new object immediately prior to it.
		m_fragments.insertFrag(pf->getPrev(),pfo);
	}
	else if (fragOffset == pf->getLength())
	{
		// we are at the end of a fragment, insert the new
		// object immediately after it.
		m_fragments.insertFrag(pf,pfo);
	}
	else
	{
		// if the insert is in the middle of the (text) fragment, we
		// split the current fragment and insert the object between
		// them.

		UT_ASSERT(pf->getType() == pf_Frag::PFT_Text);
		pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
		UT_uint32 lenTail = pft->getLength() - fragOffset;
		PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
		pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
		if (!pftTail)
			goto MemoryError;

		pft->changeLength(fragOffset);
		m_fragments.insertFrag(pft,pfo);
		m_fragments.insertFrag(pfo,pftTail);
	}

	return true;

MemoryError:
	if (pfo)
		delete pfo;
	return false;
}
