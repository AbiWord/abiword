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

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"

class fp_Page;
class FL_DocLayout;
class fl_BlockLayout;
class fb_LineBreaker;
class fp_HdrFtrContainer;
class fp_Column;
class fp_Run;
class fp_Line;
class fp_Container;
class PD_Document;
class PP_AttrProp;
class fl_HdrFtrShadow;
class PX_ChangeRecord_FmtMark;
class PX_ChangeRecord_FmtMarkChange;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;
class fl_HdrFtrSectionLayout;

typedef enum 
{
	FL_SECTION_DOC,
    FL_SECTION_HDRFTR,
    FL_SECTION_SHADOW
} SectionType;


typedef enum 
{       
	FL_HDRFTR_HEADER,
	FL_HDRFTR_FOOTER
} HdrFtrType;

class fl_SectionLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, SectionType iType);
	virtual ~fl_SectionLayout();

	SectionType     	getType(void) const { return m_iType; }

	virtual bool		recalculateFields(void);
	
	FL_DocLayout*		getDocLayout(void) const;

	virtual fp_Container*		getNewContainer() = 0;
	virtual fp_Container*		getFirstContainer() = 0;
	virtual fp_Container*		getLastContainer() = 0;

	fl_HdrFtrSectionLayout *    getHdrFtrSectionLayout(void) {return m_pHdrFtrSL;}


	virtual void		format(void) = 0;
	virtual void		updateLayout(void) = 0;

	virtual void		redrawUpdate(void) = 0;

	inline fl_SectionLayout*	getPrev(void) const { return m_pPrev; }
	inline fl_SectionLayout*	getNext(void) const { return m_pNext; }
	void				setPrev(fl_SectionLayout*);
	void				setNext(fl_SectionLayout*);
	
	fl_BlockLayout *	getFirstBlock(void) const;
	fl_BlockLayout *	getLastBlock(void) const;
	void                setLastBlock(fl_BlockLayout * pLast) { m_pLastBlock = pLast;}
	fl_BlockLayout *	appendBlock(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP);
	fl_BlockLayout *	insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP);
	void				addBlock(fl_BlockLayout* pBL);
	void				removeBlock(fl_BlockLayout * pBL);

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc) = 0;

	const char*			getAttribute(const char* pszName) const;
	
	virtual bool bl_doclistener_populateSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual bool bl_doclistener_populateObject(fl_BlockLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_insertSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_deleteSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_changeSpan(fl_BlockLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual bool bl_doclistener_deleteStrux(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_changeStrux(fl_BlockLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool bl_doclistener_insertBlock(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertSection(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertHdrFtrSection(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_BlockLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif
	
protected:
	virtual void		_lookupProperties(void) = 0;
	
	void				_purgeLayout();
	fb_LineBreaker *	_getLineBreaker(void);

	SectionType			m_iType;
	
	fl_SectionLayout*	m_pPrev;
	fl_SectionLayout*	m_pNext;
	
	FL_DocLayout*		m_pLayout;
	fb_LineBreaker*		m_pLB;

	fl_BlockLayout*		m_pFirstBlock;
	fl_BlockLayout*		m_pLastBlock;
	fl_HdrFtrSectionLayout * m_pHdrFtrSL;
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

	virtual void		redrawUpdate(void);
	
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();

	inline UT_sint32			getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32			getLeftMarginInLayoutUnits(void) const { return m_iLeftMarginLayoutUnits; }
	inline UT_sint32			getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32			getRightMarginInLayoutUnits(void) const { return m_iRightMarginLayoutUnits; }
	inline UT_sint32			getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32			getTopMarginInLayoutUnits(void) const { return m_iTopMarginLayoutUnits; }
	inline UT_sint32			getBottomMargin(void) const { return m_iBottomMargin; }
	inline UT_sint32			getBottomMarginInLayoutUnits(void) const { return m_iBottomMarginLayoutUnits; }
	inline UT_sint32			getFooterMargin(void) const { return m_iFooterMargin; }
	inline UT_sint32			getFooterMarginInLayoutUnits(void) const { return m_iFooterMarginLayoutUnits; }
	inline UT_sint32			getHeaderMargin(void) const { return m_iHeaderMargin; }
	inline UT_sint32			getHeaderMarginInLayoutUnits(void) const { return m_iHeaderMarginLayoutUnits; }
	inline UT_sint32			getSpaceAfter(void) const { return m_iSpaceAfter; }
	inline UT_sint32			getSpaceAfterInLayoutUnits(void) const { return m_iSpaceAfterLayoutUnits; }
	
	UT_uint32			getNumColumns(void) const;
	UT_uint32			getColumnGap(void) const;
	UT_uint32			getColumnGapInLayoutUnits(void) const;
	bool				getColumnLineBetween(void) const {return m_bColumnLineBetween;}

	void				deleteEmptyColumns(void);
	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);

	void				setHdrFtr(HdrFtrType iType, fl_HdrFtrSectionLayout* pHFSL);

	fl_HdrFtrSectionLayout*         getHeader(void);
	fl_HdrFtrSectionLayout*         getFooter(void);

	void				addOwnedPage(fp_Page*);
	void                            prependOwnedHeaderPage(fp_Page * p_Page);
	void                            prependOwnedFooterPage(fp_Page * p_Page);
	void				deleteOwnedPage(fp_Page*);

	void				checkAndAdjustColumnGap(UT_sint32 iLayoutWidth);

protected:
	virtual void		_lookupProperties(void);

	/*
	  TODO support special case header/footer for first page of section
	*/
	fl_HdrFtrSectionLayout*		m_pHeaderSL;
	fl_HdrFtrSectionLayout*		m_pFooterSL;
	
	UT_uint32			m_iNumColumns;
	UT_uint32			m_iColumnGap;
	UT_uint32			m_iColumnGapLayoutUnits;
	bool				m_bColumnLineBetween;

	UT_sint32			m_iSpaceAfter;
	UT_sint32			m_iSpaceAfterLayoutUnits;
	UT_sint32			m_iLeftMargin;
	UT_sint32			m_iLeftMarginLayoutUnits;
	double				m_dLeftMarginUserUnits;
	UT_sint32			m_iRightMargin;
	UT_sint32			m_iRightMarginLayoutUnits;
	double				m_dRightMarginUserUnits;
	UT_sint32			m_iTopMargin;
	UT_sint32			m_iTopMarginLayoutUnits;
	double				m_dTopMarginUserUnits;
	UT_sint32			m_iBottomMargin;
	UT_sint32			m_iBottomMarginLayoutUnits;
	double				m_dBottomMarginUserUnits;
	UT_sint32			m_iFooterMargin;
	UT_sint32			m_iFooterMarginLayoutUnits;
	double				m_dFooterMarginUserUnits;
	UT_sint32			m_iHeaderMargin;
	UT_sint32			m_iHeaderMarginLayoutUnits;
	double				m_dHeaderMarginUserUnits;

	bool				m_bForceNewPage;

	fp_Column*			m_pFirstColumn;
	fp_Column*			m_pLastColumn;
	fp_Page *                       m_pFirstOwnedPage;

};

class fl_HdrFtrSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrSectionLayout();
	
	inline fl_DocSectionLayout*	getDocSectionLayout(void) const { return m_pDocSL; }
	HdrFtrType      			getHFType(void) const { return m_iHFType; }
	void                        setDocSectionLayout(fl_DocSectionLayout * pDSL) { m_pDocSL = pDSL;}
	void                        setHdrFtr(HdrFtrType iHFType) { 	m_iHFType = iHFType;}
	virtual bool		recalculateFields(void);
	bool                        doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	void                        localFormat(void);
	void                        localCollapse(void);
	virtual void				format(void);
	virtual void				updateLayout(void);
	fl_BlockLayout *            findMatchingBlock( fl_BlockLayout * pBL);
	virtual void				redrawUpdate(void);
	
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();
	fl_HdrFtrShadow *               getFirstShadow(void);
	fl_HdrFtrShadow *               findShadow( fp_Page * pPage);
	virtual bool 			doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	void                                    changeStrux( fl_DocSectionLayout * pSL);
	void						addPage(fp_Page*);
	void						deletePage(fp_Page*);
	void                            clearScreen(void);
	void                            collapse(void);

	virtual bool bl_doclistener_populateSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual bool bl_doclistener_populateObject(fl_BlockLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_insertSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_deleteSpan(fl_BlockLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_changeSpan(fl_BlockLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual bool bl_doclistener_deleteStrux(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_changeStrux(fl_BlockLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool bl_doclistener_insertBlock(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertSection(fl_BlockLayout*, const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_BlockLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_BlockLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_BlockLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);
	
protected:
	UT_sint32					_findShadow(fp_Page * pPage);
	virtual void				_lookupProperties(void);
	
	fl_DocSectionLayout*		m_pDocSL;
	HdrFtrType					m_iHFType;
	fp_Container *              m_pVirContainer;
	UT_Vector					m_vecPages;
};

class fl_HdrFtrShadow : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrShadow();
	
	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const { return m_pHdrFtrSL; }
	fl_BlockLayout *			findMatchingBlock(fl_BlockLayout * pBL);
	fl_BlockLayout *			findBlockAtPosition(PT_DocPosition pos);
	virtual void				format(void);
	virtual void				updateLayout(void);
	void						clearScreen(void);
	virtual void				redrawUpdate(void);
	fp_Page *                       getPage(void) { return m_pPage;}
	virtual fp_Container*		getNewContainer();
	virtual fp_Container*		getFirstContainer();
	virtual fp_Container*		getLastContainer();

	virtual bool				doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);

protected:
	virtual void				_lookupProperties(void);
	void						_createContainer(void);
	
	fp_HdrFtrContainer*			m_pContainer;
	fp_Page*					m_pPage;
};

class fl_ShadowListener : public PL_Listener
{
public:
	fl_ShadowListener(fl_HdrFtrSectionLayout* pHFSL, fl_HdrFtrShadow* pShadow);
	virtual ~fl_ShadowListener();

	virtual bool				populate(PL_StruxFmtHandle sfh,
										 const PX_ChangeRecord * pcr);

	virtual bool				populateStrux(PL_StruxDocHandle sdh,
											  const PX_ChangeRecord * pcr,
											  PL_StruxFmtHandle * psfh);

	virtual bool				change(PL_StruxFmtHandle sfh,
									   const PX_ChangeRecord * pcr);

	virtual bool				insertStrux(PL_StruxFmtHandle sfh,
											const PX_ChangeRecord * pcr,
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));

	virtual bool				signal(UT_uint32 iSignal);

protected:
	PD_Document*				m_pDoc;
	fl_HdrFtrShadow* 			m_pShadow;
	bool						m_bListening;
	fl_BlockLayout*				m_pCurrentBL;
	fl_HdrFtrSectionLayout *    m_pHFSL;
};

#endif /* SECTIONLAYOUT_H */
