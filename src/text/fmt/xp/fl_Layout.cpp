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
#include "pf_Frag_Strux.h"

fl_Layout::fl_Layout(PTStruxType type, pf_Frag_Strux* sdh) :
	m_type(type),
	m_apIndex(0),
	m_pAutoNum(NULL),
	m_pDoc(NULL), // set by child
	m_sdh(sdh),
	m_endSdh(NULL)
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

void fl_Layout::setEndStruxDocHandle(pf_Frag_Strux* pfs)
{
    UT_ASSERT(!m_endSdh);
    m_endSdh = pfs;
}

void fl_Layout::setType(PTStruxType type)
{
	m_type = type;
}

void fl_Layout::setAttrPropIndex(PT_AttrPropIndex apIndex)
{
	m_apIndex = apIndex;
}

/*!
    ppAP [out] -- the requested AP
    
    pRevisions [in/out] -- revisions attribute; can be set to NULL, in
                       which case an instance will be created; the
                       caller is responsible for deleting

    bShowRevisions -- indicates if in the current view revisions are
                      to be shown

    iRevisionId -- the id of revision which is shown in the current view
    
    bHiddenRevision [out] indicates if the element associated with
                          ppAP is hidden or visible

    bDelete [out] -- if set, the caller must delete ppAP when done
                     with it.
    
*/
bool fl_Layout::getAttrProp(const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
							bool bShowRevisions, UT_uint32 iRevisionId, bool &bHiddenRevision) const
{
	UT_return_val_if_fail(m_pDoc, false);
	return m_pDoc->getAttrProp(m_apIndex, ppAP, pRevisions, bShowRevisions, iRevisionId, bHiddenRevision);
}

/*!
    if pRevisions is not needed, set the pointer to NULL(this speeds up things)
*/
bool fl_Layout::getSpanAttrProp(UT_uint32 offset, bool bLeftSide, const PP_AttrProp ** ppAP,
								PP_RevisionAttr ** pRevisions,
								bool bShowRevisions, UT_uint32 iRevisionId,
								bool &bHiddenRevision) const
{
	UT_return_val_if_fail(m_pDoc, false);
	return m_pDoc->getSpanAttrProp(m_sdh,offset, bLeftSide, ppAP, pRevisions,
								   bShowRevisions, iRevisionId, bHiddenRevision);
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
