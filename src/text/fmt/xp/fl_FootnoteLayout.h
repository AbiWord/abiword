/* AbiWord
 * Copyright (C) 2002 Patrick Lam <plam@mit.edu>
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

#ifndef FOOTNOTELAYOUT_H
#define FOOTNOTELAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"

// We have one fl_FootnoteLayout for each footnote.  They all
// get physically placed at the bottom of the fp_Page in their own
// little container.  

// The fl_FootnoteLayout lives after each block.

// Need to do cursor navigation between blocks
class ABI_EXPORT fl_FootnoteLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FootnoteContainer;

public:
	fl_FootnoteLayout(FL_DocLayout* pLayout, fl_DocSectionLayout * pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_FootnoteLayout();

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertEndFootnote(fl_ContainerLayout*,
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
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	fl_DocSectionLayout*	 getDocSectionLayout(void) const { return m_pDocSL; }

protected:
	virtual void		     _lookupProperties(void);
	virtual void             _purgeLayout(void);
private:
	void                     _createFootnoteContainer(void);
	void                     _insertFootnoteContainer(fp_Container * pNewFC);
	void                     _localCollapse();

	fl_DocSectionLayout*	 m_pDocSL;
	bool                   m_bNeedsFormat;
	bool                   m_bNeedsRebuild;
};

#endif /* FOOTNOTELAYOUT_H */
