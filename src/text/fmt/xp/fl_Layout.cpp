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
#include "pt_Types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "fl_Layout.h"
#include "pd_Document.h"
#include "fd_Field.h"
#include "po_Bookmark.h"
#include "pp_AttrProp.h"
#include "pf_Frag.h"
fl_Layout::fl_Layout(PTStruxType type, PL_StruxDocHandle sdh) :
	m_type(type),
	m_apIndex(0),
	m_pAutoNum(NULL),
	m_pDoc(NULL), // set by child
	m_sdh(sdh)
{
	xxx_UT_DEBUGMSG(("Layout Strux type = %d \n",type));
	const pf_Frag * pf = static_cast<const pf_Frag *>(sdh);
	if(pf)
	{
		UT_ASSERT(pf->getType() ==  pf_Frag::PFT_Strux);
	}
}

fl_Layout::~fl_Layout()
{
}

PL_StruxDocHandle fl_Layout::getStruxDocHandle(void) const
{
	return m_sdh;
}


PTStruxType	fl_Layout::getType(void) const
{
	return m_type;
}


void fl_Layout::setType(PTStruxType type)
{
	m_type = type;
}

PT_AttrPropIndex fl_Layout::getAttrPropIndex(void) const
{
	return m_apIndex;
}

void fl_Layout::setAttrPropIndex(PT_AttrPropIndex apIndex)
{
	m_apIndex = apIndex;
}

/*!
Gets the attributes and properties belong to this layout class.
\param ppAP The requested AP [out]
\param pRevisions Revisions attribute; can be set to NULL, in which case an 
       instance will be created; the caller is responsible for deleting it [in]
\param bShowRevisions Indicates if in the current view revisions are to be shown
\param iRevisionId The ID of revision which is shown in the current view
\param bHiddenRevision Indicates if the element associated with ppAP is 
       hidden or visible [out]
\return true if successful, false otherwise    
*/
bool fl_Layout::getAttrProp(const PP_AttrProp ** ppAP, PP_RevisionAttr *& pRevisions,
							bool bShowRevisions, UT_uint32 iRevisionId, bool &bHiddenRevision) const
{
	bHiddenRevision = false;

	const PP_AttrProp * pAP = NULL;

	UT_return_val_if_fail(m_pDoc, false);
	
	if(!m_pDoc->getAttrProp(m_apIndex,&pAP))
		return false;

	if(   pAP->getRevisedIndex() != 0xffffffff
	   && pAP->getRevisionState().isEqual(iRevisionId, bShowRevisions, m_pDoc->isMarkRevisions()))
	{
		// the revision has a valid index to an inflated AP, so we use it
		bHiddenRevision = pAP->getRevisionHidden();
		PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

		m_pDoc->getAttrProp(revAPI, ppAP);
		return true;
	}
	
	const PP_AttrProp * pNewAP = m_pDoc->explodeRevisions(pRevisions, pAP, bShowRevisions, iRevisionId, bHiddenRevision);

	if(pNewAP)
	{
		*ppAP = pNewAP;
	}
	else
	{
		*ppAP = pAP;
	}
	
	return true;
}

bool fl_Layout::getSpanAttrProp(UT_uint32 offset, bool bLeftSide, const PP_AttrProp ** ppAP,
								PP_RevisionAttr *& pRevisions,
								bool bShowRevisions, UT_uint32 iRevisionId,
								bool &bHiddenRevision) const
{
	bHiddenRevision = false;

	const PP_AttrProp * pAP = NULL;

	UT_return_val_if_fail(m_pDoc, false);
	
	if(!m_pDoc->getSpanAttrProp(m_sdh,offset,bLeftSide,&pAP))
		return false;

	if(   pAP->getRevisedIndex() != 0xffffffff
	   && pAP->getRevisionState().isEqual(iRevisionId, bShowRevisions, m_pDoc->isMarkRevisions()))
	{
		// the revision has a valid index to an inflated AP, so we use it
		bHiddenRevision = pAP->getRevisionHidden();
		PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

		m_pDoc->getAttrProp(revAPI, ppAP);
		return true;
	}
	
	const PP_AttrProp * pNewAP = m_pDoc->explodeRevisions(pRevisions, pAP, bShowRevisions, iRevisionId, bHiddenRevision);

	if(pNewAP)
	{
		*ppAP = pNewAP;
	}
	else
	{
		*ppAP = pAP;
	}
	
	return true;
}

bool fl_Layout::getField(UT_uint32 offset, fd_Field * & pField)
{
    return m_pDoc->getField(m_sdh,offset,pField);
}

po_Bookmark * fl_Layout::getBookmark(UT_uint32 offset)
{
    return m_pDoc->getBookmark(m_sdh,offset);
}

void fl_Layout::setAutoNum(fl_AutoNum * pAutoNum)
{
	m_pAutoNum = pAutoNum;
}
