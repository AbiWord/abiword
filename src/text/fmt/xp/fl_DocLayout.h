/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 



#ifndef DOCLAYOUT_H
#define DOCLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "pt_Types.h"

class FV_View;
class fl_DocListener;
class fl_BlockLayout;
class fp_Page;
class PD_Document;
class PP_AttrProp;
class DG_Graphics;
class DG_Font;

// ----------------------------------------------------------------
/*
	FL_DocLayout is a formatted representation of a specific PD_Document, 
	formatted for a specific DG_Graphics context.  

	A FL_DocLayout encapsulates two related hierarchies of objects.  

	The "logical" hierarchy corresponds to the logical structure of the 
	document, and consists of a list of fl_SectionLayout objects, which are 
	in turn composed of fl_BlockLayout objects. 

	The "physical" hierarchy, by contrast, encapsulates the subdivision 
	of physical space into objects of successively finer granularity.  Thus, 
	a FL_DocLayout is also a list of Pages, each of which was constructed 
	during the process of formatting the document.  In turn, 
	
		each fp_Page is a list of SectionSlices.  
		Each fp_SectionSlice is a list of Columns.  
		Each fp_Column is a list of BlockSlices.
		Each fp_BlockSlice is a list of Lines.  
		Each fp_Line is a list of Runs.  
		
	Finally, each fp_Run contains some fragment of content from the original 
	document, usually text.
*/

class FL_DocLayout
{
	friend class fl_DocListener;

public:
	FL_DocLayout(PD_Document* doc, DG_Graphics* pG);
	~FL_DocLayout();

	void setView(FV_View*);

	DG_Graphics*	getGraphics();
	PD_Document*	getDocument() const;
	UT_uint32		getHeight();
	UT_uint32       getWidth();

	DG_Font*		findFont(const PP_AttrProp * pSpanAP,
							 const PP_AttrProp * pBlockAP,
							 const PP_AttrProp * pSectionAP);
	
	fp_Page*	addNewPage();
	fp_Page*	getFirstPage();
	fp_Page*	getLastPage();
	fp_Page*	getNthPage(int n);
	int			countPages();

	fl_BlockLayout*	findBlockAtPosition(PT_DocPosition pos);

	int			formatAll();
	int			reformat();

	// Debug-related routines
	void dump();
	
protected:
	DG_Graphics*		m_pG;
	PD_Document*		m_pDoc;
	FV_View*			m_pView;
	fl_DocListener*		m_pDocListener;
	PL_ListenerId		m_lid;

	UT_Vector		m_vecPages;
	UT_Vector		m_vecSectionLayouts;
	UT_HashTable	m_hashFontCache;
};

#endif /* DOCLAYOUT_H */
