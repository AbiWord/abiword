/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#include "rl_Layout.h"
#include "rl_DocLayout.h"

rl_Layout::rl_Layout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout) :
	fl_Layout(type, sdh),
	m_bBreakAfter(false),
	m_pPrev(NULL),
	m_pNext(NULL),
	m_pParent(pParent),
	m_pDocLayout(pDocLayout),
	m_pObject(NULL)
{
	m_pDoc = pDocLayout->getDocument();
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
bool rl_Layout::getAttrProp(const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
							bool bShowRevisions, UT_uint32 iRevisionId, bool &bHiddenRevision) const
{
	UT_return_val_if_fail(m_pDoc, false);
	return m_pDoc->getAttrProp(m_apIndex, ppAP, pRevisions, bShowRevisions, iRevisionId, bHiddenRevision);
}
