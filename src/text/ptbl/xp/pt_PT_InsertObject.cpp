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
#include "px_ChangeRecord_Object.h"
#include "px_ChangeRecord_ObjectChange.h"

/*****************************************************************/
/*****************************************************************/

UT_Bool pt_PieceTable::insertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const XML_Char ** attributes,
									const XML_Char ** properties)
{
	// TODO currently we force the caller to pass in the attr/prop.
	// TODO this is probably a good thing for Images, but might be
	// TODO bogus for things like Fields.
	
	UT_ASSERT(m_pts==PTS_Editing);

	PT_DocPosition dposTemp;
	if (_haveTempSpanFmt(&dposTemp,NULL))
	{
		// TODO What should we do with the TempSpanFmt ??  Should
		// TODO we use it (might be appropriate for Fields, but
		// TODO probably bogus for Images) or should we just blindly
		// TODO ignore it.  For now, I'm just going to clear it.
		clearTemporarySpanFmt();
	}
	
	// store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	// get the fragment at the given document position.
	
	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);

	if (!_insertObject(pf,fragOffset,pto,indexAP))
		return UT_FALSE;
	
	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
									 dpos,indexAP,pto);
	UT_ASSERT(pcr);

	pf_Frag_Strux * pfs = NULL;
	UT_Bool bFoundStrux = _getStruxFromPosition(pcr->getPosition(),&pfs);
	UT_ASSERT(bFoundStrux);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
}
	
UT_Bool pt_PieceTable::_createObject(PTObjectType pto,
									 PT_AttrPropIndex indexAP,
									 pf_Frag_Object ** ppfo)
{
	// create an object frag for this.
	// return *pfo and true if successful.
	// create an unlinked object fragment.

	pf_Frag_Object * pfo = NULL;
	switch(pto)
	{
	case PTO_Image:
		pfo = new pf_Frag_Object(this,pto,indexAP);
		break;

	case PTO_Field:
		pfo = new pf_Frag_Object(this,pto,indexAP);
		break;
		
	default:
		UT_ASSERT(0);
		break;
	}

	if (!pfo)
	{
		UT_DEBUGMSG(("Could not create object fragment.\n"));
		// we forget about the AP that we created.
		return UT_FALSE;
	}

	*ppfo = pfo;
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_insertObject(pf_Frag * pf,
									 PT_BlockOffset fragOffset,									 
									 PTObjectType pto,
									 PT_AttrPropIndex indexAP)
{
	pf_Frag_Object * pfo = NULL;
	if (!_createObject(pto,indexAP,&pfo))
		return UT_FALSE;

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
		pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP());
		if (!pftTail)
			goto MemoryError;
			
		pft->changeLength(fragOffset);
		m_fragments.insertFrag(pft,pfo);
		m_fragments.insertFrag(pfo,pftTail);
	}

	return UT_TRUE;

MemoryError:
	if (pfo)
		delete pfo;
	return UT_FALSE;
}
