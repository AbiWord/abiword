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

#ifndef SECTIONLAYOUT_H
#define SECTIONLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "pl_Listener.h"

class fp_Page;
class FL_DocLayout;
class fl_BlockLayout;
class fb_LineBreaker;
class fp_HdrFtrContainer;
class fp_Column;
class fp_Container;
class PD_Document;
class PP_AttrProp;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;
class fl_HdrFtrSectionLayout;

#define FL_SECTION_DOC			1
#define FL_SECTION_HDRFTR		2
#define FL_SECTION_SHADOW		3

#define FL_HDRFTR_HEADER		1
#define FL_HDRFTR_FOOTER		2

class fl_SectionLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, UT_uint32 iType);
	virtual ~fl_SectionLayout();

	inline UT_uint32	getType(void) const { return m_iType; }

	virtual UT_Bool		recalculateFields(void);
	
	FL_DocLayout*		getDocLayout(void) const;

	virtual fp_Container*		getNewContainer() = 0;
	virtual fp_Container*		getFirstContainer() = 0;
	virtual fp_Container*		getLastContainer() = 0;

	virtual void		format(void) = 0;
	virtual void		updateLayout(void) = 0;

	inline fl_SectionLayout*	getPrev(void) const { return m_pPrev; }
	inline fl_SectionLayout*	getNext(void) const { return m_pNext; }
	void				setPrev(fl_SectionLayout*);
	void				setNext(fl_SectionLayout*);
	
	fl_BlockLayout *	getFirstBlock(void) const;
	fl_BlockLayout *	getLastBlock(void) const;
	fl_BlockLayout *	appendBlock(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP);
	fl_BlockLayout *	insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP);
	void				addBlock(fl_BlockLayout* pBL);
	void				removeBlock(fl_BlockLayout * pBL);

	virtual UT_Bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc) = 0;

	const char*			getAttribute(const char* pszName) const;
	
	virtual UT_Bool bl_doclistener_populateSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual UT_Bool bl_doclistener_populateObject(fl_BlockLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_insertSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual UT_Bool bl_doclistener_deleteSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual UT_Bool bl_doclistener_changeSpan(fl_BlockLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual UT_Bool bl_doclistener_deleteStrux(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual UT_Bool bl_doclistener_changeStrux(fl_BlockLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual UT_Bool bl_doclistener_insertBlock(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));
	virtual UT_Bool bl_doclistener_insertSection(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
									  PL_StruxDocHandle sdh,
									  PL_ListenerId lid,
									  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															  PL_ListenerId lid,
															  PL_StruxFmtHandle sfhNew));
	virtual UT_Bool bl_doclistener_insertObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_deleteObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_changeObject(fl_BlockLayout*, const PX_ChangeRecord_ObjectChange * pcroc);
	
protected:
	virtual void		_lookupProperties(void) = 0;
	
	void				_purgeLayout();
	fb_LineBreaker *	_getLineBreaker(void);

	UT_uint32			m_iType;
	
	fl_SectionLayout*	m_pPrev;
	fl_SectionLayout*	m_pNext;
	
	FL_DocLayout*		m_pLayout;
	fb_LineBreaker*		m_pLB;

	fl_BlockLayout*		m_pFirstBlock;
	fl_BlockLayout*		m_pLastBlock;
};

class fl_DocSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_DocSectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_DocSectionLayout();

	fl_DocSectionLayout* getNextDocSection(void) const;
	fl_DocSectionLayout* getPrevDocSection(void) const;
	
	virtual void		format(void);
	virtual void		updateLayout(void);
	UT_sint32			breakSection(void);
	
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();

	inline UT_sint32			getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32			getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32			getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32			getBottomMargin(void) const { return m_iBottomMargin; }
	inline UT_sint32			getSpaceAfter(void) const { return m_iSpaceAfter; }
	
	UT_uint32			getNumColumns(void) const;
	UT_uint32			getColumnGap(void) const;

	void				deleteEmptyColumns(void);
	virtual UT_Bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	UT_Bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);

	void				setHdrFtr(UT_uint32 iType, fl_HdrFtrSectionLayout* pHFSL);

	void				addOwnedPage(fp_Page*);
	void				deleteOwnedPage(fp_Page*);

protected:
	virtual void		_lookupProperties(void);

	/*
	  TODO support special case header/footer for first page of section
	*/
	fl_HdrFtrSectionLayout*		m_pHeaderSL;
	fl_HdrFtrSectionLayout*		m_pFooterSL;
	
	UT_uint32			m_iNumColumns;
	UT_uint32			m_iColumnGap;

	UT_sint32			m_iSpaceAfter;
	UT_sint32			m_iLeftMargin;
	UT_sint32			m_iRightMargin;
	UT_sint32			m_iTopMargin;
	UT_sint32			m_iBottomMargin;

	UT_Bool				m_bForceNewPage;

	fp_Column*			m_pFirstColumn;
	fp_Column*			m_pLastColumn;

	UT_Vector			m_vecOwnedPages;
};

class fl_HdrFtrSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrSectionLayout(UT_uint32 iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrSectionLayout();
	
	inline fl_DocSectionLayout*	getDocSectionLayout(void) const { return m_pDocSL; }
	inline UT_uint32			getHFType(void) const { return m_iHFType; }

	virtual UT_Bool		recalculateFields(void);
	
	virtual void				format(void);
	virtual void				updateLayout(void);
	
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();

	virtual UT_Bool 			doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);

	void						addPage(fp_Page*);
	void						deletePage(fp_Page*);
	
	virtual UT_Bool bl_doclistener_populateSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual UT_Bool bl_doclistener_populateObject(fl_BlockLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_insertSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual UT_Bool bl_doclistener_deleteSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual UT_Bool bl_doclistener_changeSpan(fl_BlockLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual UT_Bool bl_doclistener_deleteStrux(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual UT_Bool bl_doclistener_changeStrux(fl_BlockLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual UT_Bool bl_doclistener_insertBlock(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));
	virtual UT_Bool bl_doclistener_insertSection(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
									  PL_StruxDocHandle sdh,
									  PL_ListenerId lid,
									  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															  PL_ListenerId lid,
															  PL_StruxFmtHandle sfhNew));
	virtual UT_Bool bl_doclistener_insertObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_deleteObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual UT_Bool bl_doclistener_changeObject(fl_BlockLayout*, const PX_ChangeRecord_ObjectChange * pcroc);
	
protected:
	UT_sint32					_findShadow(fp_Page* pPage);
	virtual void				_lookupProperties(void);
	
	fl_DocSectionLayout*		m_pDocSL;
	UT_uint32					m_iHFType;

	UT_Vector					m_vecPages;
};

class fl_HdrFtrShadow : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrShadow();
	
	inline fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const { return m_pHdrFtrSL; }

	virtual void				format(void);
	virtual void				updateLayout(void);
	
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();

	virtual UT_Bool 			doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);

protected:
	virtual void				_lookupProperties(void);
	void						_createContainer(void);
	
	fl_HdrFtrSectionLayout*		m_pHdrFtrSL;
	fp_HdrFtrContainer*			m_pContainer;
	fp_Page*					m_pPage;
};

class fl_ShadowListener : public PL_Listener
{
public:
	fl_ShadowListener(fl_HdrFtrSectionLayout* pHFSL, fl_HdrFtrShadow* pShadow);
	virtual ~fl_ShadowListener();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual UT_Bool		signal(UT_uint32 iSignal);

protected:
	PD_Document*				m_pDoc;
	fl_HdrFtrSectionLayout*		m_pHFSL;
	fl_HdrFtrShadow* 			m_pShadow;
	UT_Bool						m_bListening;
	fl_BlockLayout*				m_pCurrentBL;
};

#endif /* SECTIONLAYOUT_H */
