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



#ifndef FL_LAYOUT_H
#define FL_LAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"

class PP_AttrProp;
class PP_RevisionAttr;
class PD_Document;
class fd_Field;
class po_Bookmark;
class fl_AutoNum;

/*!
	fl_Layout is the base class for all layout objects which correspond to 
	logical elements of the PD_Document.  

	We use an enum to remember type, rather than use any of the
	run-time stuff.
*/

class ABI_EXPORT fl_Layout
{
public:
	fl_Layout(PTStruxType type, PL_StruxDocHandle sdh);
	virtual ~fl_Layout();

	PL_StruxDocHandle               getStruxDocHandle(void) const;
	PTStruxType			getType(void) const;
	void                setType(PTStruxType type);
	PT_AttrPropIndex 	getAttrPropIndex(void) const;
	void				setAttrPropIndex(PT_AttrPropIndex apIndex);

	bool				getAttrProp(const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
									bool bShowRevisions, UT_uint32 iRevisionId,
									bool &bHiddenRevision) const;
	
	bool				getSpanAttrProp(UT_uint32 offset, bool bLeftSide, const PP_AttrProp ** ppAP,
										PP_RevisionAttr ** pRevisions,
										bool bShowRevisions, UT_uint32 iRevisionId,
										bool &bHiddenRevision) const;
	
	bool				getField(UT_uint32 offset, fd_Field * &pField);
	po_Bookmark *		getBookmark(UT_uint32 offset);
	virtual	void		listUpdate(void) { return; }
	inline fl_AutoNum *	getAutoNum(void) const { return m_pAutoNum; }
	void    			setAutoNum(fl_AutoNum * pAutoNum);
	
	PD_Document *	    getDocument(void) const { return m_pDoc; };
	
protected:
	PTStruxType				m_type;
	PT_AttrPropIndex		m_apIndex;
	fl_AutoNum * 			m_pAutoNum;
	
	PD_Document *			m_pDoc;		// set by child
private:
	PL_StruxDocHandle		m_sdh;
};

#endif /* FL_LAYOUT_H */
