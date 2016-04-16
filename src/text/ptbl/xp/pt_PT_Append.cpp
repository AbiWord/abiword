/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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

#include <ctype.h>
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"

/****************************************************************/
/****************************************************************/

bool pt_PieceTable::appendStrux(PTStruxType pts, const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret)
{
	pf_Frag_Strux * pfs = NULL;
	if(!_makeStrux(pts, attributes, pfs) || !pfs)
		return false;

	if(!attributes.empty())
	{
		std::string pXID = PP_getAttribute(PT_XID_ATTRIBUTE_NAME, attributes);
		UT_uint32 iXID = 0;
		if(!pXID.empty())
		{
			iXID = atoi(pXID.c_str());
			pfs->setXID(iXID);
		}
	}

	pf_Frag * pfPrev = m_fragments.getLast();
	bool bDoInsertFmt = false;
	if(pfPrev != NULL && pfPrev->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfsPrev = static_cast<pf_Frag_Strux *>(pfPrev);
		if(pfsPrev->getStruxType() == PTX_Block)
		{
			bDoInsertFmt = true;
		}
	}
	m_fragments.appendFrag(pfs);
	// insert frag in the embedded_strux list if needed
	if ((pts == PTX_EndFootnote) || (pts == PTX_EndEndnote) || (pts == PTX_EndAnnotation))
	{
		_insertNoteInEmbeddedStruxList(pfs);
	}

	if(bDoInsertFmt)
	{
		insertFmtMarkBeforeFrag(static_cast<pf_Frag *>(pfs));
	}
	if (ppfs_ret)
		*ppfs_ret = pfs;
	return true;
}

/**
 * MIQ11: Extends the old _findLastStruxOfType adding a stopCondition
 * for failure and returning a Strux* directly in the case of success.
 * This is like a findBackwards() from a fragment.
 * 
 * stopConditions must be terminated with a PTX_StruxDummy entry like:
 * PTStruxType stopCondition[] = { PTX_SectionTable, PTX_StruxDummy };
 * 
 * Find a fragment of strux type pst looking backwards from pfStart.
 * If a strux fragment matching the stopCondition is found first then
 * the function stops and returns 0. If no fragment with pst is found
 * then 0 is returned.
 *
 * MAYBE: extend this again to take yes() and no() functors so a
 *    function can call _findLastStruxOfType() and decide what is ok
 *    and what is not using those.
 *    boost::lambda would be handy to simplify the functors?
 */
pf_Frag_Strux* pt_PieceTable::_findLastStruxOfType( pf_Frag * pfStart,
                                                    PTStruxType pst,
                                                    PTStruxType* stopConditions,
                                                    bool bSkipEmbededSections )
{
	UT_return_val_if_fail( pfStart, NULL );

	pf_Frag * pf = pfStart;
    PTStruxType* stopConditionsBegin = stopConditions;
    PTStruxType* stopConditionsEnd   = stopConditions;
    while( *stopConditionsEnd != PTX_StruxDummy )
        ++stopConditionsEnd;

    while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfs2 = static_cast<pf_Frag_Strux*>(pf);

            PTStruxType eStruxType = pfs2->getStruxType();

			if( eStruxType == pst)
                return pfs2;
            
            if( stopConditionsEnd !=
                std::find( stopConditionsBegin, stopConditionsEnd, eStruxType ))
            {
                return 0;
            }
            
			if(bSkipEmbededSections)
			{
				if(pfs2->getStruxType() == PTX_EndTOC)
				{
					while(pf)
					{
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf);

							if(pfs->getStruxType() == PTX_SectionTOC)
								break;
						}

						pf = pf->getPrev();
					}
				}
				if(pfs2->getStruxType() == PTX_EndFrame)
				{
					while(pf)
					{
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf);

							if(pfs->getStruxType() == PTX_SectionFrame)
								break;
						}

						pf = pf->getPrev();
					}
				}

				if(pfs2->getStruxType() == PTX_EndEndnote)
				{
					while(pf)
					{
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf);

							if(pfs->getStruxType() == PTX_SectionEndnote)
								break;
						}

						pf = pf->getPrev();
					}
				}
				if(pfs2->getStruxType() == PTX_EndFootnote)
				{
					while(pf)
					{
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf);

							if(pfs->getStruxType() == PTX_SectionFootnote)
								break;
						}

						pf = pf->getPrev();
					}
				}
				if(pfs2->getStruxType() == PTX_EndMarginnote)
				{
					while(pf)
					{
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf);

							if(pfs->getStruxType() == PTX_SectionMarginnote)
								break;
						}

						pf = pf->getPrev();
					}
				}
			}
		}
		if(pf)
			pf = pf->getPrev();
	}

	return 0;
}



pf_Frag_Strux* pt_PieceTable::_findLastStruxOfType( pf_Frag * pfStart,
                                                    PTStruxType pst,
                                                    bool bSkipEmbededSections )
{
	UT_return_val_if_fail( pfStart, NULL );
    PTStruxType stopCondition[] = { PTX_StruxDummy };
    return _findLastStruxOfType( pfStart, pst, stopCondition, bSkipEmbededSections );
}


/*!
    Changes formating of the last strux of type pts
    bSkipEmbededSections indicates whether when an end of an embeded section is
    encountered, the entire section is to be skipped over, for example if the end of the
    document looks like

    <p><footnote><p></p></footnote>

    when searching for <p> if bSkipEmbededSections == true the paragraph before <footnote>
    will be modified
*/
bool pt_PieceTable::appendLastStruxFmt(PTStruxType pst, const PP_PropertyVector & attributes, const PP_PropertyVector & props,
									   bool bSkipEmbededSections)
{
	// can only be used while loading the document
	UT_return_val_if_fail (m_pts==PTS_Loading,false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(), false);
	if (!m_fragments.getFirst())
		return false;

	pf_Frag * pf = m_fragments.getLast();

	UT_return_val_if_fail ( pf, false );

	pf = _findLastStruxOfType(pf, pst, bSkipEmbededSections);

	UT_return_val_if_fail( pf, false );

	PT_AttrPropIndex currentAP = pf->getIndexAP();

	const PP_AttrProp * pOldAP;
    if(!getAttrProp(currentAP,&pOldAP))
		return false;

	PP_AttrProp * pNewAP = pOldAP->cloneWithReplacements(attributes, props, false);
	pNewAP->markReadOnly();

	PT_AttrPropIndex indexAP;
	if (!m_varset.addIfUniqueAP(pNewAP,&indexAP))
		return false;

	pf->setIndexAP(indexAP);

	return true;
}

/*!
    As above, but props represented by a single XML string
*/
bool pt_PieceTable::appendLastStruxFmt(PTStruxType pst, const PP_PropertyVector & attributes, const std::string & props,
									   bool bSkipEmbededSections)
{
	// XXX there is a lot in common with changeLastStruxFmt()
	if(!props.empty())
	{
		// we parse the xml props string into separate field by simply duplicating it and then
		// replacing ; and : with '0';

		// foolproofing
		// pProps belong to the std::string.
		const char* pProps = props.c_str();
		if(*pProps == ';')
			pProps++;

		char * pProps2 = g_strdup(pProps);

		const gchar ** pPropsArray = UT_splitPropsToArray(pProps2);
		UT_return_val_if_fail( pPropsArray, false );

		bool bRet = appendLastStruxFmt(pst, attributes, PP_std_copyProps(pPropsArray), bSkipEmbededSections);

		delete [] pPropsArray;
		FREEP(pProps2);

		return bRet;
	}
	else
	{
		return appendLastStruxFmt(pst, attributes, PP_NOPROPS, bSkipEmbededSections);
	}
}

/*! changes formatting of a strux while loading document */
bool pt_PieceTable::appendStruxFmt(pf_Frag_Strux * pfs, const PP_PropertyVector & attributes)
{
	// can only be used while loading the document
	UT_return_val_if_fail (m_pts==PTS_Loading,false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(), false);
	if (!m_fragments.getFirst())
		return false;

	UT_return_val_if_fail ( pfs, false );

	PT_AttrPropIndex currentAP = pfs->getIndexAP();

	const PP_AttrProp * pOldAP;
    if(!getAttrProp(currentAP,&pOldAP))
		return false;

	PP_AttrProp * pNewAP = pOldAP->cloneWithReplacements(attributes, PP_NOPROPS, true);
	pNewAP->markReadOnly();

	PT_AttrPropIndex indexAP;
	if (!m_varset.addIfUniqueAP(pNewAP,&indexAP))
		return false;

	pfs->setIndexAP(indexAP);

	return true;
}

bool pt_PieceTable::appendFmt(const PP_PropertyVector & vecAttributes)
{
	// can only be used while loading the document
	UT_return_val_if_fail (m_pts==PTS_Loading, false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(),false);

	// create a new Attribute/Property structure in the table
	// and set the current index to it.  the next span of text
	// (in this block) that comes in will then be set to these
	// attributes/properties.  becase we are loading, we do not
	// create a Fragment or a ChangeRecord.  (Formatting changes
	// are implicit at this point in time.)

	if (!m_varset.storeAP(vecAttributes, &loading.m_indexCurrentInlineAP))
		return false;

	return true;
}

bool pt_PieceTable::appendSpan(const UT_UCSChar * pbuf, UT_uint32 length)
{
	// can only be used while loading the document
	UT_return_val_if_fail (m_pts==PTS_Loading, false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(),false);

	// append the text data to the end of the buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(pbuf,length,&bi))
		return false;

	// set the formatting Attributes/Properties to that
	// of the last fmt set in this paragraph.

	// see if this span can be appended to the previous fragment
	// (perhaps the parser was a bit lazy in chunking up the data).

	pf_Frag * pfLast = m_fragments.getLast();
	if ((pfLast != NULL) && (pfLast->getType() == pf_Frag::PFT_Text))
	{
		pf_Frag_Text * pfLastText = static_cast<pf_Frag_Text *>(pfLast);
		if (   (pfLastText->getIndexAP() == loading.m_indexCurrentInlineAP)
			&& m_varset.isContiguous(pfLastText->getBufIndex(),pfLastText->getLength(),bi))
		{
			pfLastText->changeLength(pfLastText->getLength() + length);
			return true;
		}
	}

	// could not coalesce, so create a new fragment for this text span.

	pf_Frag_Text * pft = new pf_Frag_Text(this,bi,length,loading.m_indexCurrentInlineAP,NULL);
	if (!pft)
		return false;

	m_fragments.appendFrag(pft);

	// because we are loading, we do not create change
	// records or any of the other stuff that an insertSpan
	// would do.

	return true;
}

bool pt_PieceTable::appendObject(PTObjectType pto, const PP_PropertyVector & attributes)
{
	pf_Frag_Object * pfo = NULL;
	if(!_makeObject(pto,attributes,pfo) || !pfo)
		return false;

	if(!attributes.empty())
	{
		const std::string & pXID =
			PP_getAttribute(PT_XID_ATTRIBUTE_NAME, attributes);
		UT_uint32 iXID = 0;
		if(!pXID.empty())
		{
			iXID = atoi(pXID.c_str());
			pfo->setXID(iXID);
		}
	}

	m_fragments.appendFrag(pfo);
	return true;
}

bool pt_PieceTable::appendFmtMark(void)
{
	pf_Frag_FmtMark * pff = NULL;
	if (!_makeFmtMark(pff) || !pff)
		return false;

	m_fragments.appendFrag(pff);
	return true;
}

bool pt_PieceTable::insertStruxBeforeFrag(pf_Frag * pF, PTStruxType pts,
										  const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret)
{
	UT_return_val_if_fail(pF , false);

	pf_Frag_Strux * pfs = NULL;
	if(!_makeStrux(pts, attributes, pfs) || !pfs)
		return false;

	if(!attributes.empty())
	{
		std::string pXID = PP_getAttribute(PT_XID_ATTRIBUTE_NAME, attributes);
		UT_uint32 iXID = 0;
		if(!pXID.empty())
		{
			iXID = atoi(pXID.c_str());
			pfs->setXID(iXID);
		}
	}

	m_fragments.insertFragBefore(pF, pfs);
	if (ppfs_ret)
		*ppfs_ret = pfs;
	// insert frag in the embedded_strux list if needed
	if ((pts == PTX_EndFootnote) || (pts == PTX_EndEndnote) || (pts == PTX_EndAnnotation))
	{
		_insertNoteInEmbeddedStruxList(pfs);
	}

	return true;
}


bool pt_PieceTable::insertSpanBeforeFrag(pf_Frag * pf, const UT_UCSChar * p, UT_uint32 length)
{
	// can only be used while loading the document
	UT_return_val_if_fail (m_pts==PTS_Loading, false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(),false);

	// cannot insert before first fragment (i.e., span cannot start a document)
	UT_return_val_if_fail(pf && pf->getPrev() && pf != m_fragments.getFirst(), false);

	// append the text to the buffer
	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return false;

	// update the fragment and/or the fragment list.
	// return true if successful.

	pf_Frag_Text * pft = NULL;

	// see if the fragement before this one is a text frag ...
	if (pf->getPrev()->getType() == pf_Frag::PFT_Text)
	{
		pft = static_cast<pf_Frag_Text *>(pf->getPrev());
	}

	if (pft)
	{
		// We have a text frag on the left.  Try to coalesce this
		// character data with an existing fragment.

		if((pft->getIndexAP() == loading.m_indexCurrentInlineAP) &&
		   m_varset.isContiguous(pft->getBufIndex(),pft->getLength(),bi))
		{
			// new text is contiguous, we just update the length of this fragment.
			pft->changeLength(pft->getLength()+length);

			// see if this (enlarged) fragment is now contiguous with the
			// one that follows -- it cannot be when we are appending
			return true;
		}
	}

	// new text is not contiguous on the left, we need to insert a new text
	// fragment into the list.  first we construct a new text fragment
	// for the data that we inserted.

	pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi,length,loading.m_indexCurrentInlineAP,NULL);
	if (!pftNew)
		return false;

	m_fragments.insertFragBefore(pf,pftNew);

	// no need to check for the new frag being continguous with the
	// one on its right -- it cannot be, since the insertion occured in
	// oposite order than the fargments have in the buffer

	return true;
}

bool pt_PieceTable::insertObjectBeforeFrag(pf_Frag * pF, PTObjectType pto,
										   const PP_PropertyVector & attributes)
{
	// cannot insert before first fragment
	UT_return_val_if_fail(pF && pF->getPrev() && pF != m_fragments.getFirst(), false);

	pf_Frag_Object * pfo = NULL;
	if(!_makeObject(pto, attributes, pfo) || !pfo)
		return false;

	if(!attributes.empty())
	{
		std::string pXID = PP_getAttribute(PT_XID_ATTRIBUTE_NAME, attributes);
		UT_uint32 iXID = 0;
		if(!pXID.empty())
		{
			iXID = atoi(pXID.c_str());
			pfo->setXID(iXID);
		}
	}

	m_fragments.insertFragBefore(pF, pfo);
	return true;
}

bool pt_PieceTable::insertFmtMarkBeforeFrag(pf_Frag * pF)
{
	// cannot insert before first fragment
	UT_return_val_if_fail(pF && pF->getPrev() && pF != m_fragments.getFirst(), false);

	pf_Frag_FmtMark * pff = NULL;
	if (!_makeFmtMark(pff) || !pff)
		return false;

	m_fragments.insertFragBefore(pF, pff);
	return true;
}


bool pt_PieceTable::insertFmtMarkBeforeFrag(pf_Frag * pF, const PP_PropertyVector & attributes)
{
	// cannot insert before first fragment
	UT_return_val_if_fail(pF && pF->getPrev() && pF != m_fragments.getFirst(), false);

	pf_Frag_FmtMark * pff = NULL;
	if (!_makeFmtMark(pff,attributes) || !pff)
		return false;

	m_fragments.insertFragBefore(pF, pff);
	return true;
}

bool pt_PieceTable::_makeStrux(PTStruxType pts, const PP_PropertyVector & attributes, pf_Frag_Strux * &pfs)
{
	// create a new structure fragment at the current end of the document.
	// this function can only be called while loading the document.
	UT_return_val_if_fail (m_pts==PTS_Loading, false);

	// first, store the attributes and properties and get an index to them.

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	//
	// OK we've got to interogate attributes to determine what sort of section strux
	// we have.
	//
	if((pts == PTX_Section) && (!attributes.empty()))
	{
		const std::string & struxType = PP_getAttribute("type", attributes);
		if(!struxType.empty())
		{
			if(struxType == "header" ||
			   struxType == "footer" ||
			   struxType == "header-even" ||
			   struxType == "footer-even" ||
			   struxType == "header-first" ||
			   struxType == "footer-first" ||
			   struxType == "header-last" ||
			   struxType == "footer-last")
			{
				pts = PTX_SectionHdrFtr;
			}
	    }
	}
	if (!_createStrux(pts,indexAP,&pfs))
		return false;

	return true;
}


bool pt_PieceTable::_makeObject(PTObjectType pto, const PP_PropertyVector & attributes, pf_Frag_Object * &pfo)
{
	// create a new object fragment at the current end of the document.
	// this function can only be called while loading the document.
	UT_return_val_if_fail (m_pts==PTS_Loading, false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(), false);

	// first, store the attributes and properties and get an index to them.

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	if (!_createObject(pto,indexAP,&pfo))
		return false;

	return true;
}

bool pt_PieceTable::_makeFmtMark(pf_Frag_FmtMark * &pff)
{
	// this function can only be called while loading the document.
	UT_return_val_if_fail (m_pts==PTS_Loading,false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(),false);

	pff = new pf_Frag_FmtMark(this,loading.m_indexCurrentInlineAP);
	if (!pff)
		return false;

	return true;
}


bool pt_PieceTable::_makeFmtMark(pf_Frag_FmtMark * &pff, const PP_PropertyVector & attributes)
{
	// this function can only be called while loading the document.
	UT_return_val_if_fail (m_pts==PTS_Loading,false);

	// Only a strux can be appended to an empty document
	UT_return_val_if_fail (NULL != m_fragments.getFirst(), false);
	if(attributes.empty())
		{
			return _makeFmtMark(pff);
		}
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	pff = new pf_Frag_FmtMark(this,indexAP);
	if (!pff)
		return false;

	return true;
}
