/* AbiWord
 * Copyright (C) 2003 Martin Sevior.
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

#ifndef FRAMELAYOUT_H
#define FRAMELAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"
#include "pp_PropertyMap.h"

enum FL_FrameFormatMode
{
	FL_FRAME_POSITIONED_TO_BLOCK_ABOVE_TEXT,
	FL_FRAME_POSITIONED_TO_BLOCK_BELOW_TEXT,
	FL_FRAME_POSITIONED_TO_COLUMN_ABOVE_TEXT,
	FL_FRAME_POSITIONED_TO_COLUMN_BELOW_TEXT,
	FL_FRAME_POSITIONED_TO_PAGE_ABOVE_TEXT,
	FL_FRAME_POSITIONED_TO_PAGE_BELOW_TEXT,
	FL_FRAME_POSITIONED_INLINE,
	FL_FRAME_TEXT_WRAPPED_TO_RIGHT,
	FL_FRAME_TEXT_WRAPPED_TO_LEFT,
	FL_FRAME_CENTERED_IN_TEXT
};

// We have one fl_FrameLayout for each Frame.  They all
// get physically placed on a page. 


// The fl_FrameLayout is placed before the Block it is closest to on the page.
// It holds Blocks of it's own like cells and foornotes.

class ABI_EXPORT fl_FrameLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FrameContainer;

public:
	fl_FrameLayout(FL_DocLayout* pLayout,
				   fl_DocSectionLayout * pDocSL, 
				   PL_StruxDocHandle sdh, 
				   PT_AttrPropIndex ap, 
				   fl_ContainerLayout * pMyContainerLayout,
				   SectionType iSecType,
				   fl_ContainerType myType,
				   PTStruxType myStruxType);
	virtual ~fl_FrameLayout();

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    doclistener_deleteEndFrame(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertEndFrame(fl_ContainerLayout*,
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
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
private:
	void		             _lookupProperties(void);
	void                     _purgeLayout(void);
	UT_uint32                getFramePID(void) const
		{return m_iFramePID;}
	void                     _createFrameContainer(void);
	void                     _insertFrameContainer(fp_Container * pNewFC);
	void                     _localCollapse();
	UT_uint32                m_iFramePID;
	bool                     m_bNeedsRebuild;
	bool                     m_bNeedsFormat;
	bool                     m_bIsOnPage;
	fl_DocSectionLayout*	 m_pDocSL;
	bool                     m_bHasEndFrame;

// Frame-background properties
	PP_PropertyMap::Background	m_background;

// Frame-border properties
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;
};

#endif /* FRAMELAYOUT_H */
