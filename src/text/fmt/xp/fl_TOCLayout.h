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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef TOCLAYOUT_H
#define TOCLAYOUT_H

#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"

class PD_Style;
class fp_TOCContainer;

class ABI_EXPORT TOCEntry
{
public:
	TOCEntry(fl_BlockLayout * pBlock,
			 UT_sint32 iLevel,
			 UT_UTF8String & sDispStyle,
			 bool bHaveLabel,
			 FootnoteType iFType,
			 UT_UTF8String & sBefore,
			 UT_UTF8String sAfter,
			 bool bInherit,
			 UT_sint32 iStartAt);

	virtual ~ TOCEntry(void);
	fl_BlockLayout *     getBlock(void)
		{ return m_pBlock;}
	PT_DocPosition       getPositionInDoc(void);
	UT_sint32            getLevel(void)
		{ return m_iLevel;}
	UT_UTF8String &      getDispStyle(void)
		{ return m_sDispStyle;}
	bool                 hasLabel(void) const
		{ return m_bHasLabel;}
	bool                 doesInherit(void)
		{ return m_bInherit;}
	void                 setPosInList(UT_sint32 posInList);
	UT_sint32            getPosInList(void)
		{ return m_iPosInList;}
	void                 calculateLabel(TOCEntry * pPrevLevel);
    UT_UTF8String &      getNumLabel(void)
		{ return m_sLabel;}
	UT_UTF8String       getFullLabel(void);
private:
	fl_BlockLayout *  m_pBlock;
	UT_sint32         m_iLevel;
	UT_UTF8String     m_sDispStyle;
	bool              m_bHasLabel;
	FootnoteType      m_iFType;
	UT_UTF8String     m_sBefore;
	UT_UTF8String     m_sAfter;
	bool              m_bInherit;
	UT_sint32         m_iPosInList;
	UT_UTF8String     m_sLabel;
};

// We have one fl_TOCLayout for each Table of Contents.

class ABI_EXPORT fl_TOCLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_FootnoteContainer;

public:
	fl_TOCLayout(FL_DocLayout* pLayout,
				 fl_DocSectionLayout * pDocSL,
				 pf_Frag_Strux* sdh,
				 PT_AttrPropIndex ap,
				 fl_ContainerLayout * pMyContainerLayout);

	virtual ~fl_TOCLayout();

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    doclistener_deleteEndTOC(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertEndTOC(fl_ContainerLayout*,
												const PX_ChangeRecord_Strux * pcrx,
												pf_Frag_Strux* sdh,
												PL_ListenerId lid,
												void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																		PL_ListenerId lid,
																		fl_ContainerLayout* sfhNew));

	virtual void		     format(void);
	virtual void		     updateLayout(bool bDoFull);
	virtual void             collapse(void);
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container* = NULL);
	fl_DocSectionLayout*	 getDocSectionLayout(void) const { return m_pDocSL; }
	bool                     isEndTOCIn(void) const {return m_bHasEndTOC;}
	void                     setTOCEndIn(void);
	TOCEntry *               createNewEntry(fl_BlockLayout * pBL);
	PT_DocPosition           getDocPosition(void);
	UT_uint32                getLength(void);
    fl_BlockLayout  *        findMatchingBlock(fl_BlockLayout * pBlock);
	UT_sint32                isInVector(fl_BlockLayout * pBlock, UT_GenericVector<TOCEntry *>* pVecBlocks);
	UT_uint32                getTOCPID(void) const { return m_iTOCPID;}
	bool                     isTOCEmpty() const {return (m_vecEntries.getItemCount() == 0);}
	bool                     isStyleInTOC(UT_UTF8String & sStyle);
	bool                     isBlockInTOC(fl_BlockLayout * pBlock);
	bool                     addBlock(fl_BlockLayout * pBlock, bool bVerifyRange = true);
	bool                     removeBlock(fl_BlockLayout * pBlock);
	fl_BlockLayout *         getMatchingBlock(fl_BlockLayout * pBlock);
	UT_UTF8String &          getTOCListLabel(fl_BlockLayout * pBlock);
	UT_UTF8String &          getTOCHeading(void) { return m_sTOCHeading;}
	UT_sint32                getCurrentLevel(void) const { return m_iCurrentLevel;}
	FootnoteType             getNumType(UT_sint32 iLevel);
	eTabLeader               getTabLeader(UT_sint32 iLevel);
	UT_sint32                getTabPosition(UT_sint32 iLevel, const fl_BlockLayout * pBlock);
	void                     setSelected(bool bSetSelected);
	bool                     isSelected(void) { return m_bIsSelected;}

	const UT_UTF8String &    getRangeBookmarkName() const {return m_sRangeBookmark;}
	bool                     verifyBookmarkAssumptions();
	bool                     fillTOC(void);
	
	static std::string       getDefaultHeading();
	static UT_UTF8String     getDefaultSourceStyle(UT_uint32 iLevel);
	static UT_UTF8String     getDefaultDestStyle(UT_uint32 iLevel);

private:
	virtual void             _purgeLayout(void);
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
	void                     _createTOCContainer(void);
	bool                     _isStyleInTOC(UT_UTF8String & sStyle, UT_UTF8String & sTOCStyle);
	void                     _insertTOCContainer(fp_TOCContainer * pNewTOC);
	void                     _localCollapse();

	void                     _createAndFillTOCEntry(PT_DocPosition posStart, PT_DocPosition posEnd,
													fl_BlockLayout * pPrevBL, const char * pszStyle,
													UT_sint32 iAllBlocks);

	void                     _addBlockInVec(fl_BlockLayout * pBlock,UT_UTF8String & sStyle);
	void                     _removeBlockInVec(fl_BlockLayout * pBlock, bool bDontRecurse = false);
	void                     _calculateLabels(void);
	UT_sint32                _getStartValue(TOCEntry * pEntry);
	bool                     m_bNeedsRebuild;
	bool                     m_bNeedsFormat;
	bool                     m_bIsOnPage;
	UT_uint32                m_iTOCPID;
	fl_DocSectionLayout*	 m_pDocSL;
	bool                     m_bHasEndTOC;
	bool                     m_bDoingPurge;
	bool                     m_bIsSelected;
	UT_UTF8String            m_sSourceStyle1;
	UT_UTF8String            m_sSourceStyle2;
	UT_UTF8String            m_sSourceStyle3;
	UT_UTF8String            m_sSourceStyle4;
	UT_UTF8String            m_sDestStyle1;
	UT_UTF8String            m_sDestStyle2;
	UT_UTF8String            m_sDestStyle3;
	UT_UTF8String            m_sDestStyle4;
	UT_UTF8String            m_sNumOff1;
	UT_UTF8String            m_sNumOff2;
	UT_UTF8String            m_sNumOff3;
	UT_UTF8String            m_sNumOff4;
	FootnoteType             m_iNumType1;
	FootnoteType             m_iNumType2;
	FootnoteType             m_iNumType3;
	FootnoteType             m_iNumType4;
	eTabLeader               m_iTabLeader1;
	eTabLeader               m_iTabLeader2;
	eTabLeader               m_iTabLeader3;
	eTabLeader               m_iTabLeader4;
	UT_GenericVector<TOCEntry *> m_vecEntries;
	UT_sint32                m_iCurrentLevel;
	UT_UTF8String            m_sTOCHeading;
	bool                     m_bTOCHeading;
	UT_UTF8String            m_sTOCHeadingStyle;
	FootnoteType             m_iLabType1;
	FootnoteType             m_iLabType2;
	FootnoteType             m_iLabType3;
	FootnoteType             m_iLabType4;
	UT_UTF8String            m_sLabBefore1;
	UT_UTF8String            m_sLabBefore2;
	UT_UTF8String            m_sLabBefore3;
	UT_UTF8String            m_sLabBefore4;
	UT_UTF8String            m_sLabAfter1;
	UT_UTF8String            m_sLabAfter2;
	UT_UTF8String            m_sLabAfter3;
	UT_UTF8String            m_sLabAfter4;
	UT_UTF8String            m_sRangeBookmark;
	bool                     m_bHasLabel1;
	bool                     m_bHasLabel2;
	bool                     m_bHasLabel3;
	bool                     m_bHasLabel4;
	bool                     m_bInherit1;
	bool                     m_bInherit2;
	bool                     m_bInherit3;
	bool                     m_bInherit4;
	UT_sint32                m_iStartAt1;
	UT_sint32                m_iStartAt2;
	UT_sint32                m_iStartAt3;
	UT_sint32                m_iStartAt4;
	bool                     m_bMissingBookmark;
	bool                     m_bFalseBookmarkEstimate;
	UT_NumberVector          m_vecBookmarkPositions;
};


class ABI_EXPORT fl_TOCListener : public PL_Listener
{
public:
	fl_TOCListener(fl_TOCLayout* pTOCL, fl_BlockLayout * pPrevBL, PD_Style * pStyle);
	virtual ~fl_TOCListener();

	virtual bool				populate(fl_ContainerLayout* sfh,
										 const PX_ChangeRecord * pcr);

	virtual bool				populateStrux(pf_Frag_Strux* sdh,
											  const PX_ChangeRecord * pcr,
											  fl_ContainerLayout* * psfh);

	virtual bool				change(fl_ContainerLayout* sfh,
									   const PX_ChangeRecord * pcr);

	virtual bool				insertStrux(fl_ContainerLayout* sfh,
											const PX_ChangeRecord * pcr,
											pf_Frag_Strux* sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	PL_ListenerId lid,
																	fl_ContainerLayout* sfhNew));

	virtual bool				signal(UT_uint32 iSignal);

private:
	PD_Document*				m_pDoc;
	fl_TOCLayout* 			    m_pTOCL;
	fl_BlockLayout *            m_pPrevBL;
	bool						m_bListening;
	fl_ContainerLayout*			m_pCurrentBL;
	PD_Style *                  m_pStyle;
};

#endif /* TOCLAYOUT_H */
