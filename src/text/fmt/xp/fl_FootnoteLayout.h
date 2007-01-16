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
class ABI_EXPORT fl_EmbedLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FootnoteContainer;

public:
	fl_EmbedLayout(FL_DocLayout* pLayout,
				   fl_DocSectionLayout * pDocSL, 
				   PL_StruxDocHandle sdh, 
				   PT_AttrPropIndex ap, 
				   fl_ContainerLayout * pMyContainerLayout,
				   SectionType iSecType,
				   fl_ContainerType myType,
				   PTStruxType myStruxType);
	virtual ~fl_EmbedLayout();
	virtual void		   setNeedsReformat(fl_ContainerLayout * pCL, UT_uint32 offset = 0);
	virtual void		updateLayout(bool bDoAll);

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    doclistener_deleteEndEmbed(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertEndEmbed(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));

	virtual void		     format(void) = 0;
	virtual void		     updateLayout(void);
	virtual void             collapse(void) = 0;
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL) =0;
	fl_DocSectionLayout*	 getDocSectionLayout(void) const { return m_pDocSL; }
	void                     setDocSectionLayout(fl_DocSectionLayout * pDSL) 
	  { m_pDocSL = pDSL;}
	bool                     isEndFootnoteIn(void) const
		{return m_bHasEndFootnote;}
	void                     setFootnoteEndIn(void)
		{ m_bHasEndFootnote = true;}
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
	UT_sint32                getOldSize(void) const
		{ return m_iOldSize;}
	void                     setOldSize(UT_sint32 i)
		{ m_iOldSize = i;}
protected:
	virtual void             _purgeLayout(void);
	bool                     m_bNeedsRebuild;
	bool                     m_bNeedsFormat;
	bool                     m_bIsOnPage;
private:

	fl_DocSectionLayout*	 m_pDocSL;
	bool                     m_bHasEndFootnote;
	UT_sint32                m_iOldSize;
};

class ABI_EXPORT fl_FootnoteLayout : public fl_EmbedLayout
{
	friend class fl_DocListener;
	friend class fp_FootnoteContainer;

public:
	fl_FootnoteLayout(FL_DocLayout* pLayout, 
					  fl_DocSectionLayout * pDocSL, 
					  PL_StruxDocHandle sdh, 
					  PT_AttrPropIndex ap, 
					  fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_FootnoteLayout();

	virtual void		     format(void);
	virtual void             collapse(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	UT_uint32                getFootnotePID(void) const
		{return m_iFootnotePID;}
protected:
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
private:
	void                     _createFootnoteContainer(void);
	void                     _insertFootnoteContainer(fp_Container * pNewFC);
	void                     _localCollapse();
	UT_uint32                m_iFootnotePID;
};


class ABI_EXPORT fl_EndnoteLayout : public fl_EmbedLayout
{
	friend class fl_DocListener;
	friend class fp_EndnoteContainer;

public:
	fl_EndnoteLayout(FL_DocLayout* pLayout, fl_DocSectionLayout * pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_EndnoteLayout();

	virtual void		     format(void);
	virtual void             collapse(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	UT_uint32                getEndnotePID(void) const
		{return m_iEndnotePID;}
protected:
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
private:
	void                     _createEndnoteContainer(void);
	void                     _insertEndnoteContainer(fp_Container * pNewFC);
	void                     _localCollapse();

	UT_uint32                m_iEndnotePID;
};


#endif /* FOOTNOTELAYOUT_H */
