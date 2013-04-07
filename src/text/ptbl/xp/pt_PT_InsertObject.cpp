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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "pp_Revision.h"

/*****************************************************************/
/*****************************************************************/
bool pt_PieceTable::insertObject(PT_DocPosition dpos,
								 PTObjectType pto,
								 const gchar ** attributes,
								 const gchar ** properties,  pf_Frag_Object ** ppfo)
{
	if(m_pDocument->isMarkRevisions())
	{
		PP_RevisionAttr Revisions(NULL);
		const gchar ** ppRevAttrs  = NULL;
		const gchar ** ppRevProps  = NULL;

		pf_Frag * pf = NULL;
		PT_BlockOffset fragOffset = 0;
		bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
		UT_return_val_if_fail( bFound, false );

		if(pf->getType() == pf_Frag::PFT_EndOfDoc)
			pf = pf->getPrev();

		UT_return_val_if_fail( pf, false );
		
		PT_AttrPropIndex indexAP = pf->getIndexAP();
		//UT_uint32 length = pf->getLength();
		
		_translateRevisionAttribute(Revisions, indexAP, PP_REVISION_ADDITION,
									ppRevAttrs, ppRevProps, NULL, NULL);

		// count original attributes and the revision-inherited
		// attributes and add them to the revision attribute
		UT_uint32 iAttrCount = 0;
		for (; attributes && attributes[iAttrCount]; iAttrCount+=2){}

		UT_uint32 iRevAttrCount = 0;
		for (; ppRevAttrs && ppRevAttrs[iRevAttrCount]; iRevAttrCount+=2){}

		const gchar ** ppRevAttrib = NULL;
		if(iAttrCount + iRevAttrCount > 0)
		{
			ppRevAttrib = new const gchar * [iAttrCount + iRevAttrCount + 1];
			UT_return_val_if_fail( ppRevAttrib, false );

			UT_uint32 i = 0;
			for (i = 0; i < iAttrCount; ++i)
			{
				ppRevAttrib[i] = attributes[i];
			}

			for (; i < iRevAttrCount + iAttrCount; ++i)
			{
				ppRevAttrib[i] = ppRevAttrs[i - iAttrCount];
			}
		
			ppRevAttrib[i]   = NULL;
		}
		
		//return _realChangeSpanFmt(PTC_AddFmt, dpos, dpos + length,
		//ppRevAttrib, ppRevProps);
		// NB: objects are not supposed to have props, and so do not
		//inherit props ...
		bool bRet =  _realInsertObject(dpos, pto, ppRevAttrib, /*ppRevProps*/properties, ppfo);
		delete [] ppRevAttrib;
		return bRet;
	}
	else
	{
		return _realInsertObject(dpos, pto, attributes, properties, ppfo);
	}
}

bool pt_PieceTable::insertObject(PT_DocPosition dpos,
								 PTObjectType pto,
								 const gchar ** attributes,
								 const gchar ** properties )
{
	if(m_pDocument->isMarkRevisions())
	{
		PP_RevisionAttr Revisions(NULL);
		const gchar ** ppRevAttrs = NULL;
		const gchar ** ppRevProps  = NULL;

		pf_Frag * pf = NULL;
		PT_BlockOffset fragOffset = 0;
		bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
		UT_return_val_if_fail( bFound, false );

		if(pf->getType() == pf_Frag::PFT_EndOfDoc)
			pf = pf->getPrev();

		UT_return_val_if_fail( pf, false );
		
		PT_AttrPropIndex indexAP = pf->getIndexAP();
		//UT_uint32 length = pf->getLength();

		_translateRevisionAttribute(Revisions, indexAP, PP_REVISION_ADDITION,
									ppRevAttrs, ppRevProps, attributes, properties);
		
		// count original attributes and the revision-inherited attributes and add them to the revision attribute
		UT_uint32 iAttrCount = 0;
		for (; attributes && attributes[iAttrCount]; iAttrCount+=2){}

		UT_uint32 iRevAttrCount = 0;
		for (; ppRevAttrs && ppRevAttrs[iRevAttrCount]; iRevAttrCount+=2){}

		const gchar ** ppRevAttrib = NULL;
		if(iAttrCount + iRevAttrCount > 0)
		{
			ppRevAttrib = new const gchar * [iAttrCount + iRevAttrCount + 1];
			UT_return_val_if_fail( ppRevAttrib, false );

			UT_uint32 i = 0;
			for (i = 0; i < iAttrCount; ++i)
			{
				ppRevAttrib[i] = attributes[i];
			}

			for (; i < iRevAttrCount + iAttrCount; ++i)
			{
				ppRevAttrib[i] = ppRevAttrs[i - iAttrCount];
			}
		
			ppRevAttrib[i]   = NULL;
		}

		// return _realChangeSpanFmt(PTC_AddFmt, dpos, dpos + length, ppRevAttrib, ppRevProps);
		// NB: objects are not supposed to have props, and so do not
		//inherit props ...
		bool bRet = _realInsertObject(dpos, pto, ppRevAttrib, /*ppRevProps*/properties);
		delete [] ppRevAttrib;
		return bRet;
	}
	else
	{
		return _realInsertObject(dpos, pto, attributes, properties);
	}
}

bool pt_PieceTable::_realInsertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const gchar ** attributes,
									const gchar ** properties,  pf_Frag_Object ** ppfo)
{
	UT_ASSERT_HARMLESS((pto == PTO_Math) || (pto == PTO_Embed) || (properties == NULL));

	// dpos == 1 seems to be generally bad. - plam
	// I'm curious about how often it happens.  Please mail me if it does!
	UT_ASSERT_HARMLESS(dpos > 1);

	// TODO currently we force the caller to pass in the attr/prop.
	// TODO this is probably a good thing for Images, but might be
	// TODO bogus for things like Fields.

	UT_return_val_if_fail (m_pts==PTS_Editing, false);

	// store the attributes and properties and get an index to them.
	PT_AttrPropIndex apiOld = 0, indexAP;

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_return_val_if_fail (bFound,false);
	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = _getStruxFromFrag(pf,&pfs);

	UT_return_val_if_fail (bFoundStrux,false);
	if(isEndFootnote((pf_Frag *) pfs))
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *)pfs,&pfs);
	}
	UT_return_val_if_fail (bFoundStrux, false);
	
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
									 dpos,indexAP, pfo->getXID(), pto,blockOffset,
                                     pfo->getField(),pfo);
	UT_return_val_if_fail (pcr,false);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);
        *ppfo = pfo;
	return true;
}


bool pt_PieceTable::_realInsertObject(PT_DocPosition dpos,
									PTObjectType pto,
									const gchar ** attributes,
									const gchar ** properties )
{

	// dpos == 1 seems to be generally bad. - plam
	// I'm curious about how often it happens.  Please mail me if it does!
	UT_ASSERT_HARMLESS(dpos > 1);

	// TODO currently we force the caller to pass in the attr/prop.
	// TODO this is probably a good thing for Images, but might be
	// TODO bogus for things like Fields.

	UT_return_val_if_fail (m_pts==PTS_Editing,false);

	// store the attributes and properties and get an index to them.
	UT_UTF8String sProps;
	UT_sint32 i = 0;
	sProps.clear();
	if(properties != NULL)
	{
	    for(i=0;(properties[i] != NULL);i+=2)
	    {
		UT_DEBUGMSG(("Object: szProps = |%s| \n",properties[i]));
		sProps +=properties[i];
		sProps += ":";
		sProps += properties[i+1];
		if(properties[i+2] != NULL)
		{
		    sProps += ";";
		}
	    }
	}
	UT_GenericVector<const gchar*>  Atts;
	Atts.clear();
	if(attributes)
	{
		for(i=0; attributes[i] != 0; i++)
		{
		    Atts.addItem(attributes[i]);
		}
	}
	if(sProps.size() > 0)
	{
	    Atts.addItem("props");
	    Atts.addItem(sProps.utf8_str());
	}
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(&Atts,&indexAP))
		return false;

	// get the fragment at the given document position.

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_return_val_if_fail (bFound,false);

	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_return_val_if_fail (bFoundStrux,false);
	if(isEndFootnote((pf_Frag *) pfs))
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *)pfs,&pfs);
	}
	UT_return_val_if_fail (bFoundStrux,false);
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
    pf_Frag_Object * pfo = NULL;
	if (!_insertObject(pf,fragOffset,pto,indexAP,pfo))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
									 dpos,indexAP,pfo->getXID(),pto,blockOffset,
                                     pfo->getField(),pfo);
	UT_return_val_if_fail (pcr,false);

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
		case PTO_Annotation:
		case PTO_RDFAnchor:
		case PTO_Image:
	        case PTO_Math:
	        case PTO_Embed:
		case PTO_Field:
			{
				pfo = new pf_Frag_Object(this,pto,indexAP);
			}
			break;
		case PTO_Bookmark:
			{
				pfo = new pf_Frag_Object(this,pto,indexAP);
				po_Bookmark * pB = pfo->getBookmark();
				UT_return_val_if_fail (pB,false);
				if(pB->getBookmarkType() == po_Bookmark::POBOOKMARK_START)
					m_pDocument->addBookmark(pB->getName());
			}
			break;

		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	pfo->setXID(getXID());
	
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

		UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Text, false);
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
