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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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

class pf_Frag_Strux;

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
	FL_FRAME_WRAPPED_BOTH_SIDES,
	FL_FRAME_WRAPPED_TOPBOT
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
				   pf_Frag_Strux* sdh,
				   PT_AttrPropIndex ap,
				   fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_FrameLayout();

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    doclistener_deleteEndFrame(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertEndFrame(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
   bool                     insertBlockAfter(fl_ContainerLayout* pCL,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	void                     miniFormat(void);
	virtual void		     format(void);
	virtual void		     updateLayout(bool bDoFull);
	virtual void             collapse(void);
	void                     localCollapse();
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
	virtual void             setNeedsReformat(fl_ContainerLayout * pCL, UT_uint32 offset = 0);
	void                     setContainerProperties(void);
	UT_sint32                getBoundingSpace(void) const;
	FL_FrameType             getFrameType(void) const
		{return m_iFrameType;}
	FL_FrameFormatMode       getFramePositionTo(void) const
		{ return m_iFramePositionTo;}
	FL_FrameWrapMode         getFrameWrapMode(void) const
		{ return m_iFrameWrapMode;}
	UT_sint32                getFrameWidth(void) const
		{ return m_iWidth;}
	UT_sint32                getFrameHeight(void) const
		{ return m_iHeight;}
	UT_sint32                getFrameXpos(void) const
		{ return m_iXpos;}
	UT_sint32                getFrameYpos(void) const
		{ return m_iYpos;}
	UT_sint32                getFrameXColpos(void) const
		{ return m_iXColumn;}
	UT_sint32                getFrameYColpos(void) const
		{ return m_iYColumn;}
	UT_sint32                getFrameXPagepos(void) const
		{ return m_iXPage;}
	UT_sint32                getFrameYPagepos(void) const
		{ return m_iYPage;}

	void                setFrameWidth(UT_sint32 iW) { m_iWidth = iW;}
	void                setFrameHeight(UT_sint32 iH) { m_iHeight = iH;}
	void                setFrameXpos(UT_sint32 iX) 	{ m_iXpos = iX;}
	void                setFrameYpos(UT_sint32 iY) { m_iYpos = iY;}
	bool                isEndFrameIn(void) const
		{ return m_bHasEndFrame;}
	bool                isTightWrap(void)
	  { return m_bIsTightWrap;}
	bool                expandHeight(void) const
	{ return m_bExpandHeight;}
	UT_sint32           minHeight(void) const
	{ return m_iMinHeight;}
	fl_ContainerLayout *     getParentContainer(void) const
	        {return m_pParentContainer;}
	void                setParentContainer(fl_ContainerLayout * pCon)
	        {m_pParentContainer = pCon;}
private:
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
	virtual void		     _lookupMarginProperties(const PP_AttrProp* pAP);
	void                     _purgeLayout(void);
	void                     _createFrameContainer(void);
	void                     _insertFrameContainer(fp_Container * pNewFC);
	FL_FrameType             m_iFrameType;
	FL_FrameFormatMode       m_iFramePositionTo;
	bool                     m_bNeedsRebuild;
	bool                     m_bNeedsFormat;
	bool                     m_bIsOnPage;
	bool                     m_bHasEndFrame;

// Frame-background properties
	PP_PropertyMap::Background	m_background;

// Frame-border properties
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;

	UT_sint32               m_iWidth;
	UT_sint32               m_iHeight;
	UT_sint32               m_iXpos;
	UT_sint32               m_iYpos;

	UT_sint32               m_iXpad;
	UT_sint32               m_iYpad;

	UT_sint32               m_iXColumn;
	UT_sint32               m_iYColumn;

	UT_sint32               m_iXPage;
	UT_sint32               m_iYPage;

	UT_sint32               m_iBoundingSpace;
        FL_FrameWrapMode        m_iFrameWrapMode;
	bool                    m_bIsTightWrap;
	UT_sint32               m_iPrefPage;
	UT_sint32               m_iPrefColumn;
	bool                    m_bExpandHeight;
	UT_sint32               m_iMinHeight;
	fl_ContainerLayout *    m_pParentContainer;
};

#endif /* FRAMELAYOUT_H */
