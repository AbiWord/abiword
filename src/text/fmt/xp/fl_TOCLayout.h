/* AbiWord
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#ifndef TOCLAYOUT_H
#define TOCLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"

// We have one fl_TOCLayout for each Table of Contents.

class ABI_EXPORT fl_TOCLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FootnoteContainer;

public:
	fl_TOCLayout(FL_DocLayout* pLayout,
				   fl_DocSectionLayout * pDocSL, 
				   PL_StruxDocHandle sdh, 
				   PT_AttrPropIndex ap, 
				   fl_ContainerLayout * pMyContainerLayout,
				   SectionType iSecType,
				   fl_ContainerType myType,
				   PTStruxType myStruxType);
	virtual ~fl_TOCLayout();

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    doclistener_deleteEndTOC(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertTOC(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));

	virtual void		     format(void);
	virtual void		     updateLayout(void);
	virtual void             collapse(void);
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL) =0;
	fl_DocSectionLayout*	 getDocSectionLayout(void) const { return m_pDocSL; }
	bool                     isEndTOCIn(void) const
		{return m_bHasEndTOC;}
	void                     setTOCEndIn(void)
		{ m_bHasEndFootnote = true;}
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
private:
	virtual void		     _lookupProperties(void) = 0;
	virtual void             _purgeLayout(void);
	virtual void		     _lookupProperties(void);
	void                     _createFootnoteContainer(void);
	void                     _insertFootnoteContainer(fp_Container * pNewFC);
	void                     _localCollapse();
	bool                     m_bNeedsRebuild;
	bool                     m_bNeedsFormat;
	bool                     m_bIsOnPage;
	UT_uint32                m_iTOCPID;
	fl_DocSectionLayout*	 m_pDocSL;
	bool                     m_bHasEndTOC;
};
#endif /* TOCLAYOUT_H */
