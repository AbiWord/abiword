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
	FL_FRAME_POSITIONED_TO_BLOCK,
	FL_FRAME_POSITIONED_TO_COLUMN,
	FL_FRAME_POSITIONED_TO_PAGE,
	FL_FRAME_POSITIONED_INLINE
};

enum FL_FrameWrapMode
{
	FL_FRAME_ABOVE_TEXT,
	FL_FRAME_BELOW_TEXT,
	FL_FRAME_WRAPPED_TO_RIGHT,
	FL_FRAME_WRAPPED_TO_LEFT,
	FL_FRAME_WRAPPED_BOTH_SIDES
};

enum FL_FrameType
{
	FL_FRAME_TEXTBOX_TYPE,
	FL_FRAME_WRAPPER_IMAGE,
	FL_FRAME_WRAPPER_TABLE,
	FL_FRAME_WRAPPER_EMBED
};

// We have one fl_FrameLayout for each Frame.  They all
// get physically placed on a page. 


// The fl_FrameLayout is placed before the Block it is closest to on the page.
// It holds Blocks of it's own like cells and footnotes.

class ABI_EXPORT fl_FrameLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FrameContainer;

public:
	fl_FrameLayout(FL_DocLayout* pLayout,
				   fl_DocSectionLayout * pDocSL, 
				   PL_StruxDocHandle sdh, 
				   PT_AttrPropIndex ap, 
				   fl_ContainerLayout * pMyContainerLayout);
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
   bool                     insertBlockAfter(fl_ContainerLayout* pCL,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));

	void                     miniFormat(void);
	virtual void		     format(void);
	virtual void		     updateLayout(void);
	virtual void             collapse(void);
	void                     localCollapse();
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	fl_DocSectionLayout*	 getDocSectionLayout(void) const { return m_pDocSL; }
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
	void                     setContainerProperties(void);
	double                   getBoundingSpace(void) const;
	FL_FrameType             getFrameType(void) const 
		{return m_iFrameType;}
	FL_FrameFormatMode       getFramePositionTo(void) const
		{ return m_iFramePositionTo;}
	FL_FrameWrapMode         getFrameWrapMode(void) const
		{ return m_iFrameWrapMode;}
	double                   getFrameWidth(void) const
		{ return m_iWidth;}
	double                   getFrameHeight(void) const
		{ return m_iHeight;}
	double                   getFrameXpos(void) const
		{ return m_iXpos;}
	double                   getFrameYpos(void) const
		{ return m_iYpos;}
	double                   getFrameXColpos(void) const
		{ return m_iXColumn;}
	double                   getFrameYColpos(void) const
		{ return m_iYColumn;}
	double                   getFrameXPagepos(void) const
		{ return m_iXPage;}
	double                  getFrameYPagepos(void) const
		{ return m_iYPage;}

	void                setFrameWidth(double iW) { m_iWidth = iW;}
	void                setFrameHeight(double iH) { m_iHeight = iH;}
	void                setFrameXpos(double iX) 	{ m_iXpos = iX;}
	void                setFrameYpos(double iY) { m_iYpos = iY;}
	bool                isEndFrameIn(void) const
		{ return m_bHasEndFrame;}
	bool                isTightWrap(void)
	  { return m_bIsTightWrap;}
private:
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
	void                     _purgeLayout(void);
	void                     _createFrameContainer(void);
	void                     _insertFrameContainer(fp_Container * pNewFC);
	FL_FrameType             m_iFrameType;
	FL_FrameFormatMode       m_iFramePositionTo;
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

	double               m_iWidth;
	double               m_iHeight;
	double               m_iXpos;
	double               m_iYpos;

	double               m_iXpad;
	double               m_iYpad;

	double               m_iXColumn;
	double               m_iYColumn;

	double               m_iXPage;
	double               m_iYPage;

	double                  m_iBoundingSpace;
        FL_FrameWrapMode        m_iFrameWrapMode;
	bool                    m_bIsTightWrap;
};

#endif /* FRAMELAYOUT_H */
