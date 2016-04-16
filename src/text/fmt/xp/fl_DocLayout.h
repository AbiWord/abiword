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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef DOCLAYOUT_H
#define DOCLAYOUT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "pt_Types.h"
#include "fp_PageSize.h"
#include "fl_Layout.h"
#include "ut_units.h"
#include "xav_Listener.h"


typedef enum _FootnoteType
{
	FOOTNOTE_TYPE_NUMERIC = 0,
	FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS,
	FOOTNOTE_TYPE_NUMERIC_PAREN,
	FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN,
	FOOTNOTE_TYPE_LOWER,
	FOOTNOTE_TYPE_LOWER_PAREN,
	FOOTNOTE_TYPE_LOWER_OPEN_PAREN,
	FOOTNOTE_TYPE_UPPER,
	FOOTNOTE_TYPE_UPPER_PAREN,
	FOOTNOTE_TYPE_UPPER_OPEN_PAREN,
	FOOTNOTE_TYPE_LOWER_ROMAN,
	FOOTNOTE_TYPE_LOWER_ROMAN_PAREN,
	FOOTNOTE_TYPE_UPPER_ROMAN,
	FOOTNOTE_TYPE_UPPER_ROMAN_PAREN,
	_FOOTNOTE_TYPE_INVALID = 10000
} FootnoteType;

/** describe a footnote. */
typedef struct {
	FootnoteType n;        /**< numeric value */
	const char * label;    /**< UI label */
	const char * prop;     /**< property string */
} FootnoteTypeDesc;

extern const FootnoteTypeDesc s_FootnoteTypeDesc[];

class FV_View;
class fl_DocListener;
class fl_SectionLayout;
class fl_DocSectionLayout;
class fl_BlockLayout;
class fl_TOCLayout;
class fp_Page;
class PD_Document;
class PP_AttrProp;
class GR_Graphics;
class GR_Font;
class UT_Timer;
class UT_Worker;
class fl_PartOfBlock;
class fl_AutoNum;
class PX_ChangeRecord_StruxChange;
class fl_FootnoteLayout;
class fl_AnnotationLayout;
class fl_EndnoteLayout;
class fp_EndnoteContainer;
class GR_EmbedManager;
class fl_FrameLayout;
class fp_FrameContainer;
class fp_Container;

// the following get used by view and layout code,
// since they're private to the formatter, we stick 'em here
#define fl_PAGEVIEW_PAGE_SEP	m_pG->tlu(20)		// must be <= MARGIN_Y
#define fl_PAGEVIEW_MARGIN_X	m_pG->tlu(25)
#define fl_PAGEVIEW_MARGIN_Y	m_pG->tlu(25)

// ----------------------------------------------------------------

// Note: Here used to be an overview of the classes in this
// directory. That has been moved to README.TXT since a newcomer would
// not know which source file to look in for that
// information. README.TXT on the other hand stands clearly out from
// the rest of the files.                           jskov 2000.12.29

class ABI_EXPORT FL_DocLayout
{
	friend class fl_DocListener;
	friend class fl_BlockLayout;
public:
	FL_DocLayout(PD_Document* doc, GR_Graphics* pG);
	~FL_DocLayout();

	void fillLayouts(void);
	bool loadPendingObjects(void);
	bool AnchoredObjectHelper(double x, double y, UT_sint32 iPage, UT_UTF8String & allProps, PT_DocPosition & pos, fp_Page *& pPage);
	void setView(FV_View*);

	inline FV_View * getView(void) const { return m_pView; }
	inline GR_Graphics*	getGraphics(void) const { return m_pG; }
	void setGraphics(GR_Graphics * pG);
	UT_uint32           getGraphicTick(void) const { return m_iGraphicTick;}
	void                incrementGraphicTick(void) { m_iGraphicTick++;}
	inline PD_Document*	getDocument(void) const { return m_pDoc; }
#ifdef ENABLE_SPELL
	inline const fl_BlockLayout* getPendingBlockForSpell(void) const { return m_pPendingBlockForSpell; };
	inline fl_PartOfBlock* getPendingWordForSpell(void) const { return m_pPendingWordForSpell; };
#endif

	// The smart quote stuff works by listening for insertions (typing
	// and paste) and motion.  It needs one character of type-ahead
	// before working the algorithm, so a single quote character going
	// by is remembered as "pending".  After the type-ahead (or
	// motion) occurs, the pending quote is considered for promotion.
	// For an insertion of multiple characters (which probably just
	// means a paste), all smart quote consideration can be done
	// immediately except for a quote occuring in the very last
	// character of the stuff being inserted.
	inline fl_BlockLayout* getPendingBlockForSmartQuote(void) const { return m_pPendingBlockForSmartQuote; };
	inline UT_uint32 getOffsetForSmartQuote(void) const { return m_uOffsetForSmartQuote; };
	void setPendingSmartQuote(fl_BlockLayout *block, UT_uint32 offset);
	void considerSmartQuoteCandidateAt(fl_BlockLayout *block, UT_uint32 offset);
	inline void considerPendingSmartQuoteCandidate() {considerSmartQuoteCandidateAt(m_pPendingBlockForSmartQuote, m_uOffsetForSmartQuote); }

	void			notifyBlockIsBeingDeleted(fl_BlockLayout *pBlock);
	void            setSkipUpdates(UT_uint32 numSkips) {m_iSkipUpdates = numSkips;}
	UT_uint32       getSkipUpdates(void) { return m_iSkipUpdates;}
	void            setNeedsRedraw(void);
	UT_sint32		getHeight() const;
	UT_sint32       getWidth() const;
	void            refreshRunProperties(void);
	const GR_Font*	findFont(const PP_AttrProp * pSpanAP,
							 const PP_AttrProp * pBlockAP,
							 const PP_AttrProp * pSectionAP,
							 bool isField = false
		) const;

	const GR_Font*	findFont(const PP_AttrProp * pSpanAP,
				 const PP_AttrProp * pBlockAP,
				 const PP_AttrProp * pSectionAP,
				 GR_Graphics * pG,
				 bool isField = false
		) const;

	void        changeDocSections(const PX_ChangeRecord_StruxChange * pcrx, fl_DocSectionLayout * pDSL);
	fp_Page*	addNewPage(fl_DocSectionLayout* pOwner, bool bNoUpdate=false);
	fp_Page*	getFirstPage() const;
	fp_Page*	getLastPage() const;
	fp_Page*	getNthPage(int n) const;
	UT_sint32	countPages() const;
	UT_sint32   findPage(fp_Page * pPage) const; // FIXME figure out how to pass a const fp_Page *
	void            setFramePageNumbers(UT_sint32 iStartPage);
	fl_FrameLayout* relocateFrame(fl_FrameLayout * pFL, fl_BlockLayout * newBlock,
				      const PP_PropertyVector & attributes = PP_NOPROPS, const PP_PropertyVector & properties = PP_NOPROPS);
	void            clearAllCountWraps(void);
	bool addFramesToBeInserted(fp_FrameContainer * pFrame);
	bool removeFramesToBeInserted(fp_FrameContainer * pFrame);
	fp_FrameContainer * findFramesToBeInserted(fp_Page * pPage);

	UT_sint32   getPercentFilled(void) const
		{ return m_iFilled;}
	void         setPercentFilled(UT_sint32 iFill)
		{ m_iFilled = iFill;}
	void        setLayoutIsFilling(bool bisFill) { m_bisLayoutFilling = bisFill;}
	bool        isLayoutFilling(void) const { return m_bisLayoutFilling;}
	fl_BlockLayout*	findBlockAtPosition(PT_DocPosition pos, bool bLookOnlyBefore = false) const;
    fl_BlockLayout*	findBlockAtPositionReverse(PT_DocPosition pos) const;
	void		deletePage(fp_Page* pPage, bool bDontNotify);

	void		formatAll();
	void  		updateLayout();
	void        updateOnViewModeChange();
	void        rebuildFromHere(fl_DocSectionLayout * pDSL);
	void        updateColor();

#ifdef ENABLE_SPELL
	bool		isPendingWordForSpell(void) const;
	bool		touchesPendingWordForSpell(fl_BlockLayout *pBlock,
										   UT_sint32 iOffset,
										   UT_sint32 chg) const;
	void		setPendingWordForSpell(const fl_BlockLayout *pBlock, fl_PartOfBlock* pWord);
	bool		checkPendingWordForSpell(void);
	void        dequeueAll(void);
	void        queueAll(UT_uint32 iReason);
	void 		queueBlockForBackgroundCheck(UT_uint32 reason, fl_BlockLayout *pBlock, bool bHead=false);
	bool 		dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock);

    fl_BlockLayout *spellQueueHead(void) const
		{
			return m_toSpellCheckHead;
		}
	void        setSpellQueueHead(fl_BlockLayout *h)
		{
			m_toSpellCheckHead = h;
		}
	fl_BlockLayout *spellQueueTail(void) const
		{
			return m_toSpellCheckTail;
		}
	void        setSpellQueueTail(fl_BlockLayout *t)
		{
			m_toSpellCheckTail = t;
		}
#endif
	void		addSection(fl_DocSectionLayout*);
	void		removeSection(fl_DocSectionLayout*);
	void		insertSectionAfter(fl_DocSectionLayout* pAfter, fl_DocSectionLayout* pNewSL);
	void		addHdrFtrSection(fl_SectionLayout* pHdrFtrSL);
	void		removeHdrFtrSection(fl_SectionLayout* pHdrFtrSL);

	inline		fl_DocSectionLayout* getFirstSection(void) const { return m_pFirstSection; }
	inline		fl_DocSectionLayout* getLastSection(void) const { return m_pLastSection; }

	fl_DocSectionLayout* 	findSectionForHdrFtr(const char* pszHdrFtrID) const;
	void 				deleteEmptyColumnsAndPages(void);
	void 				deleteEmptyPages( bool bDontNotify = false);
	GR_EmbedManager * getEmbedManager(const char * szEmbedType);
	UT_sint32       getDocPageHeight(void) const {return m_iPageHeight;}
	UT_sint32       getDocPageWidth(void) const {return m_iPageWidth;}

// --------------------------------------------------------------------
// Footnote Methods
// fl_DocLAyout stores this Vector of footnotes to speed things up and
// to provide convience routines for other classes and methods
//
	UT_uint32           countFootnotes(void) const;
	void                addFootnote(fl_FootnoteLayout *);
	void                removeFootnote(fl_FootnoteLayout *);
	fl_FootnoteLayout * getNthFootnote(UT_sint32 i) const;
	UT_sint32           getFootnoteVal(UT_uint32 footpid) const;
	fl_FootnoteLayout * findFootnoteLayout(UT_uint32 footpid) const;
	FootnoteType        getFootnoteType(void) const;
	void                getStringFromFootnoteVal(UT_String & sVal, UT_sint32 iVal, FootnoteType iFootType) const;

	UT_sint32           getInitialFootVal(void) const
		{ return m_iFootnoteVal;}
	bool                getRestartFootOnSection(void) const
		{ return m_bRestartFootSection;}
	bool                getRestartFootOnPage(void) const
		{ return m_bRestartFootPage;}
	FootnoteType        FootnoteTypeFromString( const gchar * pszStr);
// EndNotes
	void                insertEndnoteContainer(fp_EndnoteContainer * pECon);
	void                removeEndnoteContainer(fp_EndnoteContainer * pECon);
	fl_DocSectionLayout * getDocSecForEndnote(fp_EndnoteContainer * pECon  ) const;
	UT_uint32           countEndnotes(void) const;
	void                addEndnote(fl_EndnoteLayout *);
	void                removeEndnote(fl_EndnoteLayout *);
	fl_EndnoteLayout *  getNthEndnote(UT_sint32 i) const;
	UT_sint32           getEndnoteVal(UT_uint32 endpid) const;
	fl_EndnoteLayout * findEndnoteLayout(UT_uint32 endpid) const;

	FootnoteType        getEndnoteType(void) const
		{ return m_EndnoteType; }
	UT_sint32           getInitialEndVal(void) const
		{ return m_iEndnoteVal;}
	bool                getRestartEndOnSection(void) const
		{ return m_bRestartEndSection;}
	bool                getPlaceEndAtDocEnd(void) const
		{ return m_bPlaceAtDocEnd;}
	bool                getPlaceEndAtSecEnd(void) const
		{ return m_bPlaceAtSecEnd;}
// --------------------------------------------------------------------
// Annotation Methods
// fl_DocLAyout stores this Vector of Anntations to speed things up and
// to provide convience routines for other classes and methods
//
	UT_uint32           countAnnotations(void) const;
	void                addAnnotation(fl_AnnotationLayout * pAL);
	void                removeAnnotation(fl_AnnotationLayout * pAL);
	fl_AnnotationLayout * getNthAnnotation(UT_sint32 i) const;
	UT_sint32           getAnnotationVal(UT_uint32 footpid) const;
	fl_AnnotationLayout * findAnnotationLayout(UT_uint32 footpid) const;
	bool                displayAnnotations(void) const;
	void                setDisplayAnnotations(bool bDisplayAnnotations);
	bool                collapseAnnotations(void);

// --------------------------------------------------------------------
// RDF Anchor Methods
	bool                displayRDFAnchors(void) const;
	void                setDisplayRDFAnchors(bool v);

// ---------------------------------------------------
// Table of contents
//----------------------------------------------------
	UT_sint32           getNumTOCs(void) const;
	fl_TOCLayout *      getNthTOC(UT_sint32 i) const;
	bool                addOrRemoveBlockFromTOC(fl_BlockLayout * pBlock);
	bool                removeBlockFromTOC(fl_BlockLayout * pBlock);
	bool                isBlockInTOC(fl_BlockLayout * pBlock) const;
	bool                getMatchingBlocksFromTOCs(fl_BlockLayout * pBlock,  UT_GenericVector<fl_BlockLayout*>* pVecBlock) const;
	bool                addTOC(fl_TOCLayout * pTOC);
	bool                removeTOC(fl_TOCLayout * pTOC);
	void                recalculateTOCFields(void);
	bool                updateTOCsOnBookmarkChange(const gchar * pBookmark);
// --------------------------------------------------------------------
#ifdef ENABLE_SPELL
	bool		getAutoSpellCheck(void) const { return (hasBackgroundCheckReason(bgcrSpelling)); }
	bool		getAutoGrammarCheck(void) const { return (hasBackgroundCheckReason(bgcrGrammar)); }
	bool		getSpellCheckCaps(void) const { return m_bSpellCheckCaps; }
	bool		getSpellCheckNumbers(void) const { return m_bSpellCheckNumbers; }
	bool		getSpellCheckInternet(void) const { return m_bSpellCheckInternet; }
	void		recheckIgnoredWords();
#endif
// --------------------------------------------------------------------
	bool		getSmartQuotes(void) const { return (hasBackgroundCheckReason(bgcrSmartQuotes)); }
// --------------------------------------------------------------------

	inline void			addBackgroundCheckReason(UT_uint32 reason) {m_uDocBackgroundCheckReasons |= reason;}
	inline void			removeBackgroundCheckReason(UT_uint32 reason) {m_uDocBackgroundCheckReasons &= ~reason;}
	inline bool		hasBackgroundCheckReason(UT_uint32 reason) const {return ((m_uDocBackgroundCheckReasons & reason) ? true : false);}
	inline UT_uint32	getBackgroundCheckReasons() const {return (m_uDocBackgroundCheckReasons);}

#ifdef ENABLE_SPELL
	fl_BlockLayout *        getPendingBlockForGrammar(void) const
	  {
	    return m_PendingBlockForGrammar;
	  }
	void        setPendingBlockForGrammar(fl_BlockLayout * pBL);
#endif
	void        triggerPendingBlock(fl_BlockLayout * pBL);

	// These are used as bit flags in a UT_uint32.  The enum is here just
	// to get the namespace protection.
	enum backgroundCheckReason
	{
		bgcrNone         = 0,
		bgcrDebugFlash   = (1 <<  0),
		bgcrSpelling     = (1 <<  1),
		bgcrSmartQuotes  = (1 <<  2),   // ha!  we're not using background checks for this after all
		bgcrGrammar     = (1 <<  3)
	};

	// New List Guts
	inline fl_AutoNum * 	getListByID(UT_uint32 id) const;
	inline fl_AutoNum *	getNthList(UT_uint32 i) const; // { return m_vecLists[i]; }
	inline UT_uint32	getListsCount(void) const; // { return m_vecLists.getItemCount(); }
	inline void		addList(fl_AutoNum * pAutoNum);
	bool            isLayoutDeleting(void) const {return m_bDeletingLayout;}
	UT_uint32       getRedrawCount() const {return m_iRedrawCount;}


	void            updatePropsNoRebuild(void);
	void            updatePropsRebuild(void);
	PT_DocPosition  getDocSize(void) const
		{ return m_iDocSize;}
	UT_uint32       getLID(void) const
		{ return m_lid;}
	void            notifyListeners(AV_ChangeMask mask);
	void            setQuickPrint(GR_Graphics * pGraphics);
	GR_Graphics *   getQuickPrintGraphics(void) const;
	bool            isQuickPrint(void) const
	{ return m_bIsQuickPrint;}
	GR_EmbedManager * getQuickPrintEmbedManager(const char * szEmbedType);
	fp_PageSize	m_docViewPageSize;
	bool            setDocViewPageSize(const PP_AttrProp * pAP);
        void            setSaveContainerPointer( fp_Container * pContainer);
	void            setRebuiltBlock(fl_BlockLayout *pBlock);
	fl_BlockLayout* getRebuiltBlock(void) const;
	fp_Container *  getSavedContainerPointer(void) const;

#ifdef FMT_TEST
	//! Pointer to last instatiated FL_DocLayout. Used for debugging.
	static		FL_DocLayout* m_pDocLayout;

	void		__dump(FILE * fp) const;
#endif

protected:
	static void			_backgroundCheck(UT_Worker * pTimer);
#ifdef ENABLE_SPELL
	void				_toggleAutoSpell(bool bSpell);
	void				_toggleAutoGrammar(bool bGrammar);
#endif
	void				_toggleAutoSmartQuotes(bool bSQ);

	static void			_prefsListener(class XAP_Prefs *,
									   UT_StringPtrMap *, void *);


	static void			_redrawUpdate(UT_Worker * pTimer);
	void				_setDocPageDimensions(void);

private:
	void                _lookupProperties(void);
	GR_Graphics*		m_pG;
	PD_Document*		m_pDoc;
	FV_View*			m_pView;
	fl_DocListener*		m_pDocListener;
	PL_ListenerId		m_lid;

	UT_GenericVector<fp_Page *> m_vecPages;
	fl_DocSectionLayout*m_pFirstSection;
	fl_DocSectionLayout*m_pLastSection;

	// spell check stuff
	// UT_GenericVector<fl_BlockLayout *> m_vecUncheckedBlocks;
	fl_BlockLayout      *m_toSpellCheckHead;
	fl_BlockLayout      *m_toSpellCheckTail;
	const fl_BlockLayout*		m_pPendingBlockForSpell;	// if NULL, then ignore m_pPendingWordForSpell
	fl_PartOfBlock*		m_pPendingWordForSpell;
	bool				m_bSpellCheckCaps;
	bool				m_bSpellCheckNumbers;
	bool				m_bSpellCheckInternet;
	bool                m_bAutoSpellCheck;
	UT_uint32                       m_uDocBackgroundCheckReasons;
	bool                         m_bStopSpellChecking; // Handshaking
	bool                         m_bImSpellCheckingNow; // Variables
	// smart quote latent instance
	fl_BlockLayout*		m_pPendingBlockForSmartQuote;  // if NULL, ignore m_uOffsetForSmartQuote
	UT_uint32           m_uOffsetForSmartQuote;
	char                m_szCurrentTransparentColor[10];
	UT_Worker*			m_pBackgroundCheckTimer;

	XAP_Prefs *			m_pPrefs;

	UT_Timer*			m_pRedrawUpdateTimer;
	UT_uint32           m_iSkipUpdates;
	bool                m_bDeletingLayout;
	bool                m_bisLayoutFilling;
	UT_uint32           m_iRedrawCount;
	UT_sint32           m_iPageWidth;
	UT_sint32           m_iPageHeight;
	UT_GenericVector<fl_FootnoteLayout *> m_vecFootnotes;
	UT_GenericVector<fl_AnnotationLayout *> m_vecAnnotations;
	UT_GenericVector<fl_EndnoteLayout *> m_vecEndnotes;
	FootnoteType        m_FootnoteType;
	UT_sint32           m_iFootnoteVal;
	bool                m_bRestartFootSection;
	bool                m_bRestartFootPage;
	UT_sint32           m_iEndnoteVal;
	FootnoteType        m_EndnoteType;
	bool                m_bRestartEndSection;
	bool                m_bPlaceAtDocEnd;
	bool                m_bPlaceAtSecEnd;
	UT_uint32           m_iGraphicTick;
	UT_GenericVector<fl_TOCLayout *> m_vecTOC;
	PT_DocPosition      m_iDocSize;
	UT_sint32           m_iFilled;
	bool                m_bSpellCheckInProgress;
	std::map<std::string, GR_EmbedManager *> m_mapEmbedManager;
	bool                m_bAutoGrammarCheck;
	fl_BlockLayout  *   m_PendingBlockForGrammar;
	UT_sint32           m_iGrammarCount;
	bool                m_bFinishedInitialCheck;
	PT_DocPosition      m_iPrevPos;
	std::map<std::string, GR_EmbedManager *> m_mapQuickPrintEmbedManager;
	GR_Graphics *       m_pQuickPrintGraphics;
	bool                m_bIsQuickPrint;
	bool                m_bDisplayAnnotations;
	bool                m_bDisplayRDFAnchors;
        fp_Container *      m_pSavedContainer;
	fl_BlockLayout *    m_pRebuiltBlockLayout;
	UT_GenericVector<fp_FrameContainer *> m_vecFramesToBeInserted;
};

#endif /* DOCLAYOUT_H */


