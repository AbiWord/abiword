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


class FV_View;
class fl_DocListener;
class fl_SectionLayout;
class fl_BlockLayout;
class fp_Page;
class PD_Document;
class PP_AttrProp;
class GR_Graphics;
class GR_Font;
class UT_Timer;
class fl_PartOfBlock;


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
	inline fl_BlockLayout* getPendingBlock(void) const { return m_pPendingBlock; };
	inline fl_PartOfBlock* getPendingWord(void) const { return m_pPendingWord; };
	
	UT_sint32		getHeight();
	UT_sint32       getWidth();

	GR_Font*		findFont(const PP_AttrProp * pSpanAP,
							 const PP_AttrProp * pBlockAP,
							 const PP_AttrProp * pSectionAP);
	
	fp_Page*	addNewPage();
	fp_Page*	getFirstPage();
	fp_Page*	getLastPage();
	fp_Page*	getNthPage(int n);
	UT_uint32	countPages();

	fl_BlockLayout*	findBlockAtPosition(PT_DocPosition pos);
	void deleteEmptyColumnsAndPages(void);
	void		deletePage(fp_Page*);

	void		formatAll();
	void  		updateLayout();

	UT_Bool		isPendingWord(void) const;
	UT_Bool		touchesPendingWord(fl_BlockLayout *pBlock, 
								   UT_uint32 iOffset, 
								   UT_sint32 chg) const;
	void		setPendingWord(fl_BlockLayout *pBlock, fl_PartOfBlock* pWord);
	void		checkPendingWord(void);
	
	void 		queueBlockForSpell(fl_BlockLayout *pBlock, UT_Bool bHead=UT_FALSE);
	void 		dequeueBlock(fl_BlockLayout *pBlock);

	void		addSection(fl_SectionLayout*);
	void		removeSection(fl_SectionLayout*);
	
	inline		fl_SectionLayout* getFirstSection(void) const { return m_pFirstSection; }
	inline		fl_SectionLayout* getLastSection(void) const { return m_pLastSection; }

#ifdef FMT_TEST
	void		__dump(FILE * fp) const;
#endif
	
protected:
	static void			_spellCheck(UT_Timer * pTimer);

	GR_Graphics*		m_pG;
	PD_Document*		m_pDoc;
	FV_View*			m_pView;
	fl_DocListener*		m_pDocListener;
	PL_ListenerId		m_lid;

	UT_Vector			m_vecPages;
	fl_SectionLayout*	m_pFirstSection;
	fl_SectionLayout*	m_pLastSection;
	
	UT_HashTable		m_hashFontCache;

	// spell check stuff
	UT_Timer*			m_pSpellCheckTimer; 
	UT_Vector			m_vecUncheckedBlocks;
	fl_BlockLayout*		m_pPendingBlock;	// if NULL, then ignore m_pPendingWord
	fl_PartOfBlock*		m_pPendingWord;
};

#endif /* DOCLAYOUT_H */
