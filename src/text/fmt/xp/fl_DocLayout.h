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
#include "fl_SectionLayout.h"

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

// Note: Here used to be an overview of the classes in this
// directory. That has been moved to README.TXT since a newcomer would
// not know which source file to look in for that
// information. README.TXT on the other hand stands clearly out from
// the rest of the files.                           jskov 2000.12.29

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
							 bool isField
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
	void        updateColor();

	bool		isPendingWordForSpell(void) const;
	bool		touchesPendingWordForSpell(fl_BlockLayout *pBlock, 
										   UT_uint32 iOffset, 
										   UT_sint32 chg) const;
	void		setPendingWordForSpell(fl_BlockLayout *pBlock, fl_PartOfBlock* pWord);
	bool		checkPendingWordForSpell(void);
	
	void 		queueBlockForBackgroundCheck(UT_uint32 reason, fl_BlockLayout *pBlock, bool bHead=false);
	void 		dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock);

	void		addSection(fl_DocSectionLayout*);
	void		removeSection(fl_DocSectionLayout*);
	void		insertSectionAfter(fl_DocSectionLayout* pAfter, fl_DocSectionLayout* pNewSL);
	void		addHdrFtrSection(fl_SectionLayout* pHdrFtrSL);
	void		removeHdrFtrSection(fl_SectionLayout* pHdrFtrSL);


	inline		fl_DocSectionLayout* getFirstSection(void) const { return m_pFirstSection; }
	inline		fl_DocSectionLayout* getLastSection(void) const { return m_pLastSection; }

	fl_DocSectionLayout* 	findSectionForHdrFtr(const char* pszHdrFtrID) const;
	void 				deleteEmptyColumnsAndPages(void);
	void 				deleteEmptyPages(void);


	bool		getAutoSpellCheck(void) const { return (hasBackgroundCheckReason(bgcrSpelling)); }
	bool		getSpellCheckCaps(void) const { return m_bSpellCheckCaps; }
	bool		getSpellCheckNumbers(void) const { return m_bSpellCheckNumbers; }
	bool		getSpellCheckInternet(void) const { return m_bSpellCheckInternet; }

	void		recheckIgnoredWords();

	inline void			addBackgroundCheckReason(UT_uint32 reason) {m_uDocBackgroundCheckReasons |= reason;}
	inline void			removeBackgroundCheckReason(UT_uint32 reason) {m_uDocBackgroundCheckReasons &= ~reason;}
	inline bool		hasBackgroundCheckReason(UT_uint32 reason) const {return ((m_uDocBackgroundCheckReasons & reason) ? true : false);}
	inline UT_uint32	getBackgroundCheckReasons() const {return (m_uDocBackgroundCheckReasons);}

	// These are used as bit flags in a UT_uint32.  The enum is here just
	// to get the namespace protection.
	enum backgroundCheckReason
	{
		bgcrNone         = 0,
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
	//! Pointer to last instatiated FL_DocLayout. Used for debugging.
	static		FL_DocLayout* m_pDocLayout;

	void		__dump(FILE * fp) const;
#endif
	
protected:
	static void			_backgroundCheck(UT_Timer * pTimer);
	void				_toggleAutoSpell(bool bSpell);
	void				_toggleAutoSmartQuotes(bool bSQ);
	
	static void			_prefsListener(class XAP_App *, class XAP_Prefs *, 
									   class UT_AlphaHashTable *, void *);


	static void			_redrawUpdate(UT_Timer * pTimer);

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
	bool				m_bSpellCheckCaps;
	bool				m_bSpellCheckNumbers;
	bool				m_bSpellCheckInternet;
	UT_uint32                       m_uDocBackgroundCheckReasons;
	bool                         m_bStopSpellChecking; // Handshaking
	bool                         m_bImSpellCheckingNow; // Variables
	// smart quote latent instance
	fl_BlockLayout*		m_pPendingBlockForSmartQuote;  // if NULL, ignore m_uOffsetForSmartQuote
	UT_uint32           m_uOffsetForSmartQuote;
	char                m_szCurrentTransparentColor[10];
	UT_Timer*			m_pBackgroundCheckTimer; 

	XAP_Prefs *			m_pPrefs;

//	UT_Vector			m_vecLists;
	
	UT_Timer*			m_pRedrawUpdateTimer;
};

#endif /* DOCLAYOUT_H */

