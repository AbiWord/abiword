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

#ifndef DOCLAYOUT_H
#define DOCLAYOUT_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "pt_Types.h"
#include "fp_PageSize.h"


class FV_View;
class fl_DocListener;
class fl_DocSectionLayout;
class fl_BlockLayout;
class fp_Page;
class PD_Document;
class PP_AttrProp;
class GR_Graphics;
class GR_Font;
class UT_Timer;
class fl_PartOfBlock;
class fl_AutoNum;

// the following get used by view and layout code, 
// since they're private to the formatter, we stick 'em here
#define fl_PAGEVIEW_PAGE_SEP	20		// must be <= MARGIN_Y
#define fl_PAGEVIEW_MARGIN_X	25		
#define fl_PAGEVIEW_MARGIN_Y	25		

// ----------------------------------------------------------------
/*
	FL_DocLayout is a formatted representation of a specific PD_Document, 
	formatted for a specific GR_Graphics context.  

	A FL_DocLayout encapsulates two related hierarchies of objects.  

	The "logical" hierarchy corresponds to the logical structure of the 
	document, and consists of a list of fl_SectionLayout objects, which are 
	in turn composed of fl_BlockLayout objects. 

	The "physical" hierarchy, by contrast, encapsulates the subdivision 
	of physical space into objects of successively finer granularity.  Thus, 
	a FL_DocLayout is also a list of Pages, each of which was constructed 
	during the process of formatting the document.  In turn, 
	
		each fp_Page is a list of fp_Columns
		Each fp_Column is a list of fp_Lines
		Each fp_Line is a list of Runs.  
		
	Finally, each fp_Run contains some fragment of content from the original 
	document, usually text.
*/

class FL_DocLayout
{
	friend class fl_DocListener;

public:
	FL_DocLayout(PD_Document* doc, GR_Graphics* pG);
	~FL_DocLayout();

	void setView(FV_View*);

	inline FV_View * getView(void) const { return m_pView; }
	inline GR_Graphics*	getGraphics(void) const { return m_pG; }
	inline PD_Document*	getDocument(void) const { return m_pDoc; }
	inline fl_BlockLayout* getPendingBlockForSpell(void) const { return m_pPendingBlockForSpell; };
	inline fl_PartOfBlock* getPendingWordForSpell(void) const { return m_pPendingWordForSpell; };
	
	// The smart quote stuff works by listening for insertions (typing and paste) and motion.
	// It needs one character of type-ahead before working the algorithm, so a single
	// quote character going by is remembered as "pending".  After the type-ahead (or
	// motion) occurs, the pending quote is considered for promotion.  For an insertion
	// of multiple characters (which probably just means a paste), all smart quote consideration
	// can be done immediately except for a quote occuring in the very last character
	// of the stuff being inserted.
	inline fl_BlockLayout* getPendingBlockForSmartQuote(void) const { return m_pPendingBlockForSmartQuote; };
	inline UT_uint32 getOffsetForSmartQuote(void) const { return m_uOffsetForSmartQuote; };
	void setPendingSmartQuote(fl_BlockLayout *block, UT_uint32 offset);
	void considerSmartQuoteCandidateAt(fl_BlockLayout *block, UT_uint32 offset);
	inline void considerPendingSmartQuoteCandidate() {considerSmartQuoteCandidateAt(m_pPendingBlockForSmartQuote, m_uOffsetForSmartQuote); }

	void			notifyBlockIsBeingDeleted(fl_BlockLayout *pBlock);

	UT_sint32		getHeight();
	UT_sint32       getWidth();

	enum
	{
		FIND_FONT_AT_SCREEN_RESOLUTION,
		FIND_FONT_AT_LAYOUT_RESOLUTION
	};
	GR_Font*		findFont(const PP_AttrProp * pSpanAP,
							 const PP_AttrProp * pBlockAP,
							 const PP_AttrProp * pSectionAP,
							 UT_sint32 iUseLayoutResolution
							);
	GR_Font*		findFont(const PP_AttrProp * pSpanAP,
							 const PP_AttrProp * pBlockAP,
							 const PP_AttrProp * pSectionAP,
					 UT_sint32 iUseLayoutResolution,
					 UT_Bool isField
							);
	
	fp_Page*	addNewPage(fl_DocSectionLayout* pOwner);
	fp_Page*	getFirstPage();
	fp_Page*	getLastPage();
	fp_Page*	getNthPage(int n);
	UT_uint32	countPages();

	fl_BlockLayout*	findBlockAtPosition(PT_DocPosition pos);
	void		deletePage(fp_Page*);

	void		formatAll();
	void  		updateLayout();

	UT_Bool		isPendingWordForSpell(void) const;
	UT_Bool		touchesPendingWordForSpell(fl_BlockLayout *pBlock, 
								   UT_uint32 iOffset, 
								   UT_sint32 chg) const;
	void		setPendingWordForSpell(fl_BlockLayout *pBlock, fl_PartOfBlock* pWord);
	UT_Bool		checkPendingWordForSpell(void);
	
	void 		queueBlockForBackgroundCheck(UT_uint32 reason, fl_BlockLayout *pBlock, UT_Bool bHead=UT_FALSE);
	void 		dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock);

	void		addSection(fl_DocSectionLayout*);
	void		removeSection(fl_DocSectionLayout*);
	void		insertSectionAfter(fl_DocSectionLayout* pAfter, fl_DocSectionLayout* pNewSL);

	inline		fl_DocSectionLayout* getFirstSection(void) const { return m_pFirstSection; }
	inline		fl_DocSectionLayout* getLastSection(void) const { return m_pLastSection; }

	fl_DocSectionLayout* 	findSectionForHdrFtr(const char* pszHdrFtrID) const;
	void 				deleteEmptyColumnsAndPages(void);
	void 				deleteEmptyPages(void);


	UT_Bool		getAutoSpellCheck(void) const { return (hasBackgroundCheckReason(bgcrSpelling)); }
	UT_Bool		getSpellCheckCaps(void) const { return m_bSpellCheckCaps; }
	UT_Bool		getSpellCheckNumbers(void) const { return m_bSpellCheckNumbers; }
	UT_Bool		getSpellCheckInternet(void) const { return m_bSpellCheckInternet; }

	void		recheckIgnoredWords();

	inline void			addBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons |= reason;}
	inline void			removeBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons &= ~reason;}
	inline UT_Bool		hasBackgroundCheckReason(UT_uint32 reason) const {return (m_uBackgroundCheckReasons & reason);}
	inline UT_uint32	getBackgroundCheckReasons() const {return (m_uBackgroundCheckReasons);}

	// These are used as bit flags in a UT_uint32.  The enum is here just
	// to get the namespace protection.
	enum backgroundCheckReason
	{
		bgcrDebugFlash   = (1 <<  0),
		bgcrSpelling     = (1 <<  1),
		bgcrSmartQuotes  = (1 <<  2)   // ha!  we're not using background checks for this after all
	};

	// New List Guts
	inline fl_AutoNum * 	getListByID(UT_uint32 id) const;
	inline fl_AutoNum *	getNthList(UT_uint32 i) const; // { return m_vecLists[i]; }
	inline UT_uint32	getListsCount(void) const; // { return m_vecLists.getItemCount(); }
	inline void		addList(fl_AutoNum * pAutoNum);
	
#ifdef FMT_TEST
	static		FL_DocLayout* m_pDocLayout;

	void		__dump(FILE * fp) const;
#endif
	
protected:
	static void			_backgroundCheck(UT_Timer * pTimer);
	void				_toggleAutoSpell(UT_Bool bSpell);
	void				_toggleAutoSmartQuotes(UT_Bool bSQ);
	
	static void			_prefsListener(class XAP_App *, class XAP_Prefs *, 
									   class UT_AlphaHashTable *, void *);


	static void			_redrawUpdate(UT_Timer * pTimer);
	static fp_PageSize	_getDefaultPageSize();

	GR_Graphics*		m_pG;
	PD_Document*		m_pDoc;
	FV_View*			m_pView;
	fl_DocListener*		m_pDocListener;
	PL_ListenerId		m_lid;

	UT_Vector			m_vecPages;
	fl_DocSectionLayout*m_pFirstSection;
	fl_DocSectionLayout*m_pLastSection;
	
	UT_HashTable		m_hashFontCache;

	// spell check stuff
	UT_Vector			m_vecUncheckedBlocks;
	fl_BlockLayout*		m_pPendingBlockForSpell;	// if NULL, then ignore m_pPendingWordForSpell
	fl_PartOfBlock*		m_pPendingWordForSpell;
	UT_Bool				m_bSpellCheckCaps;
	UT_Bool				m_bSpellCheckNumbers;
	UT_Bool				m_bSpellCheckInternet;

	UT_Bool                         m_bStopSpellChecking; // Handshaking
	UT_Bool                         m_bImSpellCheckingNow; // Variables
	// smart quote latent instance
	fl_BlockLayout*		m_pPendingBlockForSmartQuote;  // if NULL, ignore m_uOffsetForSmartQuote
	UT_uint32           m_uOffsetForSmartQuote;

	UT_Timer*			m_pBackgroundCheckTimer; 
	UT_uint32			m_uBackgroundCheckReasons;  // bit flags

	XAP_Prefs *			m_pPrefs;

//	UT_Vector			m_vecLists;
	
	UT_Timer*			m_pRedrawUpdateTimer;
	fp_PageSize			m_pageSize;
};

#endif /* DOCLAYOUT_H */


